<?xml version="1.0" encoding="utf-8"?>
<!DOCTYPE TS>
<TS version="2.1" language="it" sourcelanguage="en">
<context>
    <name>AppController</name>
    <message>
        <source>%1 OPEN</source>
        <translation>%1 APERTA</translation>
    </message>
    <message>
        <source>%1 CLOSED</source>
        <translation>%1 CHIUSA</translation>
    </message>
    <message>
        <source>Port is not open — click &quot;Open port&quot; first.</source>
        <translation>La porta non è aperta: fai prima clic su &quot;Apri porta&quot;.</translation>
    </message>
    <message>
        <source>Select a serial port first.</source>
        <translation>Seleziona prima una porta seriale.</translation>
    </message>
    <message>
        <source>Command library is up to date (%1).</source>
        <translation>La libreria comandi è aggiornata (%1).</translation>
    </message>
    <message>
        <source>Version %1 is available, but requires app version %2 or later -- please update the app first.</source>
        <translation>La versione %1 è disponibile, ma richiede l&apos;app versione %2 o successiva: aggiorna prima l&apos;app.</translation>
    </message>
    <message>
        <source>Version %1 is available.</source>
        <translation>La versione %1 è disponibile.</translation>
    </message>
    <message>
        <source>Downloaded data could not be applied: %1</source>
        <translation>Impossibile applicare i dati scaricati: %1</translation>
    </message>
    <message>
        <source>Updated to %1.</source>
        <translation>Aggiornato a %1.</translation>
    </message>
    <message>
        <source>You&apos;re on the latest version (%1).</source>
        <translation>Stai usando l&apos;ultima versione (%1).</translation>
    </message>
    <message>
        <source>The send box isn&apos;t valid HEX, so it was left unchanged.</source>
        <translation>Il contenuto della casella di invio non è HEX valido ed è stato lasciato invariato.</translation>
    </message>
    <message>
        <source>The HEX data contains bytes that can&apos;t be shown as text, so it was left unchanged.</source>
        <translation>I dati HEX contengono byte non visualizzabili come testo e sono stati lasciati invariati.</translation>
    </message>
    <message>
        <source>%1 opened · %2</source>
        <translation>%1 aperta · %2</translation>
    </message>
    <message>
        <source>Port closed</source>
        <translation>Porta chiusa</translation>
    </message>
    <message>
        <source>Not sent: invalid HEX. Use pairs of 0-9 / A-F digits, e.g. &quot;7B 22 0D 0A&quot;.</source>
        <translation>Non inviato: HEX non valido. Usa coppie di cifre 0-9 / A-F, ad es. &quot;7B 22 0D 0A&quot;.</translation>
    </message>
    <message>
        <source>Batch &quot;%1&quot; started — %2 step(s).</source>
        <translation>Batch &quot;%1&quot; avviato — %2 passaggi.</translation>
    </message>
    <message>
        <source>Batch step %1 skipped: invalid HEX &quot;%2&quot;.</source>
        <translation>Passaggio %1 del batch saltato: HEX non valido &quot;%2&quot;.</translation>
    </message>
    <message>
        <source>Batch &quot;%1&quot; finished.</source>
        <translation>Batch &quot;%1&quot; completato.</translation>
    </message>
    <message>
        <source>Batch &quot;%1&quot; stopped.</source>
        <translation>Batch &quot;%1&quot; interrotto.</translation>
    </message>
    <message>
        <source>%1 / %2</source>
        <translation>%1 / %2</translation>
    </message>
    <message>
        <source>Log saved to %1</source>
        <translation>Registro salvato in %1</translation>
    </message>
    <message>
        <source>No update server configured (.env) -- using the bundled command library only.</source>
        <translation>Nessun server di aggiornamento configurato (.env): viene usata solo la libreria comandi integrata.</translation>
    </message>
    <message>
        <source>Checking for updates…</source>
        <translation>Ricerca aggiornamenti…</translation>
    </message>
    <message>
        <source>Downloading…</source>
        <translation>Download in corso…</translation>
    </message>
    <message>
        <source>No update source configured.</source>
        <translation>Nessuna sorgente di aggiornamento configurata.</translation>
    </message>
    <message>
        <source>Automatic installation isn&apos;t available on this platform. Opening the download page…</source>
        <translation>L&apos;installazione automatica non è disponibile su questa piattaforma. Apertura della pagina di download…</translation>
    </message>
    <message>
        <source>No release page available.</source>
        <translation>Pagina di rilascio non disponibile.</translation>
    </message>
    <message>
        <source>Downloading update…</source>
        <translation>Download dell&apos;aggiornamento…</translation>
    </message>
    <message>
        <source>Download cancelled.</source>
        <translation>Download annullato.</translation>
    </message>
    <message>
        <source>Downloaded file failed checksum verification -- update not applied.</source>
        <translation>Il file scaricato non ha superato la verifica del checksum: aggiornamento non applicato.</translation>
    </message>
    <message>
        <source>Preparing update…</source>
        <translation>Preparazione dell&apos;aggiornamento…</translation>
    </message>
    <message>
        <source>Failed to launch the updater script.</source>
        <translation>Impossibile avviare lo script di aggiornamento.</translation>
    </message>
    <message>
        <source>Restarting to apply update…</source>
        <translation>Riavvio per applicare l&apos;aggiornamento…</translation>
    </message>
    <message>
        <source>Self-update isn&apos;t supported on this platform yet -- please download the new version manually.</source>
        <translation>L&apos;aggiornamento automatico non è ancora supportato su questa piattaforma: scarica manualmente la nuova versione.</translation>
    </message>
