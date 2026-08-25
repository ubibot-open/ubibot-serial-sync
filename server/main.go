// Command device-library-mock-server is a stand-in for the real backend
// described in docs/device-library-update-protocol.md -- it implements just
// the two read-only JSON endpoints the desktop app calls (GET
// /api/device-library/version and GET /api/device-library/latest, matching
// the path layout of the real Laravel backend in ubibot-appcenter's
// routes/api.php), reading their data straight from the two files under
// data/ so testing "there's an update" is just editing JSON and refreshing,
// no rebuild needed. No authentication -- every request is served as-is;
// the app's optional X-Api-Key header (see core/device_library_update_client.h)
// is simply ignored here.
package main

import (
	"crypto/sha256"
	"encoding/hex"
	"encoding/json"
	"flag"
	"log"
	"net/http"
	"os"
	"path/filepath"
)

// localizedText mirrors the client's LocalizedText ({zh, en}) -- see
// docs/device-library-update-protocol.md and src/core/device_library.h.
type localizedText struct {
	Zh string `json:"zh"`
	En string `json:"en"`
}

// meta.json holds the handful of fields that aren't already part of
// devices.json's own schema -- editing this file (not the Go source) is how
// you change what /version reports without recompiling.
type meta struct {
	PublishedAt   string        `json:"publishedAt"`
	MinAppVersion string        `json:"minAppVersion"`
	Changelog     localizedText `json:"changelog"`
}

// The parts of devices.json this server actually needs to look at -- models
// is kept as raw JSON (untouched, passed straight through to /latest's
// response) rather than fully modeled, since this server never needs to
// understand a command's own fields, just count them.
type devicesDoc struct {
	Version string            `json:"version"`
	Models  []json.RawMessage `json:"models"`
}

// Just enough of a model to count its commands for /version's modelCount/
// commandCount fields.
type modelForCounting struct {
	Commands []json.RawMessage `json:"commands"`
}

type versionResponse struct {
	OK            bool          `json:"ok"`
	Version       string        `json:"version"`
	PublishedAt   string        `json:"publishedAt,omitempty"`
	MinAppVersion string        `json:"minAppVersion,omitempty"`
	ModelCount    int           `json:"modelCount"`
	CommandCount  int           `json:"commandCount"`
	Changelog     localizedText `json:"changelog"`
}

type latestResponse struct {
	OK          bool              `json:"ok"`
	Version     string            `json:"version"`
	PublishedAt string            `json:"publishedAt,omitempty"`
	Models      []json.RawMessage `json:"models"`
}

type errorEnvelope struct {
	OK    bool `json:"ok"`
	Error struct {
		Code    string `json:"code"`
		Message string `json:"message"`
	} `json:"error"`
}

// server holds the data directory -- devices.json/meta.json are re-read from
// disk on every request (rather than cached at startup) so editing either
// file and just refreshing the app's "Check for updates" is enough to try
// out a new scenario; this is a mock for manual testing, not a production
// server, so the extra disk reads are a non-issue.
type server struct {
	dataDir string
}

func (s *server) loadDevices() (devicesDoc, error) {
	var doc devicesDoc
	data, err := os.ReadFile(filepath.Join(s.dataDir, "devices.json"))
	if err != nil {
		return doc, err
	}
	if err := json.Unmarshal(data, &doc); err != nil {
		return doc, err
	}
	return doc, nil
}

func (s *server) loadMeta() (meta, error) {
	var m meta
	data, err := os.ReadFile(filepath.Join(s.dataDir, "meta.json"))
	if err != nil {
		return m, err
	}
	if err := json.Unmarshal(data, &m); err != nil {
		return m, err
	}
	return m, nil
}

func writeJSON(w http.ResponseWriter, status int, body []byte) {
	w.Header().Set("Content-Type", "application/json; charset=utf-8")
	w.WriteHeader(status)
	_, _ = w.Write(body)
}

func writeError(w http.ResponseWriter, status int, code, message string) {
	var env errorEnvelope
	env.OK = false
	env.Error.Code = code
	env.Error.Message = message
	body, _ := json.Marshal(env)
	writeJSON(w, status, body)
}

