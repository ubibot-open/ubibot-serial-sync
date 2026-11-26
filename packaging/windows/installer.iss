; Inno Setup 6 script for the Windows installer.
;
; Built by .github/workflows/release.yml right after windeployqt assembles
; deploy\UbiBotSerialAssistant, and published next to the portable .zip. The
; installer and the zip contain the same files -- the zip is kept because
; (a) it still works as a no-install portable copy and (b) the in-app
; self-updater downloads the .zip, never this installer.
;
; Everything that varies per build is passed with ISCC /D defines:
;   /DAppVersion=0.8.2.1
;   /DSourceDir=C:\path\to\deploy\UbiBotSerialAssistant
;   /DOutputDir=C:\path\to\out
;   /DOutputBaseFilename=UbiBotSerialAssistant-0.8.2.1-windows-x64-setup
; The #ifndef defaults below let the script be compiled with no arguments
; from the repository for a local sanity check (see BUILD.md).
;
; Install scope: PER-USER, deliberately. `PrivilegesRequired=lowest` makes
; {autopf} resolve to %LOCALAPPDATA%\Programs, so Setup needs no UAC prompt,
; and -- the actual reason -- the installation directory stays writable by the
; user, which is what the in-app self-updater's robocopy needs (see
; docs/app-self-update.md). A machine-wide install under Program Files would
; require elevation that the updater doesn't request.

#ifndef AppVersion
  #define AppVersion "0.0.0.0"
#endif
#ifndef SourceDir
  #define SourceDir "..\..\deploy\UbiBotSerialAssistant"
#endif
#ifndef OutputDir
  #define OutputDir "."
#endif
#ifndef OutputBaseFilename
  #define OutputBaseFilename "UbiBotSerialAssistant-windows-x64-setup"
#endif

#define AppName "UbiBot Serial Assistant"
#define AppExeName "UbiBotSerialAssistant.exe"
#define AppPublisher "UbiBot"
#define AppURL "https://www.ubibot.com"
#define AppReleasesURL "https://github.com/ubibot-open/ubibot-serial-sync/releases"
; Stable, never change: it is how Windows/Inno recognise an existing
; installation (upgrade in place instead of a second Add/Remove entry).
#define AppId "{{F2672482-7D5A-4FA7-9BA5-850E01DFC7D8}"

[Setup]
AppId={#AppId}
AppName={#AppName}
AppVersion={#AppVersion}
AppVerName={#AppName} {#AppVersion}
AppPublisher={#AppPublisher}
AppPublisherURL={#AppURL}
AppSupportURL={#AppURL}
AppUpdatesURL={#AppReleasesURL}
VersionInfoVersion={#AppVersion}
VersionInfoCompany={#AppPublisher}
VersionInfoDescription={#AppName} Setup
VersionInfoProductName={#AppName}
VersionInfoProductVersion={#AppVersion}
DefaultDirName={autopf}\{#AppName}
DefaultGroupName={#AppName}
DisableProgramGroupPage=yes
PrivilegesRequired=lowest
OutputDir={#OutputDir}
OutputBaseFilename={#OutputBaseFilename}
SetupIconFile=..\..\resources\icons\app.ico
UninstallDisplayIcon={app}\{#AppExeName}
UninstallDisplayName={#AppName}
Compression=lzma2/ultra64
SolidCompression=yes
WizardStyle=modern
; 6.2-compatible spelling on purpose (x64compatible needs Inno 6.3+, and the
; version on a given CI image isn't pinned here).
ArchitecturesAllowed=x64
ArchitecturesInstallIn64BitMode=x64
MinVersion=10.0
; The app holds this mutex for its whole lifetime (see src/main.cpp), so
; Setup can tell it's running and ask the user to close it instead of hitting
; locked files halfway through.
AppMutex=UbiBotSerialAssistant.InstallerMutex
CloseApplications=yes
RestartApplications=no
AllowNoIcons=yes
ChangesAssociations=no

[Languages]
Name: "english"; MessagesFile: "compiler:Default.isl"

[Tasks]
Name: "desktopicon"; Description: "{cm:CreateDesktopIcon}"; GroupDescription: "{cm:AdditionalIcons}"; Flags: unchecked

[Files]
; recursesubdirs: windeployqt puts plugins in subdirectories (platforms\,
; styles\, imageformats\, tls\, ...) -- they must land under {app} as-is.
Source: "{#SourceDir}\*"; DestDir: "{app}"; Flags: ignoreversion recursesubdirs createallsubdirs

[Icons]
Name: "{autoprograms}\{#AppName}"; Filename: "{app}\{#AppExeName}"
Name: "{autodesktop}\{#AppName}"; Filename: "{app}\{#AppExeName}"; Tasks: desktopicon

[Run]
Filename: "{app}\{#AppExeName}"; Description: "{cm:LaunchProgram,{#StringChange(AppName, '&', '&&')}}"; Flags: nowait postinstall skipifsilent

; No [UninstallDelete] for {app} on purpose: the user's own ".env" and any
; other files they dropped next to the exe aren't ours to delete. Inno still
; removes every file it installed.

[Code]
function InitializeSetup(): Boolean;
begin
  Result := True;
  // Qt's official Windows build imports icuuc.dll, which is a pure forwarder
  // to the OS's own icu.dll (shipped with Windows 10 1703 and later). On older
  // or heavily stripped Windows images the app can't even load and the user
  // only sees a bare "icu.dll was not found" from the loader -- fail here,
  // where a readable explanation fits. The MSVC runtime is app-local (see the
  // workflow), so it isn't checked here.
  if not FileExists(ExpandConstant('{sys}\icu.dll')) then
  begin
    // Silent installs have nobody to read a message box; just abort there.
    if not WizardSilent then
      MsgBox('UbiBot Serial Assistant needs Windows 10 version 1703 or later.'
             + #13#10 + #13#10
             + 'This PC is missing the Windows ICU library (icu.dll), which the app requires.'
             + #13#10 + #13#10
             + 'UbiBot 串口助手需要 Windows 10 1703 或更高版本：本机缺少系统组件 icu.dll。',
             mbCriticalError, MB_OK);
    Result := False;
  end;
end;