</context>
<context>
    <name>DeviceLibraryUpdateClient</name>
    <message>
        <source>Remote update server not configured (.env).</source>
        <translation>Server di aggiornamento remoto non configurato (.env).</translation>
    </message>
    <message>
        <source>Malformed response: %1</source>
        <translation>Risposta non valida: %1</translation>
    </message>
    <message>
        <source>Checksum mismatch -- downloaded data may be corrupted.</source>
        <translation>Checksum non corrispondente: i dati scaricati potrebbero essere danneggiati.</translation>
    </message>
    <message>
        <source>Response has no models[] array.</source>
        <translation>La risposta non contiene l&apos;array models[].</translation>
    </message>
</context>
<context>
    <name>QLight::CaptionButton</name>
    <message>
        <source>Minimize</source>
        <translation>Riduci a icona</translation>
    </message>
    <message>
        <source>Maximize</source>
        <translation>Ingrandisci</translation>
    </message>
    <message>
        <source>Close</source>
        <translation>Chiudi</translation>
    </message>
    <message>
        <source>Restore</source>
        <translation>Ripristina</translation>
    </message>
</context>
<context>
    <name>QLight::Dialog</name>
    <message>
        <source>OK</source>
        <translation>OK</translation>
    </message>
    <message>
        <source>Cancel</source>
        <translation>Annulla</translation>
    </message>
</context>
<context>
    <name>QLight::InfoBar</name>
    <message>
        <source>Close</source>
        <translation>Chiudi</translation>
    </message>
</context>
<context>
    <name>QLight::TeachingTip</name>
    <message>
        <source>Close</source>
        <translation>Chiudi</translation>
    </message>
</context>
<context>
    <name>QObject</name>
    <message>
        <source>could not start &apos;tar&apos; (bundled with Windows 10 1803+/Windows 11) -- %1</source>
        <translation>impossibile avviare &apos;tar&apos; (incluso in Windows 10 1803+/Windows 11): %1</translation>
    </message>
    <message>
        <source>&apos;tar&apos; timed out extracting the update</source>
        <translation>&apos;tar&apos; è andato in timeout durante l&apos;estrazione dell&apos;aggiornamento</translation>
    </message>
    <message>
        <source>&apos;tar&apos; failed (exit %1): %2</source>
        <translation>&apos;tar&apos; non riuscito (uscita %1): %2</translation>
    </message>
    <message>
        <source>downloaded archive doesn&apos;t contain %1 -- not a valid release package</source>
        <translation>l&apos;archivio scaricato non contiene %1: non è un pacchetto di rilascio valido</translation>
    </message>
    <message>
        <source>could not write updater script to %1: %2</source>
        <translation>impossibile scrivere lo script di aggiornamento in %1: %2</translation>
    </message>
