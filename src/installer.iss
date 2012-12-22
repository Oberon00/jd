#define APPN     "FloodFill"
#define VCREDISTDIR AddBackslash(GetEnv("VS110COMNTOOLS")) + \
                    "..\..\VC\redist\x86\Microsoft.VC110.CRT"

#define SFMLDIR AddBackslash(GetEnv("SFMLDIR")) + "bin"
#define EXECUTABLE  "..\build11\src\Release\jd.exe"

#define tmp AddBackslash(GetEnv("TMP")) 
#define GAMENAME "floodfill.jd"
#define GAME tmp + GAMENAME
#define BASEDATA tmp + "base.jd"

#if !FileExists(GAME)
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
VersionInfoProductVersion = "{#VERSION}"
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
Source: "floodfill.ico"; DestDir: "{app}"
Source: "{#GAME}"; DestDir: "{app}"
Source: "{#BASEDATA}"; DestDir: "{app}"


#define PARAMS 'Parameters: """{app}\' + GAMENAME + '"""'
[Run]
Filename: "{app}\jd.exe";   Flags: postinstall runasoriginaluser; \
  Description: "FloodFill starten"; {#PARAMS}
  
#define ICONSETTINGS 'Filename: "{app}\jd.exe"; WorkingDir: "{app}";' + \
                     'IconFilename: "{app}\floodfill.ico"; ' + PARAMS
                      
[Icons]
Name: "{group}\{#APPN}"; {#ICONSETTINGS}
Name: "{commondesktop}\{#APPN}"; {#ICONSETTINGS}; Tasks: desktopicon
Name: "{group}\{#APPN} entfernen"; Filename: "{uninstallexe}"
