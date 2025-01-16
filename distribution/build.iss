
#include "temp\config.iss"

; Should be defined in config file
;#define SVIEW_VER "12.7.0.0"
;#define SVIEW_VER_FULL "v.12.00alpha00"
;#define SVIEW_VER_NAME "sView (12.00alpha00)"
;#define SVIEW_DISTR_PATH "sView_AMD64"

#define SVIEW_NAME "sView"
#define SVIEW_PUBLISHER "Kirill Gavrilov"
#define SVIEW_URL "http://www.sview.ru"
#define SVIEW_EXE_NAME "sView.exe"

[Setup]
; AppId could be optionally changed in scripts for major releases of product if we want to implement
;AppId={F22487C3-47F0-4e72-BF3A-78401BC6F459}
AppName={#SVIEW_NAME}
AppVerName={#SVIEW_VER_NAME}
AppPublisher={#SVIEW_PUBLISHER}
AppPublisherURL={#SVIEW_URL}
AppSupportURL={#SVIEW_URL}
AppUpdatesURL={#SVIEW_URL}
DefaultDirName={pf}\{#SVIEW_NAME}
DefaultGroupName={#SVIEW_NAME}
VersionInfoVersion={#SVIEW_VER}
VersionInfoTextVersion={#SVIEW_VER_NAME}
VersionInfoDescription=sView - Media Player
ArchitecturesInstallIn64BitMode=x64
;ArchitecturesAllowed=x64compatible and not arm64
ArchitecturesAllowed=x64compatible
; Output package options
OutputDir=repository\win
OutputBaseFilename=sViewSetup_{#SVIEW_VER_FULL}
Compression=lzma/ultra
SolidCompression=true
InternalCompressLevel=normal
; Icons/images
SetupIconFile=media\sView_Setup.ico
WizardImageFile=media\sView_WizImage.bmp
WizardSmallImageFile=media\sView_WizImageSmall.bmp
UninstallDisplayIcon={app}\{#SVIEW_EXE_NAME}
; Additional requirements
ChangesAssociations=true
ChangesEnvironment=true
RestartIfNeededByRun=false
PrivilegesRequired=none
; Wizard options
AllowNoIcons=true
AlwaysShowDirOnReadyPage=true
AlwaysShowGroupOnReadyPage=true
AlwaysShowComponentsList=false
FlatComponentsList=false

[Languages]
;InfoBeforeFile: {#SVIEW_DISTR_PATH}\info\ReadMeEn.rtf
Name: english; MessagesFile: compiler:Default.isl;           LicenseFile: {#SVIEW_DISTR_PATH}\info\license.rtf
Name: russian; MessagesFile: compiler:Languages\Russian.isl; LicenseFile: {#SVIEW_DISTR_PATH}\info\license.rtf
Name: spanish; MessagesFile: compiler:Languages\Spanish.isl; LicenseFile: {#SVIEW_DISTR_PATH}\info\license.rtf
Name: french;  MessagesFile: compiler:Languages\French.isl;  LicenseFile: {#SVIEW_DISTR_PATH}\info\license.rtf
Name: german;  MessagesFile: compiler:Languages\German.isl;  LicenseFile: {#SVIEW_DISTR_PATH}\info\license.rtf
Name: korean;  MessagesFile: compiler:Languages\Korean.isl;  LicenseFile: {#SVIEW_DISTR_PATH}\info\license.rtf
Name: chinese; MessagesFile: compiler:Languages\ChineseSimplified.isl; LicenseFile: {#SVIEW_DISTR_PATH}\info\license.rtf

[CustomMessages]
; Installation types
english.FullInstallation=Full Installation
russian.FullInstallation=Полная установка
spanish.FullInstallation=Instalación completa
french.FullInstallation=Installation complète
german.FullInstallation=Vollständige Installation
korean.FullInstallation=전체 설치
chinese.FullInstallation=全部安装
english.CustomInstallation=Custom Installation
spanish.CustomInstallation=Instalación personalizada
russian.CustomInstallation=Выборочная установка
french.CustomInstallation=Installation personnalisée
german.CustomInstallation=Benutzerdefinierte Installation
korean.CustomInstallation=사용자 설치
chinese.CustomInstallation=自定义安装
; Components
english.StCore=Core files
spanish.StCore=Archivos compartidos
russian.StCore=Общие файлы
german.StCore=Core-Dateien
english.StDrawers=Media types support
spanish.StDrawers=Soporte de tipos de medios
russian.StDrawers=Поддержка медиа-типов
english.StImageViewer=Stereoscopic Image Viewer
spanish.StImageViewer=Visor de imagen estereoscópica
russian.StImageViewer=Просмотр стереоизображений
english.StMoviePlayer=Stereoscopic Movie Player
spanish.StMoviePlayer=Reproductor de películas estereoscópicas
russian.StMoviePlayer=Воспроизведение стереовидео
english.StRenderers=Device support
spanish.StRenderers=Soporte del dispositivo
russian.StRenderers=Поддержка устройств стереовывода
german.StRenderers=Geräteunterstützung
english.StOutAnaglyph=Anaglyph glasses
spanish.StOutAnaglyph=Gafas anaglíficas
russian.StOutAnaglyph=Анаглифные очки
german.StOutAnaglyph=Anaglyphenbrille
english.StOutDual=Mirror Displays, Dual Projectors
spanish.StOutDual=Salida espejo, Salida dual
russian.StOutDual=Зеркальные системы, 2х-проекторные системы
english.StOutInterlace=Interlaced Displays, DLP TV
spanish.StOutInterlace=Filas entrelazadas, DLP TV
russian.StOutInterlace=Чересстрочные мониторы, DLP ТВ
english.StOutIZ3D=IZ3D Display
spanish.StOutIZ3D=Visualización IZ3D
russian.StOutIZ3D=Монитор IZ3D
german.StOutIZ3D=iZ3D-Bildschirmen
english.StOutPageFlip=Shutter glasses
spanish.StOutPageFlip=Gafas de obturador
russian.StOutPageFlip=Затворные очки
german.StOutPageFlip=Shutterbrille
english.StOutDistorted=Distorted output
spanish.StOutDistorted=Salida distorsionada
russian.StOutDistorted=Искажённый вывод
; File associations
english.FileAssociations=File associations
spanish.FileAssociations=Asociaciones de archivos
russian.FileAssociations=Файловые ассоциации
german.FileAssociations=Dateizuordnungen
english.AssocStereoImages=Associate stereoscopic images with sView (*.jps; *.pns; *.mpo)
spanish.AssocStereoImages=Asociar imágenes estereoscópicas con sView (*.jps; *.pns; *.mpo)
russian.AssocStereoImages=Связать с sView стереоизображения (*.jps; *.pns; *.mpo)
german.AssocStereoImages=Associate stereoskopische Bilder mit sView (*.jps; *.pns; *.mpo)
english.AssocImages=Associate common images with sView (*.jpg; *.png; *.webp; *.bmp; *.exr; *.hdr; *.tga)
spanish.AssocImages=Asociar imágenes comunes con sView (*.jpg; *.png; *.webp; *.bmp; *.exr; *.hdr; *.tga)
russian.AssocImages=Связать с sView обычные изображения (*.jpg; *.png; *.webp; *.bmp; *.exr; *.hdr; *.tga)
german.AssocImages=Associate normale Bilder mit sView (*.jpg; *.png; *.webp; *.bmp; *.exr; *.hdr; *.tga)
english.AssocMovies=Associate video files with sView (*.avi; *.mkv; *.mk3d; *.webm; *.wmv; *.ts)
spanish.AssocMovies=Asociar archivos de video con sView (*.avi; *.mkv; *.mk3d; *.webm; *.wmv; *.ts)
russian.AssocMovies=Связать с sView видеофайлы (*.avi; *.mkv; *.mk3d; *.webm; *.wmv; *.ts)
german.AssocMovies=Associate Videodateien mit sView (*.avi; *.mkv; *.mk3d; *.webm; *.wmv; *.ts)
english.AssocMusic=Associate music files with sView (*.mp3; *.ogg; *.wav; *.flac; *.ape)
spanish.AssocMusic=Asociar archivos de música con sView (*.mp3; *.ogg; *.wav; *.flac; *.ape)
russian.AssocMusic=Связать с sView музыкальные файлы (*.mp3; *.ogg; *.wav; *.flac; *.ape)
german.AssocMusic=Associate Musikdateien mit sView (*.mp3; *.ogg; *.wav; *.flac; *.ape)
english.AssocPlaylists=Associate playlist files with sView (*.m3u)
spanish.AssocPlaylists=Asociar archivos de lista de reproducción con sView (*.m3u)
; OpenAL soft
english.OpenALSoft51=OpenAL soft - force 5.1 channel output
russian.OpenALSoft51=OpenAL soft - force 5.1 channel output

[Types]
Name: full;   Description: "{cm:FullInstallation}"
Name: custom; Description: "{cm:CustomInstallation}"; Flags: iscustom

[Components]
Name: StCore;                     Description: "{cm:StCore}";           Types: custom full; Flags: fixed
Name: StDrawers;                  Description: "{cm:StDrawers}";        Types: custom full; Flags: fixed
Name: StDrawers\StImageViewer;    Description: "{cm:StImageViewer}";    Types: custom full
Name: StDrawers\StMoviePlayer;    Description: "{cm:StMoviePlayer}";    Types: custom full
Name: StRenderers;                Description: "{cm:StRenderers}";      Types: custom full; Flags: fixed
Name: StRenderers\StOutAnaglyph;  Description: "{cm:StOutAnaglyph}";    Types: custom full; Flags: fixed
Name: StRenderers\StOutDual;      Description: "{cm:StOutDual}";        Types: custom full; Flags: fixed
Name: StRenderers\StOutInterlace; Description: "{cm:StOutInterlace}";   Types: custom full; Flags: fixed
Name: StRenderers\StOutIZ3D;      Description: "{cm:StOutIZ3D}";        Types: custom full; Flags: fixed
Name: StRenderers\StOutPageFlip;  Description: "{cm:StOutPageFlip}";    Types: custom full; Flags: fixed
Name: StRenderers\StOutDistorted; Description: "{cm:StOutDistorted}";   Types: custom full; Flags: fixed

[Tasks]
Name: desktopicon;           Description: "{cm:CreateDesktopIcon}";     GroupDescription: "{cm:AdditionalIcons}";  Flags: unchecked
Name: quicklaunchicon;       Description: "{cm:CreateQuickLaunchIcon}"; GroupDescription: "{cm:AdditionalIcons}";  Flags: unchecked; OnlyBelowVersion: 0,6.1
Name: flagOpenAL51;          Description: "{cm:OpenALSoft51}";          GroupDescription: "{cm:AdditionalIcons}" ; Components: StDrawers\StMoviePlayer; Flags: unchecked
Name: flagAssocStereoImages; Description: "{cm:AssocStereoImages}";     GroupDescription: "{cm:FileAssociations}"; Components: StDrawers\StImageViewer
Name: flagAssocImages;       Description: "{cm:AssocImages}";           GroupDescription: "{cm:FileAssociations}"; Components: StDrawers\StImageViewer; Flags: unchecked
Name: flagAssocMovies;       Description: "{cm:AssocMovies}";           GroupDescription: "{cm:FileAssociations}"; Components: StDrawers\StMoviePlayer
Name: flagAssocMusic;        Description: "{cm:AssocMusic}";            GroupDescription: "{cm:FileAssociations}"; Components: StDrawers\StMoviePlayer; Flags: unchecked
Name: flagAssocPlaylists;    Description: "{cm:AssocPlaylists}";        GroupDescription: "{cm:FileAssociations}"; Components: StDrawers\StMoviePlayer; Flags: unchecked

[Files]
; Core files
Source: {#SVIEW_DISTR_PATH}\info\*;                     DestDir: {app}\info;          Flags: ignoreversion; Components: StCore
Source: {#SVIEW_DISTR_PATH}\lang\Czech\language.lng;    DestDir: {app}\lang\Czech;    Flags: ignoreversion; Components: StCore
Source: {#SVIEW_DISTR_PATH}\lang\Korean\language.lng;   DestDir: {app}\lang\Korean;   Flags: ignoreversion; Components: StCore
Source: {#SVIEW_DISTR_PATH}\lang\ChineseS\language.lng; DestDir: {app}\lang\ChineseS; Flags: ignoreversion; Components: StCore

Source: {#SVIEW_DISTR_PATH}\*.exe;                    DestDir: {app};  Flags: 64bit ignoreversion; Components: StCore
Source: {#SVIEW_DISTR_PATH}\StShared.dll;             DestDir: {app};  Flags: 64bit ignoreversion; Components: StCore
Source: {#SVIEW_DISTR_PATH}\StGLWidgets.dll;          DestDir: {app};  Flags: 64bit ignoreversion; Components: StCore
Source: {#SVIEW_DISTR_PATH}\StCore.dll;               DestDir: {app};  Flags: 64bit ignoreversion; Components: StCore
Source: {#SVIEW_DISTR_PATH}\StDiagnostics.dll;        DestDir: {app};  Flags: 64bit ignoreversion; Components: StCore
; MSVC C-Runtime libraries (mask compatible for vc100)
Source: {#SVIEW_DISTR_PATH}\msvc*.dll;                DestDir: {app};  Flags: 64bit ignoreversion; Components: StCore
; FreeType2 library commonly used
Source: {#SVIEW_DISTR_PATH}\freetype.dll;             DestDir: {app};  Flags: 64bit ignoreversion; Components: StCore
; FFmpeg libraries are commonly used
Source: {#SVIEW_DISTR_PATH}\av*.dll;                  DestDir: {app};  Flags: 64bit ignoreversion; Components: StCore
Source: {#SVIEW_DISTR_PATH}\sw*.dll;                  DestDir: {app};  Flags: 64bit ignoreversion; Components: StCore

; StRenderers -> StOutAnaglyph
Source: {#SVIEW_DISTR_PATH}\lang\*StOutAnaglyph.lng;  DestDir: {app}\lang;                   Flags: ignoreversion recursesubdirs;       Components: StRenderers\StOutAnaglyph
Source: {#SVIEW_DISTR_PATH}\shaders\StOutAnaglyph\*;  DestDir: {app}\shaders\StOutAnaglyph;  Flags: ignoreversion recursesubdirs;       Components: StRenderers\StOutAnaglyph
Source: {#SVIEW_DISTR_PATH}\StOutAnaglyph.dll;        DestDir: {app};                        Flags: 64bit ignoreversion;                Components: StRenderers\StOutAnaglyph
; StRenderers -> StOutDual
Source: {#SVIEW_DISTR_PATH}\lang\*StOutDual.lng;      DestDir: {app}\lang;                   Flags: ignoreversion recursesubdirs;       Components: StRenderers\StOutDual
Source: {#SVIEW_DISTR_PATH}\StOutDual.dll;            DestDir: {app};                        Flags: 64bit ignoreversion;                Components: StRenderers\StOutDual
; StRenderers -> StOutInterlace
Source: {#SVIEW_DISTR_PATH}\lang\*StOutInterlace.lng; DestDir: {app}\lang;                   Flags: ignoreversion recursesubdirs;       Components: StRenderers\StOutInterlace
Source: {#SVIEW_DISTR_PATH}\shaders\StOutInterlace\*; DestDir: {app}\shaders\StOutInterlace; Flags: 64bit ignoreversion recursesubdirs; Components: StRenderers\StOutInterlace
Source: {#SVIEW_DISTR_PATH}\StOutInterlace.dll;       DestDir: {app};                        Flags: 64bit ignoreversion;                Components: StRenderers\StOutInterlace
; StRenderers -> StOutIZ3D
Source: {#SVIEW_DISTR_PATH}\lang\*StOutIZ3D.lng;      DestDir: {app}\lang;                   Flags: ignoreversion recursesubdirs;       Components: StRenderers\StOutIZ3D
Source: {#SVIEW_DISTR_PATH}\shaders\StOutIZ3D\*;      DestDir: {app}\shaders\StOutIZ3D;      Flags: 64bit ignoreversion recursesubdirs; Components: StRenderers\StOutIZ3D
Source: {#SVIEW_DISTR_PATH}\StOutIZ3D.dll;            DestDir: {app};                        Flags: 64bit ignoreversion;                Components: StRenderers\StOutIZ3D
; StRenderers -> StOutPageFlip
Source: {#SVIEW_DISTR_PATH}\lang\*StOutPageFlip.lng;  DestDir: {app}\lang;                   Flags: ignoreversion recursesubdirs;       Components: StRenderers\StOutPageFlip
Source: {#SVIEW_DISTR_PATH}\StOutPageFlip.dll;        DestDir: {app};                        Flags: 64bit ignoreversion;                Components: StRenderers\StOutPageFlip
; StRenderers -> StOutDistorted
Source: {#SVIEW_DISTR_PATH}\lang\*StOutDistorted.lng; DestDir: {app}\lang;                   Flags: ignoreversion recursesubdirs;       Components: StRenderers\StOutDistorted
Source: {#SVIEW_DISTR_PATH}\StOutDistorted.dll;       DestDir: {app};                        Flags: 64bit ignoreversion;                Components: StRenderers\StOutDistorted
Source: {#SVIEW_DISTR_PATH}\openvr_api.dll;           DestDir: {app};                        Flags: 64bit ignoreversion;                Components: StRenderers\StOutDistorted

; StDrawers
Source: {#SVIEW_DISTR_PATH}\textures\*;               DestDir: {app}\textures;               Flags: ignoreversion;         Components: StDrawers\StImageViewer or StDrawers\StMoviePlayer
Source: {#SVIEW_DISTR_PATH}\icons\sView_Media.ico;    DestDir: {app}\icons;                  Flags: ignoreversion;         Components: StDrawers\StImageViewer or StDrawers\StMoviePlayer
; StDrawers -> Image Viewer
Source: {#SVIEW_DISTR_PATH}\lang\*StImageViewer.lng;  DestDir: {app}\lang;                   Flags: ignoreversion recursesubdirs; Components: StCore
Source: {#SVIEW_DISTR_PATH}\icons\sView_JPS.ico;      DestDir: {app}\icons;                  Flags: ignoreversion;         Components: StDrawers\StImageViewer
Source: {#SVIEW_DISTR_PATH}\icons\sView_PNS.ico;      DestDir: {app}\icons;                  Flags: ignoreversion;         Components: StDrawers\StImageViewer
Source: {#SVIEW_DISTR_PATH}\demo.jps;                 DestDir: {app};                        Flags: ignoreversion;         Components: StDrawers\StImageViewer
Source: {#SVIEW_DISTR_PATH}\demo_robot.jps;           DestDir: {app};                        Flags: ignoreversion;         Components: StDrawers\StImageViewer
Source: {#SVIEW_DISTR_PATH}\StImageViewer.dll;        DestDir: {app};                        Flags: 64bit ignoreversion;   Components: StCore
; FreeImage library (should be optional)
Source: {#SVIEW_DISTR_PATH}\FreeImage.dll;            DestDir: {app};                        Flags: 64bit ignoreversion;   Components: StDrawers\StImageViewer
; DevIL libraries (should be optional)
Source: {#SVIEW_DISTR_PATH}\DevIL.dll;                DestDir: {app};                        Flags: 64bit ignoreversion;   Components: StDrawers\StImageViewer
Source: {#SVIEW_DISTR_PATH}\ILU.dll;                  DestDir: {app};                        Flags: 64bit ignoreversion;   Components: StDrawers\StImageViewer

; StDrawers -> Movie Player
Source: {#SVIEW_DISTR_PATH}\lang\*StMoviePlayer.lng;  DestDir: {app}\lang;                   Flags: ignoreversion recursesubdirs; Components: StCore
Source: {#SVIEW_DISTR_PATH}\web\*.htm;                DestDir: {app}\web;                    Flags: ignoreversion recursesubdirs; Components: StDrawers\StMoviePlayer
Source: alsoft51.ini;                                 DestDir: {userappdata}; DestName: "alsoft.ini"; Tasks: flagOpenAL51; Components: StCore
Source: openal\hrtf\*;                                DestDir: {commonappdata}\openal\hrtf;                                Components: StCore
Source: {#SVIEW_DISTR_PATH}\StMoviePlayer.dll;        DestDir: {app};                        Flags: 64bit ignoreversion;   Components: StCore
Source: {#SVIEW_DISTR_PATH}\OpenAL32.dll;             DestDir: {app};                        Flags: 64bit ignoreversion;   Components: StCore

; StDrawers -> Tiny CAD viewer
;Source: {#SVIEW_DISTR_PATH}\StCADViewer.exe; DestDir: {app}; Flags: 64bit ignoreversion; Components: StCore

[Icons]
Name: {group}\sView - Image Viewer;                    Filename: {app}\{#SVIEW_EXE_NAME};  Components: StDrawers\StImageViewer; IconFilename: {app}\{#SVIEW_EXE_NAME}; Comment: "{cm:StImageViewer}";  IconIndex: 0; Parameters: "--in=image --demo=""{app}\demo.jps"""
Name: {group}\sView - Movie Player;                    Filename: {app}\{#SVIEW_EXE_NAME};  Components: StDrawers\StMoviePlayer; IconFilename: {app}\{#SVIEW_EXE_NAME}; Comment: "{cm:StMoviePlayer}";  IconIndex: 0; Parameters: "--in=video"
Name: {group}\Extras\sView - Autodetection;            Filename: {app}\{#SVIEW_EXE_NAME};  Components: StDrawers\StImageViewer; IconFilename: {app}\{#SVIEW_EXE_NAME}; Comment: Failsafe sView launch; IconIndex: 0; Parameters: --out=Auto --in=image
Name: {group}\Extras\sView - Movie Player (Last File); Filename: {app}\{#SVIEW_EXE_NAME};  Components: StDrawers\StMoviePlayer; IconFilename: {app}\{#SVIEW_EXE_NAME}; Comment: "{cm:StMoviePlayer}";  IconIndex: 0; Parameters: "--last"
Name: {group}\Extras\sView - Diagnostics;              Filename: {app}\{#SVIEW_EXE_NAME};  Components: StCore;                  IconFilename: {app}\{#SVIEW_EXE_NAME}; Comment: sView Diagnostics;     IconIndex: 0; Parameters: "--in=StDiagnostics"
Name: {group}\Extras\Monitors Dump;                    Filename: {app}\StMonitorsDump.exe; Components: StCore;                                                         Comment: Information about connected displays

;Name: {group}\user manual;          Filename: {app}\info\manual.pdf; Comment: User manual; Tasks: ; Languages:
Name: {group}\{cm:UninstallProgram,{#SVIEW_NAME}}; Filename: {uninstallexe}
Name: {commondesktop}\{#SVIEW_NAME}; Filename: {app}\{#SVIEW_EXE_NAME}; Tasks: desktopicon; Components: StDrawers\StImageViewer; IconFilename: {app}\{#SVIEW_EXE_NAME}; Comment: "{cm:StImageViewer}"; IconIndex: 0; Parameters: "--in=image --demo=""{app}\demo.jps"""
Name: {userappdata}\Microsoft\Internet Explorer\Quick Launch\{#SVIEW_NAME}; Filename: {app}\{#SVIEW_EXE_NAME}; Tasks: quicklaunchicon; Components: StDrawers\StImageViewer; IconFilename: {app}\{#SVIEW_EXE_NAME}; Comment: "{cm:StImageViewer}"; IconIndex: 0; Parameters: "--in=image --demo=""{app}\demo.jps"""

[Run]
;Filename: {app}\{#SVIEW_EXE_NAME}; WorkingDir: {app}; Components: StDrawers\StImageViewer; Description: {cm:LaunchProgram,{#SVIEW_NAME}}; Parameters: --in=image - demo.jps; Flags: nowait postinstall skipifsilent

[Registry]
; Install/Uninstall info
Root: HKCU; Subkey: SOFTWARE\sView;       ValueType: none; Flags: uninsdeletekey; Tasks: ; Languages: ; ValueData: sView 2011
Root: HKCU; Subkey: Software\sView\sView; ValueType: string; ValueName: language; ValueData: русский;  Tasks: ; Languages: russian; Flags: uninsdeletekey
Root: HKCU; Subkey: Software\sView\sView; ValueType: string; ValueName: language; ValueData: English;  Tasks: ; Languages: english; Flags: uninsdeletekey
Root: HKCU; Subkey: Software\sView\sView; ValueType: string; ValueName: language; ValueData: Español;  Tasks: ; Languages: spanish; Flags: uninsdeletekey
Root: HKCU; Subkey: Software\sView\sView; ValueType: string; ValueName: language; ValueData: français; Tasks: ; Languages: french;  Flags: uninsdeletekey
Root: HKCU; Subkey: Software\sView\sView; ValueType: string; ValueName: language; ValueData: Deutsch;  Tasks: ; Languages: german;  Flags: uninsdeletekey
Root: HKCU; Subkey: Software\sView\sView; ValueType: string; ValueName: language; ValueData: Korean;   Tasks: ; Languages: korean;  Flags: uninsdeletekey
Root: HKCU; Subkey: Software\sView\sView; ValueType: string; ValueName: language; ValueData: ChineseS; Tasks: ; Languages: chinese; Flags: uninsdeletekey
; Associations JPS
Root: HKCR; SubKey: .jps;                                      ValueType: string; ValueData: JPEG Stereo Image;             Tasks: flagAssocStereoImages; Flags: uninsdeletekey
Root: HKCR; SubKey: JPEG Stereo Image;                         ValueType: string; ValueData: JPEG Stereo Image;             Tasks: flagAssocStereoImages; Flags: uninsdeletekey
Root: HKCR; SubKey: JPEG Stereo Image\Shell\Open\Command;      ValueType: string; ValueData: """{app}\{#SVIEW_EXE_NAME}"" --in=image - ""%1"""; Tasks: flagAssocStereoImages; Flags: uninsdeletevalue
Root: HKCR; Subkey: JPEG Stereo Image\DefaultIcon;             ValueType: string; ValueData: {app}\icons\sView_JPS.ico;     Tasks: flagAssocStereoImages; Flags: uninsdeletevalue
; Associations PNS
Root: HKCR; SubKey: .pns;                                      ValueType: string; ValueData: PNG Stereo Image;              Tasks: flagAssocStereoImages; Flags: uninsdeletekey
Root: HKCR; SubKey: PNG Stereo Image;                          ValueType: string; ValueData: PNG Stereo Image;              Tasks: flagAssocStereoImages; Flags: uninsdeletekey
Root: HKCR; SubKey: PNG Stereo Image\Shell\Open\Command;       ValueType: string; ValueData: """{app}\{#SVIEW_EXE_NAME}"" --in=image - ""%1"""; Tasks: flagAssocStereoImages; Flags: uninsdeletevalue
Root: HKCR; Subkey: PNG Stereo Image\DefaultIcon;              ValueType: string; ValueData: {app}\icons\sView_PNS.ico;     Tasks: flagAssocStereoImages; Flags: uninsdeletevalue
; Associations MPO
Root: HKCR; SubKey: .mpo;                                      ValueType: string; ValueData: Multi Picture Object;          Tasks: flagAssocStereoImages; Flags: uninsdeletekey
Root: HKCR; SubKey: Multi Picture Object;                      ValueType: string; ValueData: Multi Picture Object;          Tasks: flagAssocStereoImages; Flags: uninsdeletekey
Root: HKCR; SubKey: Multi Picture Object\Shell\Open\Command;   ValueType: string; ValueData: """{app}\{#SVIEW_EXE_NAME}"" --in=image - ""%1"""; Tasks: flagAssocStereoImages; Flags: uninsdeletevalue
Root: HKCR; Subkey: Multi Picture Object\DefaultIcon;          ValueType: string; ValueData: {app}\icons\sView_Media.ico;   Tasks: flagAssocStereoImages; Flags: uninsdeletevalue

; Associations Images
Root: HKCR; SubKey: sView Image;                               ValueType: string; ValueData: sView Image;                   Tasks: flagAssocImages;       Flags: uninsdeletekey
Root: HKCR; SubKey: sView Image\Shell\Open\Command;            ValueType: string; ValueData: """{app}\{#SVIEW_EXE_NAME}"" --in=image - ""%1"""; Tasks: flagAssocImages; Flags: uninsdeletevalue
Root: HKCR; Subkey: sView Image\DefaultIcon;                   ValueType: string; ValueData: {app}\icons\sView_Media.ico;   Tasks: flagAssocImages;       Flags: uninsdeletevalue

Root: HKCR; SubKey: .bmp;                                      ValueType: string; ValueData: sView Image;                   Tasks: flagAssocImages;       Flags: uninsdeletekey
Root: HKCR; SubKey: .jpg;                                      ValueType: string; ValueData: sView Image;                   Tasks: flagAssocImages;       Flags: uninsdeletekey
Root: HKCR; SubKey: .jpeg;                                     ValueType: string; ValueData: sView Image;                   Tasks: flagAssocImages;       Flags: uninsdeletekey
Root: HKCR; SubKey: .jpe;                                      ValueType: string; ValueData: sView Image;                   Tasks: flagAssocImages;       Flags: uninsdeletekey
Root: HKCR; SubKey: .j2k;                                      ValueType: string; ValueData: sView Image;                   Tasks: flagAssocImages;       Flags: uninsdeletekey
Root: HKCR; SubKey: .jp2;                                      ValueType: string; ValueData: sView Image;                   Tasks: flagAssocImages;       Flags: uninsdeletekey
Root: HKCR; SubKey: .tga;                                      ValueType: string; ValueData: sView Image;                   Tasks: flagAssocImages;       Flags: uninsdeletekey
Root: HKCR; SubKey: .png;                                      ValueType: string; ValueData: sView Image;                   Tasks: flagAssocImages;       Flags: uninsdeletekey
Root: HKCR; SubKey: .exr;                                      ValueType: string; ValueData: sView Image;                   Tasks: flagAssocImages;       Flags: uninsdeletekey
Root: HKCR; SubKey: .hdr;                                      ValueType: string; ValueData: sView Image;                   Tasks: flagAssocImages;       Flags: uninsdeletekey
Root: HKCR; SubKey: .webp;                                     ValueType: string; ValueData: sView Image;                   Tasks: flagAssocImages;       Flags: uninsdeletekey
Root: HKCR; SubKey: .webpll;                                   ValueType: string; ValueData: sView Image;                   Tasks: flagAssocImages;       Flags: uninsdeletekey
Root: HKCR; SubKey: .tif;                                      ValueType: string; ValueData: sView Image;                   Tasks: flagAssocImages;       Flags: uninsdeletekey
Root: HKCR; SubKey: .tiff;                                     ValueType: string; ValueData: sView Image;                   Tasks: flagAssocImages;       Flags: uninsdeletekey

; Associations Video
Root: HKCR; SubKey: sView Video;                               ValueType: string; ValueData: sView Video;                   Tasks: flagAssocMovies; Flags: uninsdeletekey
Root: HKCR; SubKey: sView Video\Shell\Open\Command;            ValueType: string; ValueData: """{app}\{#SVIEW_EXE_NAME}"" --in=video - ""%1"""; Tasks: flagAssocMovies; Flags: uninsdeletevalue
Root: HKCR; Subkey: sView Video\DefaultIcon;                   ValueType: string; ValueData: {app}\icons\sView_Media.ico;   Tasks: flagAssocMovies; Flags: uninsdeletevalue

Root: HKCR; SubKey: .avi;                                      ValueType: string; ValueData: sView Video;                   Tasks: flagAssocMovies; Flags: uninsdeletekey
Root: HKCR; SubKey: .mkv;                                      ValueType: string; ValueData: sView Video;                   Tasks: flagAssocMovies; Flags: uninsdeletekey
Root: HKCR; SubKey: .mk3d;                                     ValueType: string; ValueData: sView Video;                   Tasks: flagAssocMovies; Flags: uninsdeletekey
Root: HKCR; SubKey: .webm;                                     ValueType: string; ValueData: sView Video;                   Tasks: flagAssocMovies; Flags: uninsdeletekey
Root: HKCR; SubKey: .wmv;                                      ValueType: string; ValueData: sView Video;                   Tasks: flagAssocMovies; Flags: uninsdeletekey
Root: HKCR; SubKey: .ts;                                       ValueType: string; ValueData: sView Video;                   Tasks: flagAssocMovies; Flags: uninsdeletekey
Root: HKCR; SubKey: .mts;                                      ValueType: string; ValueData: sView Video;                   Tasks: flagAssocMovies; Flags: uninsdeletekey
Root: HKCR; SubKey: .m2ts;                                     ValueType: string; ValueData: sView Video;                   Tasks: flagAssocMovies; Flags: uninsdeletekey
Root: HKCR; SubKey: .mt2s;                                     ValueType: string; ValueData: sView Video;                   Tasks: flagAssocMovies; Flags: uninsdeletekey
Root: HKCR; SubKey: .vob;                                      ValueType: string; ValueData: sView Video;                   Tasks: flagAssocMovies; Flags: uninsdeletekey
Root: HKCR; SubKey: .mp4;                                      ValueType: string; ValueData: sView Video;                   Tasks: flagAssocMovies; Flags: uninsdeletekey
Root: HKCR; SubKey: .m4v;                                      ValueType: string; ValueData: sView Video;                   Tasks: flagAssocMovies; Flags: uninsdeletekey
Root: HKCR; SubKey: .mp4v;                                     ValueType: string; ValueData: sView Video;                   Tasks: flagAssocMovies; Flags: uninsdeletekey
Root: HKCR; SubKey: .mpeg;                                     ValueType: string; ValueData: sView Video;                   Tasks: flagAssocMovies; Flags: uninsdeletekey
Root: HKCR; SubKey: .mpe;                                      ValueType: string; ValueData: sView Video;                   Tasks: flagAssocMovies; Flags: uninsdeletekey
Root: HKCR; SubKey: .mpg;                                      ValueType: string; ValueData: sView Video;                   Tasks: flagAssocMovies; Flags: uninsdeletekey
Root: HKCR; SubKey: .mpv2;                                     ValueType: string; ValueData: sView Video;                   Tasks: flagAssocMovies; Flags: uninsdeletekey
Root: HKCR; SubKey: .flv;                                      ValueType: string; ValueData: sView Video;                   Tasks: flagAssocMovies; Flags: uninsdeletekey
Root: HKCR; SubKey: .mov;                                      ValueType: string; ValueData: sView Video;                   Tasks: flagAssocMovies; Flags: uninsdeletekey
Root: HKCR; SubKey: .ogm;                                      ValueType: string; ValueData: sView Video;                   Tasks: flagAssocMovies; Flags: uninsdeletekey
Root: HKCR; SubKey: .bik;                                      ValueType: string; ValueData: sView Video;                   Tasks: flagAssocMovies; Flags: uninsdeletekey
Root: HKCR; SubKey: .mj2;                                      ValueType: string; ValueData: sView Video;                   Tasks: flagAssocMovies; Flags: uninsdeletekey

; Associations Audio
Root: HKCR; SubKey: sView Audio;                               ValueType: string; ValueData: sView Audio;                   Tasks: flagAssocMusic;  Flags: uninsdeletekey
Root: HKCR; SubKey: sView Audio\Shell\Open\Command;            ValueType: string; ValueData: """{app}\{#SVIEW_EXE_NAME}"" --in=video - ""%1"""; Tasks: flagAssocMusic; Flags: uninsdeletevalue
Root: HKCR; Subkey: sView Audio\DefaultIcon;                   ValueType: string; ValueData: {app}\icons\sView_Media.ico;   Tasks: flagAssocMusic; Flags: uninsdeletevalue

Root: HKCR; SubKey: .mp3;                                      ValueType: string; ValueData: sView Audio;                   Tasks: flagAssocMusic;  Flags: uninsdeletekey
Root: HKCR; SubKey: .m4a;                                      ValueType: string; ValueData: sView Audio;                   Tasks: flagAssocMusic;  Flags: uninsdeletekey
Root: HKCR; SubKey: .mpa;                                      ValueType: string; ValueData: sView Audio;                   Tasks: flagAssocMusic;  Flags: uninsdeletekey
Root: HKCR; SubKey: .aac;                                      ValueType: string; ValueData: sView Audio;                   Tasks: flagAssocMusic;  Flags: uninsdeletekey
Root: HKCR; SubKey: .ogg;                                      ValueType: string; ValueData: sView Audio;                   Tasks: flagAssocMusic;  Flags: uninsdeletekey
Root: HKCR; SubKey: .opus;                                     ValueType: string; ValueData: sView Audio;                   Tasks: flagAssocMusic;  Flags: uninsdeletekey
Root: HKCR; SubKey: .wav;                                      ValueType: string; ValueData: sView Audio;                   Tasks: flagAssocMusic;  Flags: uninsdeletekey
Root: HKCR; SubKey: .flac;                                     ValueType: string; ValueData: sView Audio;                   Tasks: flagAssocMusic;  Flags: uninsdeletekey
Root: HKCR; SubKey: .ape;                                      ValueType: string; ValueData: sView Audio;                   Tasks: flagAssocMusic;  Flags: uninsdeletekey
Root: HKCR; SubKey: .mka;                                      ValueType: string; ValueData: sView Audio;                   Tasks: flagAssocMusic;  Flags: uninsdeletekey

; Associations Playlists
Root: HKCR; SubKey: sView PlayList;                            ValueType: string; ValueData: sView PlayList;                Tasks: flagAssocImages;    Flags: uninsdeletekey
Root: HKCR; SubKey: sView PlayList\Shell\Open\Command;         ValueType: string; ValueData: """{app}\{#SVIEW_EXE_NAME}"" --in=video - ""%1"""; Tasks: flagAssocPlaylists; Flags: uninsdeletevalue
Root: HKCR; Subkey: sView PlayList\DefaultIcon;                ValueType: string; ValueData: {app}\icons\sView_Media.ico;   Tasks: flagAssocPlaylists; Flags: uninsdeletevalue
Root: HKCR; SubKey: .m3u;                                      ValueType: string; ValueData: sView PlayList;                Tasks: flagAssocPlaylists; Flags: uninsdeletekey

; StCoreXX environment variables
Root: HKLM64; SubKey: SYSTEM\CurrentControlSet\Control\Session Manager\Environment; ValueType: string; ValueName: StShare;  ValueData: {app}\; Flags: uninsdeletevalue
Root: HKCU64; SubKey: Environment;                                                  ValueType: string; ValueName: StShare;  ValueData: {app}\; Flags: uninsdeletevalue
Root: HKLM64; SubKey: SYSTEM\CurrentControlSet\Control\Session Manager\Environment; ValueType: string; ValueName: StCore64; ValueData: {app}\; Flags: uninsdeletevalue
Root: HKCU64; SubKey: Environment;                                                  ValueType: string; ValueName: StCore64; ValueData: {app}\; Flags: uninsdeletevalue

[INI]
Filename: {app}\sview.ru.url; Section: InternetShortcut; Key: URL; String: {#SVIEW_URL}

[UninstallDelete]
Type: files; Name: {app}\sview.ru.url

[UninstallRun]
; nothing

[Dirs]

[InstallDelete]
Name: {app}\;                  Type: filesandordirs
Name: {group}\;                Type: filesandordirs
Name: {app}\input\;            Type: filesandordirs
Name: {app}\StRenderers\;      Type: filesandordirs
; legacy files from previous installations
Name: {app}\StBrowserPlugins\; Type: filesandordirs
Name: {app}\amd64\;            Type: filesandordirs