</context>
<context>
    <name>SerialManager</name>
    <message>
        <source>Serial port disconnected.</source>
        <translation>Porta seriale disconnessa.</translation>
    </message>
</context>
<context>
    <name>SerialOptions</name>
    <message>
        <source>None</source>
        <translation>Nessuna</translation>
    </message>
    <message>
        <source>Even</source>
        <translation>Pari</translation>
    </message>
    <message>
        <source>Odd</source>
        <translation>Dispari</translation>
    </message>
    <message>
        <source>RTS/CTS</source>
        <translation>RTS/CTS</translation>
    </message>
    <message>
        <source>XON/XOFF</source>
        <translation>XON/XOFF</translation>
    </message>
</context>
<context>
    <name>SoftwareUpdateClient</name>
    <message>
        <source>No update source configured.</source>
        <translation>Nessuna sorgente di aggiornamento configurata.</translation>
    </message>
    <message>
        <source>GitHub refused the request: %1</source>
        <translation>GitHub ha rifiutato la richiesta: %1</translation>
    </message>
    <message>
        <source>No published release was found.</source>
        <translation>Nessuna release pubblicata trovata.</translation>
    </message>
    <message>
        <source>Malformed GitHub response: %1</source>
        <translation>Risposta GitHub non valida: %1</translation>
    </message>
    <message>
        <source>Release tag &quot;%1&quot; is not a version number.</source>
        <translation>Il tag di rilascio &quot;%1&quot; non è un numero di versione.</translation>
    </message>
    <message>
        <source>Release %1 has no package for this platform.</source>
        <translation>La release %1 non contiene un pacchetto per questa piattaforma.</translation>
    </message>
    <message>
        <source>Release %1 has no %2 package.</source>
        <translation>La release %1 non contiene il pacchetto %2.</translation>
    </message>
    <message>
        <source>Cannot write to %1: %2</source>
        <translation>Impossibile scrivere in %1: %2</translation>
    </message>
</context>
<context>
    <name>ui::AboutDialog</name>
    <message>
        <source>About</source>
        <translation>Informazioni</translation>
    </message>
    <message>
        <source>UbiBot Serial Assistant</source>
        <translation>Assistente seriale UbiBot</translation>
    </message>
    <message>
        <source>Version %1</source>
        <translation>Versione %1</translation>
    </message>
    <message>
        <source>A serial-port debugging tool for UbiBot IoT devices (WS1, WS1 Pro, GS1-AL4G1RS, SP1, and more), built with Qt 6 and C++17.</source>
        <translation>Uno strumento di debug della porta seriale per i dispositivi IoT UbiBot (WS1, WS1 Pro, GS1-AL4G1RS, SP1 e altri), sviluppato con Qt 6 e C++17.</translation>
    </message>
    <message>
        <source>Build time</source>
        <translation>Data di build</translation>
    </message>
    <message>
        <source>Company</source>
        <translation>Azienda</translation>
    </message>
    <message>
        <source>UbiBot · United States of America</source>
        <translation>UbiBot · Stati Uniti d&apos;America</translation>
    </message>
    <message>
        <source>Website</source>
        <translation>Sito web</translation>
    </message>
    <message>
        <source>GitHub</source>
        <translation>GitHub</translation>
    </message>
    <message>
        <source>Built with Qt %1 under the GNU Lesser General Public License v3 (LGPLv3). Qt is a trademark of The Qt Company Ltd. Qt is linked dynamically, so the bundled Qt libraries may be replaced with a compatible LGPLv3 build.</source>
        <translation>Sviluppato con Qt %1 sotto licenza GNU Lesser General Public License v3 (LGPLv3). Qt è un marchio di The Qt Company Ltd. Qt è collegato dinamicamente, quindi le librerie Qt incluse possono essere sostituite con una build compatibile LGPLv3.</translation>
    </message>
    <message>
        <source>Close</source>
        <translation>Chiudi</translation>
    </message>