// handleVersion implements GET /api/device-library/version -- see
// docs/device-library-update-protocol.md#4.
func (s *server) handleVersion(w http.ResponseWriter, r *http.Request) {
	if r.Method != http.MethodGet {
		writeError(w, http.StatusMethodNotAllowed, "METHOD_NOT_ALLOWED", "only GET is supported")
		return
	}

	doc, err := s.loadDevices()
	if err != nil {
		log.Printf("GET /version: reading devices.json: %v", err)
		writeError(w, http.StatusInternalServerError, "DATA_UNAVAILABLE", "could not read device library data")
		return
	}
	m, err := s.loadMeta()
	if err != nil {
		// meta.json is optional -- publishedAt/minAppVersion/changelog just
		// come back empty rather than the whole endpoint failing.
		log.Printf("GET /version: reading meta.json: %v (continuing with defaults)", err)
	}

	commandCount := 0
	for _, raw := range doc.Models {
		var mc modelForCounting
		if err := json.Unmarshal(raw, &mc); err == nil {
			commandCount += len(mc.Commands)
		}
	}

	resp := versionResponse{
		OK:            true,
		Version:       doc.Version,
		PublishedAt:   m.PublishedAt,
		MinAppVersion: m.MinAppVersion,
		ModelCount:    len(doc.Models),
		CommandCount:  commandCount,
		Changelog:     m.Changelog,
	}
	body, err := json.Marshal(resp)
	if err != nil {
		writeError(w, http.StatusInternalServerError, "ENCODE_FAILED", err.Error())
		return
	}
	log.Printf("GET /version -> %s (%s)", resp.Version, r.URL.RawQuery)
	writeJSON(w, http.StatusOK, body)
}

// handleLatest implements GET /api/device-library/latest -- see
// docs/device-library-update-protocol.md#5, including the X-Content-SHA256
// integrity header (hash of the exact response body being sent).
func (s *server) handleLatest(w http.ResponseWriter, r *http.Request) {
	if r.Method != http.MethodGet {
		writeError(w, http.StatusMethodNotAllowed, "METHOD_NOT_ALLOWED", "only GET is supported")
		return
	}

	doc, err := s.loadDevices()
	if err != nil {
		log.Printf("GET /latest: reading devices.json: %v", err)
		writeError(w, http.StatusInternalServerError, "DATA_UNAVAILABLE", "could not read device library data")
		return
	}
	m, err := s.loadMeta()
	if err != nil {
		log.Printf("GET /latest: reading meta.json: %v (continuing with defaults)", err)
	}

	resp := latestResponse{
		OK:          true,
		Version:     doc.Version,
		PublishedAt: m.PublishedAt,
		Models:      doc.Models,
	}
	body, err := json.Marshal(resp)
	if err != nil {
		writeError(w, http.StatusInternalServerError, "ENCODE_FAILED", err.Error())
		return
	}

	sum := sha256.Sum256(body)
	w.Header().Set("X-Content-SHA256", hex.EncodeToString(sum[:]))
	log.Printf("GET /latest -> %s (%d models, %s)", resp.Version, len(resp.Models), r.URL.RawQuery)
	writeJSON(w, http.StatusOK, body)
}

func main() {
	addr := flag.String("addr", ":8980", "address to listen on")
	dataDir := flag.String("data", "./data", "directory containing devices.json and meta.json")
	flag.Parse()

	if _, err := os.Stat(*dataDir); err != nil {
		log.Fatalf("data directory %q not usable: %v", *dataDir, err)
	}

	s := &server{dataDir: *dataDir}
	mux := http.NewServeMux()
	// Paths match the real backend (ubibot-appcenter's Laravel routes/api.php,
	// which registers these under /api/device-library/*) so a .env pointing
	// at this mock and one pointing at production differ only in host:port,
	// never in path shape.
	mux.HandleFunc("/api/device-library/version", s.handleVersion)
	mux.HandleFunc("/api/device-library/latest", s.handleLatest)

	log.Printf("device-library mock server listening on %s (data dir: %s)", *addr, *dataDir)
	log.Printf("point the app at it via .env: DEVICE_LIBRARY_API_BASE_URL=http://<this-host>%s/api/device-library", *addr)
	if err := http.ListenAndServe(*addr, mux); err != nil {
		log.Fatal(err)
	}
}
