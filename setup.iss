#ifndef MyAppVersion
  #define MyAppVersion "0.1.0"
#endif
#ifndef MyAppBuild
  #define MyAppBuild "1"
#endif
#ifndef SourceDir
  #define SourceDir "dist\ADB-Studio"
#endif
#ifndef OutputDir
  #define OutputDir "dist\installer"
#endif
#ifndef SetupIcon
  #define SetupIcon "assets\app-icon-adb.ico"
#endif

[Setup]
AppId={{6F845ED6-DC56-4F3B-B874-10C43CC4E506}
AppName=ADB Studio
AppVersion={#MyAppVersion}
AppPublisher=ADB Studio Community
AppPublisherURL=https://github.com/CRTYPUBG/adb-studio
AppSupportURL=https://github.com/CRTYPUBG/adb-studio/issues
AppUpdatesURL=https://github.com/CRTYPUBG/adb-studio/releases
DefaultDirName={localappdata}\Programs\ADB Studio
DefaultGroupName=ADB Studio
DisableProgramGroupPage=yes
PrivilegesRequired=lowest
ArchitecturesAllowed=x64compatible
ArchitecturesInstallIn64BitMode=x64compatible
OutputDir={#OutputDir}
OutputBaseFilename=ADB-Studio-Setup-{#MyAppVersion}-x64
SetupIconFile={#SetupIcon}
UninstallDisplayIcon={app}\adb-studio.exe
Compression=lzma2/ultra64
SolidCompression=yes
WizardStyle=modern
CloseApplications=yes
RestartApplications=no
VersionInfoVersion={#MyAppVersion}.{#MyAppBuild}
VersionInfoCompany=ADB Studio Community
VersionInfoDescription=ADB Studio Installer
VersionInfoProductName=ADB Studio
VersionInfoProductVersion={#MyAppVersion}.{#MyAppBuild}

[Languages]
Name: "english"; MessagesFile: "compiler:Default.isl"
Name: "turkish"; MessagesFile: "compiler:Languages\Turkish.isl"

[Tasks]
Name: "desktopicon"; Description: "{cm:CreateDesktopIcon}"; GroupDescription: "{cm:AdditionalIcons}"; Flags: unchecked

[Files]
Source: "{#SourceDir}\*"; DestDir: "{app}"; Flags: ignoreversion recursesubdirs createallsubdirs

[Icons]
Name: "{group}\ADB Studio"; Filename: "{app}\adb-studio.exe"
Name: "{autodesktop}\ADB Studio"; Filename: "{app}\adb-studio.exe"; Tasks: desktopicon

[Run]
Filename: "{app}\adb-studio.exe"; Description: "{cm:LaunchProgram,ADB Studio}"; Flags: nowait postinstall skipifsilent