</context>
<context>
    <name>ui::BatchEditDialog</name>
    <message>
        <source>Edit batch command</source>
        <translation>Modifica comando in batch</translation>
    </message>
    <message>
        <source>New batch command</source>
        <translation>Nuovo comando in batch</translation>
    </message>
    <message>
        <source>Name</source>
        <translation>Nome</translation>
    </message>
    <message>
        <source>e.g. Startup sequence</source>
        <translation>es. Sequenza di avvio</translation>
    </message>
    <message>
        <source>Interval</source>
        <translation>Intervallo</translation>
    </message>
    <message>
        <source>ms</source>
        <translation>ms</translation>
    </message>
    <message>
        <source>Steps</source>
        <translation>Passaggi</translation>
    </message>
    <message>
        <source>Add step</source>
        <translation>Aggiungi passaggio</translation>
    </message>
    <message>
        <source>Cancel</source>
        <translation>Annulla</translation>
    </message>
    <message>
        <source>Save</source>
        <translation>Salva</translation>
    </message>
    <message>
        <source>Command text</source>
        <translation>Testo del comando</translation>
    </message>
    <message>
        <source>CRC</source>
        <translation>CRC</translation>
    </message>
    <message>
        <source>Remove step</source>
        <translation>Rimuovi passaggio</translation>
    </message>
    <message>
        <source>Not valid HEX, so it was left unchanged.</source>
        <translation>HEX non valido, lasciato invariato.</translation>
    </message>
    <message>
        <source>Contains bytes that can&apos;t be shown as text, so it was left unchanged.</source>
        <translation>Contiene byte non visualizzabili come testo, lasciato invariato.</translation>
    </message>
</context>
<context>
    <name>ui::BatchListDialog</name>
    <message>
        <source>Batch commands</source>
        <translation>Comandi in batch</translation>
    </message>
    <message>
        <source>No batch commands yet</source>
        <translation>Nessun comando in batch</translation>
    </message>
    <message>
        <source>New batch</source>
        <translation>Nuovo batch</translation>
    </message>
    <message>
        <source>Close</source>
        <translation>Chiudi</translation>
    </message>
    <message>
        <source>%1 steps · %2 ms interval</source>
        <translation>%1 passaggi · intervallo %2 ms</translation>
    </message>
    <message>
        <source>Stop</source>
        <translation>Ferma</translation>
    </message>
    <message>
        <source>Run</source>
        <translation>Esegui</translation>
    </message>
    <message>
        <source>Edit</source>
        <translation>Modifica</translation>
    </message>
    <message>
        <source>Delete</source>
        <translation>Elimina</translation>
    </message>
</context>
<context>
    <name>ui::CommandLibraryPanel</name>
    <message>
        <source>Device model</source>
        <translation>Modello dispositivo</translation>
    </message>
    <message>
        <source>Search commands</source>
        <translation>Cerca comandi</translation>
    </message>
    <message>
        <source>Drag a row to reorder</source>
        <translation>Trascina una riga per riordinare</translation>
    </message>
    <message>
        <source>New template</source>
        <translation>Nuovo modello</translation>
    </message>
    <message>
        <source>Batch commands</source>
        <translation>Comandi in batch</translation>
    </message>
    <message>
        <source>Edit</source>
        <translation>Modifica</translation>
    </message>
    <message>
        <source>Delete</source>
        <translation>Elimina</translation>
    </message>
</context>
<context>
    <name>ui::CommandParamsPanel</name>
    <message>
        <source>Needs parameters</source>
        <translation>Richiede parametri</translation>
    </message>
    <message>
        <source>Preview</source>
        <translation>Anteprima</translation>
    </message>
    <message>
        <source>Cancel</source>
        <translation>Annulla</translation>
    </message>
    <message>
        <source>Insert</source>
        <translation>Inserisci</translation>
    </message>
</context>
<context>
    <name>ui::DataMonitorView</name>
    <message>
        <source>Data monitor</source>
        <translation>Monitor dati</translation>
    </message>
    <message>
        <source>Data sent and received appears here once a port is open</source>
        <translation>I dati inviati e ricevuti compaiono qui dopo l&apos;apertura di una porta</translation>
    </message>
    <message>
        <source>%1 lines</source>
        <translation>%1 righe</translation>
    </message>
    <message>
        <source>Rx %1</source>
        <translation>Rx %1</translation>
    </message>
    <message>
        <source>Tx %1</source>
        <translation>Tx %1</translation>
    </message>
    <message>
        <source>Copy</source>
        <translation>Copia</translation>
    </message>
    <message>
        <source>Clear log</source>
        <translation>Cancella registro</translation>
    </message>
    <message>
        <source>Save log…</source>
        <translation>Salva registro…</translation>
    </message>
