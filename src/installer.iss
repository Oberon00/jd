#define APPN     "FloodFill"
#define VCREDISTDIR AddBackslash(GetEnv("VS110COMNTOOLS")) + \
                    "..\..\VC\redist\x86\Microsoft.VC110.CRT"

#define SFMLDIR AddBackslash(GetEnv("SFMLDIR")) + "bin"
#define EXECUTABLE  AddBackslash(GetEnv("TMP")) + "FloodFill.exe"

#if !FileExists(EXECUTABLE)
#   error Run ..\buildInstaller.bat to create the installer.
#endif

#define VERSION GetFileVersion(EXECUTABLE)

[Setup]
AppName             = "{#APPN}"
AppVersion          = "{#VERSION}"
DefaultDirName      = "{pf}\{#APPN}"
DefaultGroupName    = "{#APPN}"
AppPublisher        = "Christian Neumüller"

VersionInfoVersion        = "{#VERSION}"
VersionInfoCompany        = "Christian Neumüller"
VersionInfoDescription    = "{#APPN} Installer"
VersionInfoTextVersion    = "{#VERSION}"
VersionInfoProductName    = "{#APPN}"
VersionInfoProductVersion ="{#VERSION}"
VersionInfoCopyright      = "© Christian Neumüller 2012"

[Tasks]
Name: desktopicon; Description: "Desktopverknüpfung erstellen"; \
  GroupDescription: "Zusätzliche Verknüpfungen:"

[Languages]
Name: "de"; MessagesFile: "compiler:Languages\German.isl"


[Files]
Source: "{#EXECUTABLE}"; DestDir: "{app}"
Source: "{#SFMLDIR}\libsndfile-1.dll"; DestDir: "{app}"
Source: "{#SFMLDIR}\openal32.dll"; DestDir: "{app}"
Source: "{#VCREDISTDIR}\msvc*.dll"; DestDir: "{app}"

[Run]
Filename: "{app}\{#APPN}.exe";   Flags: postinstall runasoriginaluser; \
  Description: "FloodFill starten"

[Icons]
Name: "{group}\{#APPN}"; Filename: "{app}\{#APPN}.exe"; WorkingDir: "{app}"
Name: "{commondesktop}\{#APPN}"; Filename: "{app}\{#APPN}.exe"; \
  WorkingDir: "{app}"; Tasks: desktopicon
Name: "{group}\{#APPN} entfernen"; Filename: "{uninstallexe}"
