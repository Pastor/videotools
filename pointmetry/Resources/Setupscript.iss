; Script generated by the Inno Setup Script Wizard.
; SEE THE DOCUMENTATION FOR DETAILS ON CREATING INNO SETUP SCRIPT FILES!

#define MyAppName "Pointmetry"
#define MyAppVersion "1.0.0.0"
#define MyAppPublisher "NIICBT"
#define MyAppURL "http://biometric.bmstu.ru"
#define MyAppExeName "Pointmetry.exe"

#define OpencvBinPath "C:\opencv248\build\x86\vc10\bin"
#define QtBinPath "C:\Qt\5.4\msvc2010_opengl\bin"
#define stasmPath "C:\Programming\videotools\3rdparty\stasm"

[Setup]
; NOTE: The value of AppId uniquely identifies this application.
; Do not use the same AppId value in installers for other applications.
; (To generate a new GUID, click Tools | Generate GUID inside the IDE.)
AppId={{CF11716E-8552-42CF-B084-B16B6FEF8CD6}
AppName={#MyAppName}
AppVersion={#MyAppVersion}
;AppVerName={#MyAppName} {#MyAppVersion}
AppPublisher={#MyAppPublisher}
AppPublisherURL={#MyAppURL}
AppSupportURL={#MyAppURL}
AppUpdatesURL={#MyAppURL}
DefaultDirName={pf}\{#MyAppName}
DefaultGroupName={#MyAppName}
OutputDir=C:\Programming\videotools\pointmetry\Install
OutputBaseFilename=SETUP_{#MyAppName}_v{#MyAppVersion}
Compression=lzma
SolidCompression=yes

[Languages]
Name: "english"; MessagesFile: "compiler:Default.isl"
Name: "russian"; MessagesFile: "compiler:Languages\Russian.isl"

[Tasks]
Name: "desktopicon"; Description: "{cm:CreateDesktopIcon}"; GroupDescription: "{cm:AdditionalIcons}"; Flags: unchecked

[Files]
Source: "C:\Programming\videotools\pointmetry\Install\build-Sources-Desktop_Qt_5_4_MSVC2010_OpenGL_32bit-Release\release\Pointmetry.exe"; DestDir: "{app}"; Flags: ignoreversion
Source: "{#OpencvBinPath}\opencv_core248.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "{#OpencvBinPath}\opencv_ffmpeg248.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "{#OpencvBinPath}\opencv_highgui248.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "{#OpencvBinPath}\opencv_imgproc248.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "{#OpencvBinPath}\opencv_objdetect248.dll"; DestDir: "{app}"; Flags: ignoreversion

Source: "{#QtBinPath}\icudt53.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "{#QtBinPath}\icuin53.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "{#QtBinPath}\icuuc53.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "{#QtBinPath}\enginio.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "{#QtBinPath}\Qt5Concurrent.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "{#QtBinPath}\Qt5Core.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "{#QtBinPath}\Qt5Gui.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "{#QtBinPath}\Qt5Multimedia.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "{#QtBinPath}\Qt5MultimediaWidgets.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "{#QtBinPath}\Qt5Network.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "{#QtBinPath}\Qt5OpenGL.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "{#QtBinPath}\Qt5Widgets.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "{#QtBinPath}\..\plugins\platforms\qminimal.dll"; DestDir: "{app}\platforms"; Flags: ignoreversion
Source: "{#QtBinPath}\..\plugins\platforms\qoffscreen.dll"; DestDir: "{app}\platforms"; Flags: ignoreversion
Source: "{#QtBinPath}\..\plugins\platforms\qwindows.dll"; DestDir: "{app}\platforms"; Flags: ignoreversion
Source: "{#QtBinPath}\..\plugins\mediaservice\dsengine.dll"; DestDir: "{app}\mediaservice"; Flags: ignoreversion
Source: "{#QtBinPath}\..\plugins\mediaservice\qtmedia_audioengine.dll"; DestDir: "{app}\mediaservice"; Flags: ignoreversion
Source: "{#QtBinPath}\..\plugins\mediaservice\wmfengine.dll"; DestDir: "{app}\mediaservice"; Flags: ignoreversion

Source: "{#stasmPath}\haarcascade_frontalface_alt2.xml"; DestDir: "{app}\stasm"; Flags: ignoreversion
; NOTE: Don't use "Flags: ignoreversion" on any shared system files

[Icons]
Name: "{group}\{#MyAppName}"; Filename: "{app}\{#MyAppExeName}"
Name: "{group}\{cm:ProgramOnTheWeb,{#MyAppName}}"; Filename: "{#MyAppURL}"
Name: "{group}\{cm:UninstallProgram,{#MyAppName}}"; Filename: "{uninstallexe}"
Name: "{commondesktop}\{#MyAppName}"; Filename: "{app}\{#MyAppExeName}"; Tasks: desktopicon

[Run]
Filename: "{app}\{#MyAppExeName}"; Description: "{cm:LaunchProgram,{#StringChange(MyAppName, '&', '&&')}}"; Flags: nowait postinstall skipifsilent