</context>
<context>
    <name>ui::DriverHintDialog</name>
    <message>
        <source>Serial driver may be missing</source>
        <translation>Il driver della porta seriale potrebbe mancare</translation>
    </message>
    <message>
        <source>The CH340 USB-to-serial driver doesn&apos;t appear to be installed on this computer. Without it, a connected UbiBot device won&apos;t show up as an available serial port.</source>
        <translation>Il driver USB-seriale CH340 non sembra essere installato su questo computer. Senza di esso, un dispositivo UbiBot collegato non verrà visualizzato come porta seriale disponibile.</translation>
    </message>
    <message>
        <source>Don&apos;t show this again</source>
        <translation>Non mostrare più</translation>
    </message>
    <message>
        <source>Download driver</source>
        <translation>Scarica driver</translation>
    </message>
    <message>
        <source>Close</source>
        <translation>Chiudi</translation>
    </message>
</context>
<context>
    <name>ui::DriverListDialog</name>
    <message>
        <source>USB-to-serial driver used by most UbiBot devices</source>
        <translation>Driver USB-seriale utilizzato dalla maggior parte dei dispositivi UbiBot</translation>
    </message>
    <message>
        <source>Driver downloads</source>
        <translation>Download driver</translation>
    </message>
    <message>
        <source>Close</source>
        <translation>Chiudi</translation>
    </message>
    <message>
        <source>No drivers are relevant on this platform.</source>
        <translation>Nessun driver rilevante per questa piattaforma.</translation>
    </message>
    <message>
        <source>Installed</source>
        <translation>Installato</translation>
    </message>
    <message>
        <source>Not detected</source>
        <translation>Non rilevato</translation>
    </message>
    <message>
        <source>Download</source>
        <translation>Scarica</translation>
    </message>
</context>
<context>
    <name>ui::LibraryUpdateDialog</name>
    <message>
        <source>Command library</source>
        <translation>Libreria comandi</translation>
    </message>
    <message>
        <source>Download and apply</source>
        <translation>Scarica e applica</translation>
    </message>
    <message>
        <source>OK</source>
        <translation>OK</translation>
    </message>
</context>
<context>
    <name>ui::MainView</name>
    <message>
        <source>Failed to open port</source>
        <translation>Apertura della porta non riuscita</translation>
    </message>
    <message>
        <source>OK</source>
        <translation>OK</translation>
    </message>
    <message>
        <source>&amp;File</source>
        <translation>&amp;File</translation>
    </message>
    <message>
        <source>Save log</source>
        <translation>Salva registro</translation>
    </message>
    <message>
        <source>Exit</source>
        <translation>Esci</translation>
    </message>
    <message>
        <source>&amp;Edit</source>
        <translation>&amp;Modifica</translation>
    </message>
    <message>
        <source>Clear</source>
        <translation>Cancella</translation>
    </message>
    <message>
        <source>&amp;View</source>
        <translation>&amp;Visualizza</translation>
    </message>
    <message>
        <source>Pause scrolling</source>
        <translation>Sospendi scorrimento</translation>
    </message>
    <message>
        <source>&amp;Tools</source>
        <translation>&amp;Strumenti</translation>
    </message>
    <message>
        <source>Settings</source>
        <translation>Impostazioni</translation>
    </message>
    <message>
        <source>&amp;Help</source>
        <translation>&amp;Aiuto</translation>
    </message>
    <message>
        <source>Check for software update</source>
        <translation>Verifica aggiornamenti</translation>
    </message>
    <message>
        <source>Driver downloads</source>
        <translation>Download driver</translation>
    </message>
    <message>
        <source>About</source>
        <translation>Informazioni</translation>
    </message>
    <message>
        <source>Serial driver not installed, so devices may not be detected.</source>
        <translation>Driver seriale non installato: i dispositivi potrebbero non essere rilevati.</translation>
    </message>
    <message>
        <source>View details</source>
        <translation>Visualizza dettagli</translation>
    </message>
    <message>
        <source>Serial</source>
        <translation>Seriale</translation>
    </message>
    <message>
        <source>Device commands</source>
        <translation>Comandi dispositivo</translation>
    </message>
    <message>
        <source>Type data to send…</source>
        <translation>Digita i dati da inviare…</translation>
    </message>
    <message>
        <source>Send</source>
        <translation>Invia</translation>
    </message>
    <message>
        <source>History</source>
        <translation>Cronologia</translation>
    </message>
    <message>
        <source>Connected · %1</source>
        <translation>Connesso · %1</translation>
    </message>
    <message>
        <source>Not connected</source>
        <translation>Non connesso</translation>
    </message>
    <message>
        <source>Send history</source>
        <translation>Cronologia invii</translation>
    </message>
    <message>
        <source>Clear history</source>
        <translation>Cancella cronologia</translation>
    </message>
    <message>
        <source>No history yet</source>
        <translation>Nessuna cronologia ancora</translation>
    </message>
