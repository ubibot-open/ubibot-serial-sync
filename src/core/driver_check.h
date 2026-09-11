#pragma once

#include <QList>
#include <QString>

// Detects whether the WCH CH340/CH341 USB-to-serial driver -- the chip
// UbiBot devices actually ship with (see SerialManager::PortHint) -- is
// installed on this machine. Purely "is the driver present at all", not
// "is a CH340 device currently plugged in": QSerialPortInfo (what
// SerialManager::availablePorts() is built on) only ever sees a device once
// its driver has already bound it to a port, so it can't distinguish "no
// device connected" from "device connected but the driver was never
// installed, so the OS never gave it a port" -- the second case is exactly
// the support complaint this exists to catch (see AppController::openPort()
// and docs/ch340-driver-hint.md).
//
// Checks for the driver's own installed file(s) directly (Windows: the
// .SYS under System32\drivers; macOS: the /Library/Extensions kext) rather
// than any registry/service lookup -- same detection UbiBot's existing
// Electron app already uses in production, ported here rather than
// re-derived, so this behaves identically to what users on that app
// already experience.
namespace DriverCheck {

// True on Linux (the in-tree ch341 kernel module needs no separate
// install, see docs/ch340-driver-hint.md) and on any platform this hasn't
// been ported to -- nothing meaningful to check there.
bool isCh340DriverInstalled();

// Where DriverHintDialog.qml's "Download driver" button should send the
// user -- a real, hardcoded-per-platform URL (Windows and macOS each have
// their own download page), not read from .env: unlike the device-library/
// software-update server URLs (which genuinely differ per deployment and
// so *need* to be runtime-configurable), these are just two fixed public
// download links. Hardcoding them means they're compiled straight into the
// binary and are there in a GitHub Actions-built release exactly the same
// as in a local dev build -- a .env-based value would have silently been
// empty in any shipped build, since .env is deliberately never committed
// (see .gitignore) and CI never creates one. Empty on any platform other
// than Windows/macOS (Linux never reaches DriverHintDialog.qml at all, see
// isCh340DriverInstalled() above).
QString downloadUrl();

// One row of DriverListDialog.qml's own driver list (Help menu -> "Driver
// downloads", also what DriverHintDialog.qml's "Download driver" button
// opens instead of jumping straight to a single URL -- see
// docs/ch340-driver-hint.md).
struct DriverInfo {
    // Shown as the row's title -- not translated here (plain product/chip
    // naming, same treatment as e.g. "CRC"/"XON/XOFF" elsewhere in this
    // app), qsTr()'d in QML only for description below.
    QString name;
    // Translatable one-line description of what the driver is for --
    // supplied as an already-qsTr()'d string isn't possible from C++ in a
    // way that re-translates live on a language switch, so this returns a
    // translation *key* (an English string), and DriverListDialog.qml
    // wraps it in its own qsTr() call to actually localize it.
    QString description;
    // Empty when this platform has no known download link for this driver
    // (DriverListDialog.qml hides the row's Download button in that case).
    QString url;
    bool installed;
};

// All drivers this app knows how to check for/help install, in display
// order. Just one entry today (CH340) -- structured as a list specifically
// so a future driver (CP210x, FTDI, ...) is "add one more entry to this
// function", not a parallel one-off mechanism repeating
// isCh340DriverInstalled()/downloadUrl()'s own AppController plumbing.
QList<DriverInfo> knownDrivers();

}  // namespace DriverCheck
