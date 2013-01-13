#define APPN     "FloodFill"
#define VCREDISTDIR AddBackslash(GetEnv("VS110COMNTOOLS")) + \
                    "..\..\VC\redist\x86\Microsoft.VC110.CRT"

#define SFMLDIR AddBackslash(GetEnv("SFMLDIR")) + "bin"
#define EXECUTABLE  "..\build11\src\Release\jd.exe"

#define tmp AddBackslash(GetEnv("TMP")) 
#define GAMENAME "floodfill.jd"
#define GAME tmp + GAMENAME
#define BASEDATA tmp + "base.jd"
#define DOCDIR "..\doc"

#if !FileExists(GAME)
#   error Run ..\buildInstaller.bat to create the installer.
#endif

#define VERSION GetFileVersion(EXECUTABLE)
#define VERSIONSTR GetStringFileInfo(EXECUTABLE, PRODUCT_VERSION)
#define AUTHOR GetStringFileInfo(EXECUTABLE, COMPANY_NAME)
#define COPYRIGHT GetStringFileInfo(EXECUTABLE, LEGAL_COPYRIGHT)

[Setup]
AppName             = "{#APPN}"
AppVersion          = "{#VERSIONSTR}"
; Version Beta 0.x sounds odd. Just display Beta 0.x instead.
AppVerName          = "{#APPN} {#VERSIONSTR}"
DefaultDirName      = "{pf}\{#APPN}"
DefaultGroupName    = "{#APPN}"
AppPublisher        = "{#AUTHOR}"
AppReadmeFile       = "{app}\usermanual.pdf"
; Needs at least Windows Vista
MinVersion          = 6.0 
UninstallDisplayIcon= "{app}\{#APPN}.ico"
AppCopyright        = "{#COPYRIGHT}"

VersionInfoVersion        = "{#VERSION}"
VersionInfoCompany        = "{#AUTHOR}"
VersionInfoDescription    = "{#APPN} Installer"
VersionInfoTextVersion    = "{#VERSIONSTR}"
VersionInfoProductName    = "{#APPN}"
VersionInfoProductVersion = "{#VERSION}"
VersionInfoProductTextVersion = "{#VERSIONSTR}"

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
Source: "floodfill.ico"; DestDir: "{app}"
Source: "{#GAME}"; DestDir: "{app}"
Source: "{#BASEDATA}"; DestDir: "{app}"
Source: "{#DOCDIR}\usermanual.pdf"; DestDir: "{app}"


#define PARAMS 'Parameters: """{app}\' + GAMENAME + '"""'
[Run]
Filename: "{app}\jd.exe"; \
    Flags: nowait postinstall runasoriginaluser skipifsilent; \
    Description: "FloodFill starten"; {#PARAMS}
    
Filename: "{app}\usermanual.pdf"; \
    Flags: postinstall shellexec runasoriginaluser skipifsilent; \
    Description: "Benutzerhandbuch lesen"

#define ICONSETTINGS 'Filename: "{app}\jd.exe"; WorkingDir: "{app}";' +  \
                     'IconFilename: "{app}\floodfill.ico"; ' + PARAMS

[Icons]
Name: "{group}\{#APPN}"; {#ICONSETTINGS}
Name: "{commondesktop}\{#APPN}"; {#ICONSETTINGS}; Tasks: desktopicon
Name: "{group}\{#APPN} Benutzerhandbuch"; Filename: "{app}\usermanual.pdf"
Name: "{group}\{#APPN} entfernen"; Filename: "{uninstallexe}"