</context>
<context>
    <name>ui::MainWindow</name>
    <message>
        <source>UbiBot Serial Assistant</source>
        <translation>Assistente seriale UbiBot</translation>
    </message>
</context>
<context>
    <name>ui::SaveLogDialog</name>
    <message>
        <source>Choose a folder</source>
        <translation>Scegli una cartella</translation>
    </message>
    <message>
        <source>Save session log</source>
        <translation>Salva registro sessione</translation>
    </message>
    <message>
        <source>File name</source>
        <translation>Nome file</translation>
    </message>
    <message>
        <source>Location</source>
        <translation>Posizione</translation>
    </message>
    <message>
        <source>Format</source>
        <translation>Formato</translation>
    </message>
    <message>
        <source>Browse…</source>
        <translation>Sfoglia…</translation>
    </message>
    <message>
        <source>Plain text (.log)</source>
        <translation>Testo semplice (.log)</translation>
    </message>
    <message>
        <source>HEX dump</source>
        <translation>Dump HEX</translation>
    </message>
    <message>
        <source>Continue logging to disk, rotating the file every day</source>
        <translation>Continua a registrare su disco, ruotando il file ogni giorno</translation>
    </message>
    <message>
        <source>Cancel</source>
        <translation>Annulla</translation>
    </message>
    <message>
        <source>Save</source>
        <translation>Salva</translation>
    </message>
</context>
<context>
    <name>ui::SerialSettingsPanel</name>
    <message>
        <source>Port settings</source>
        <translation>Impostazioni porta</translation>
    </message>
    <message>
        <source>Port</source>
        <translation>Porta</translation>
    </message>
    <message>
        <source>Baud rate</source>
        <translation>Velocità (baud)</translation>
    </message>
    <message>
        <source>Data bits</source>
        <translation>Bit di dati</translation>
    </message>
    <message>
        <source>Parity</source>
        <translation>Parità</translation>
    </message>
    <message>
        <source>Stop bits</source>
        <translation>Bit di stop</translation>
    </message>
    <message>
        <source>Flow control</source>
        <translation>Controllo di flusso</translation>
    </message>
    <message>
        <source>No ports found</source>
        <translation>Nessuna porta trovata</translation>
    </message>
    <message>
        <source>Refresh ports</source>
        <translation>Aggiorna porte</translation>
    </message>
    <message>
        <source>Receive</source>
        <translation>Ricezione</translation>
    </message>
    <message>
        <source>Show timestamp</source>
        <translation>Mostra data/ora</translation>
    </message>
    <message>
        <source>Wrap lines</source>
        <translation>A capo automatico</translation>
    </message>
    <message>
        <source>Echo sent data</source>
        <translation>Mostra i dati inviati</translation>
    </message>
    <message>
        <source>Transmit</source>
        <translation>Trasmissione</translation>
    </message>
    <message>
        <source>Append CRC (CRC16/MODBUS)</source>
        <translation>Aggiungi CRC (CRC16/MODBUS)</translation>
    </message>
    <message>
        <source>Repeat send</source>
        <translation>Invio ripetuto</translation>
    </message>
    <message>
        <source>ms</source>
        <translation>ms</translation>
    </message>
    <message>
        <source>Unknown device</source>
        <translation>Dispositivo sconosciuto</translation>
    </message>
    <message>
        <source>Close port</source>
        <translation>Chiudi porta</translation>
    </message>
    <message>
        <source>Open port</source>
        <translation>Apri porta</translation>
    </message>
</context>
<context>
    <name>ui::SettingsDialog</name>
    <message>
        <source>Settings &amp; About</source>
        <translation>Impostazioni e informazioni</translation>
    </message>
    <message>
        <source>Interface language</source>
        <translation>Lingua dell&apos;interfaccia</translation>
    </message>
    <message>
        <source>System font</source>
        <translation>Carattere di sistema</translation>
    </message>
    <message>
        <source>Size</source>
        <translation>Dimensione</translation>
    </message>
    <message>
        <source>Data monitor font</source>
        <translation>Carattere monitor dati</translation>
    </message>
    <message>
        <source>Theme</source>
        <translation>Tema</translation>
    </message>
    <message>
        <source>Light</source>
        <translation>Chiaro</translation>
    </message>
    <message>
        <source>Dark</source>
        <translation>Scuro</translation>
    </message>
    <message>
        <source>Command library</source>
        <translation>Libreria comandi</translation>
    </message>
    <message>
        <source>Check for updates</source>
        <translation>Controlla aggiornamenti</translation>
    </message>
    <message>
        <source>Version</source>
        <translation>Versione</translation>
    </message>
    <message>
        <source>Platform</source>
        <translation>Piattaforma</translation>
    </message>
    <message>
        <source>Support</source>
        <translation>Assistenza</translation>
    </message>
    <message>
        <source>Devices</source>
        <translation>Dispositivi</translation>
    </message>
    <message>
        <source>Restore defaults</source>
        <translation>Ripristina valori predefiniti</translation>
    </message>
    <message>
        <source>Close</source>
        <translation>Chiudi</translation>
    </message>
    <message>
        <source>%1 (Qt %2)</source>
        <translation>%1 (Qt %2)</translation>
    </message>
    <message>
        <source>Up to date</source>
        <translation>Aggiornato</translation>
    </message>
    <message>
        <source>Update available</source>
        <translation>Aggiornamento disponibile</translation>
    </message>
    <message>
        <source>%1 models · %2 commands</source>
        <translation>%1 modelli · %2 comandi</translation>
    </message>
</context>
<context>
    <name>ui::SoftwareUpdateDialog</name>
    <message>
        <source>Software Update</source>
        <translation>Aggiornamento software</translation>
    </message>
    <message>
        <source>Later</source>
        <translation>Più tardi</translation>
    </message>
    <message>
        <source>Cancel</source>
        <translation>Annulla</translation>
    </message>
    <message>
        <source>Retry</source>
        <translation>Riprova</translation>
    </message>
    <message>
        <source>Update now</source>
        <translation>Aggiorna ora</translation>
    </message>
    <message>
        <source>OK</source>
        <translation>OK</translation>
    </message>
    <message>
        <source>Version %1 is available (you have %2)</source>
        <translation>È disponibile la versione %1 (hai la %2)</translation>
    </message>
</context>
<context>
    <name>ui::TemplateDialog</name>
    <message>
        <source>Edit template</source>
        <translation>Modifica modello</translation>
    </message>
    <message>
        <source>New template</source>
        <translation>Nuovo modello</translation>
    </message>
    <message>
        <source>Name</source>
        <translation>Nome</translation>
    </message>
    <message>
        <source>e.g. Reset device</source>
        <translation>es. Riavvia dispositivo</translation>
    </message>
    <message>
        <source>Content</source>
        <translation>Contenuto</translation>
    </message>
    <message>
        <source>The literal text to send</source>
        <translation>Il testo esatto da inviare</translation>
    </message>
    <message>
        <source>Cancel</source>
        <translation>Annulla</translation>
    </message>
    <message>
        <source>Save</source>
        <translation>Salva</translation>
    </message>
</context>
</TS>
