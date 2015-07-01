
#include "temp\config.iss"

; Should be defined in config file
;#define SVIEW_VER "12.7.0.0"
;#define SVIEW_VER_FULL "v.12.00alpha00"
;#define SVIEW_VER_NAME "sView (12.00alpha00)"
;#define SVIEW_DISTR_PATH_x86   "sView_x86"
;#define SVIEW_DISTR_PATH_AMD64 "sView_AMD64"

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
;InfoBeforeFile: {#SVIEW_DISTR_PATH_x86}\info\ReadMeEn.rtf
Name: english; MessagesFile: compiler:Default.isl;           LicenseFile: {#SVIEW_DISTR_PATH_x86}\info\license.rtf
Name: russian; MessagesFile: compiler:Languages\Russian.isl; LicenseFile: {#SVIEW_DISTR_PATH_x86}\info\license.rtf
Name: french;  MessagesFile: compiler:Languages\French.isl;  LicenseFile: {#SVIEW_DISTR_PATH_x86}\info\license.rtf
Name: german;  MessagesFile: compiler:Languages\German.isl;  LicenseFile: {#SVIEW_DISTR_PATH_x86}\info\license.rtf
Name: korean;  MessagesFile: compiler:Languages\Korean.isl;  LicenseFile: {#SVIEW_DISTR_PATH_x86}\info\license.rtf
Name: chinese; MessagesFile: compiler:Languages\ChineseSimplified.isl; LicenseFile: {#SVIEW_DISTR_PATH_x86}\info\license.rtf

[CustomMessages]
; Installation types
english.FullInstallation=Full Installation
russian.FullInstallation=Полная установка
french.FullInstallation=Installation complète
german.FullInstallation=Vollständige Installation
korean.FullInstallation=전체 설치
chinese.FullInstallation=全部安装
english.CustomInstallation=Custom Installation
russian.CustomInstallation=Выборочная установка
french.CustomInstallation=Installation personnalisée
german.CustomInstallation=Benutzerdefinierte Installation
korean.CustomInstallation=사용자 설치
chinese.CustomInstallation=自定义安装
; Components
english.StCore=Core files
russian.StCore=Общие файлы
german.StCore=Core-Dateien
english.StBrowserPlugins=NPAPI Browser plugin (Firefox, Google Chrome, Opera,...)
russian.StBrowserPlugins=NPAPI плагин для браузеров Firefox, Google Chrome, Opera,...
english.StActiveXPlugin=ActiveX control for Internet Explorer
russian.StActiveXPlugin=ActiveX плагин для браузера Internet Explorer
english.StDrawers=Media types support
russian.StDrawers=Поддержка медиа-типов
english.StImageViewer=Stereoscopic Image Viewer
russian.StImageViewer=Просмотр стереоизображений
english.StMoviePlayer=Stereoscopic Movie Player
russian.StMoviePlayer=Воспроизведение стереовидео
english.StRenderers=Device support
russian.StRenderers=Поддержка устройств стереовывода
german.StRenderers=Geräteunterstützung
english.StOutAnaglyph=Anaglyph glasses
russian.StOutAnaglyph=Анаглифные очки
german.StOutAnaglyph=Anaglyphenbrille
english.StOutDual=Mirror Displays, Dual Projectors
russian.StOutDual=Зеркальные системы, 2х-проекторные системы
english.StOutInterlace=Interlaced Displays, DLP TV
russian.StOutInterlace=Чересстрочные мониторы, DLP ТВ
english.StOutIZ3D=IZ3D Display
russian.StOutIZ3D=Монитор IZ3D
german.StOutIZ3D=iZ3D-Bildschirmen
english.StOutPageFlip=Shutter glasses
russian.StOutPageFlip=Затворные очки
german.StOutPageFlip=Shutterbrille
english.StOutDistorted=Distorted output
russian.StOutDistorted=Искажённый вывод
; File associations
english.FileAssociations=File associations
russian.FileAssociations=Файловые ассоциации
german.FileAssociations=Dateizuordnungen
english.AssocStereoImages=Associate stereoscopic images with sView (*.jps; *.pns; *.mpo)
russian.AssocStereoImages=Связать с sView стереоизображения (*.jps; *.pns; *.mpo)
german.AssocStereoImages=Associate stereoskopische Bilder mit sView (*.jps; *.pns; *.mpo)
english.AssocImages=Associate common images with sView (*.jpg; *.png; *.webp; *.bmp; *.exr; *.hdr; *.tga)
russian.AssocImages=Связать с sView обычные изображения (*.jpg; *.png; *.webp; *.bmp; *.exr; *.hdr; *.tga)
german.AssocImages=Associate normale Bilder mit sView (*.jpg; *.png; *.webp; *.bmp; *.exr; *.hdr; *.tga)
english.AssocMovies=Associate video files with sView (*.avi; *.mkv; *.mk3d; *.webm; *.wmv; *.ts)
russian.AssocMovies=Связать с sView видеофайлы (*.avi; *.mkv; *.mk3d; *.webm; *.wmv; *.ts)
german.AssocMovies=Associate Videodateien mit sView (*.avi; *.mkv; *.mk3d; *.webm; *.wmv; *.ts)
english.AssocMusic=Associate music files with sView (*.mp3; *.ogg; *.wav; *.flac; *.ape)
russian.AssocMusic=Связать с sView музыкальные файлы (*.mp3; *.ogg; *.wav; *.flac; *.ape)
german.AssocMusic=Associate Musikdateien mit sView (*.mp3; *.ogg; *.wav; *.flac; *.ape)
; OpenAL soft
english.OpenALSoft51=OpenAL soft - force 5.1 channel output
russian.OpenALSoft51=OpenAL soft - force 5.1 channel output

[Types]
Name: full;   Description: "{cm:FullInstallation}"
Name: custom; Description: "{cm:CustomInstallation}"; Flags: iscustom

[Components]
Name: StCore;                     Description: "{cm:StCore}";           Types: custom full; Flags: fixed
Name: StBrowserPlugins;           Description: "{cm:StBrowserPlugins}"; Types: custom full
Name: StActiveXPlugin;            Description: "{cm:StActiveXPlugin}";  Types: custom full
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
Name: quicklaunchicon;       Description: "{cm:CreateQuickLaunchIcon}"; GroupDescription: "{cm:AdditionalIcons}";  Flags: unchecked
Name: flagOpenAL51;          Description: "{cm:OpenALSoft51}";          GroupDescription: "{cm:AdditionalIcons}" ; Components: StDrawers\StMoviePlayer; Flags: unchecked
Name: flagAssocStereoImages; Description: "{cm:AssocStereoImages}";     GroupDescription: "{cm:FileAssociations}"; Components: StDrawers\StImageViewer
Name: flagAssocImages;       Description: "{cm:AssocImages}";           GroupDescription: "{cm:FileAssociations}"; Components: StDrawers\StImageViewer; Flags: unchecked
Name: flagAssocMovies;       Description: "{cm:AssocMovies}";           GroupDescription: "{cm:FileAssociations}"; Components: StDrawers\StMoviePlayer
Name: flagAssocMusic;        Description: "{cm:AssocMusic}";            GroupDescription: "{cm:FileAssociations}"; Components: StDrawers\StMoviePlayer; Flags: unchecked

[Files]
; Core files
Source: {#SVIEW_DISTR_PATH_x86}\*.exe;            DestDir: {app};        Flags: 32bit ignoreversion; Components: StCore
Source: {#SVIEW_DISTR_PATH_x86}\info\*;           DestDir: {app}\info;   Flags: 32bit ignoreversion; Components: StCore
Source: {#SVIEW_DISTR_PATH_AMD64}\*.exe;          DestDir: {app}\amd64;  Flags: 64bit ignoreversion; Components: StCore; Check: IsWin64
Source: {#SVIEW_DISTR_PATH_x86}\StShared.dll;     DestDir: {app};        Flags: 32bit ignoreversion; Components: StCore
Source: {#SVIEW_DISTR_PATH_x86}\StGLWidgets.dll;  DestDir: {app};        Flags: 32bit ignoreversion; Components: StCore
Source: {#SVIEW_DISTR_PATH_x86}\StCore.dll;       DestDir: {app};        Flags: 32bit ignoreversion; Components: StCore
Source: {#SVIEW_DISTR_PATH_AMD64}\StShared.dll;   DestDir: {app}\amd64;  Flags: 64bit ignoreversion; Components: StCore; Check: IsWin64
Source: {#SVIEW_DISTR_PATH_AMD64}\StGLWidgets.dll;DestDir: {app}\amd64;  Flags: 64bit ignoreversion; Components: StCore; Check: IsWin64
Source: {#SVIEW_DISTR_PATH_AMD64}\StCore.dll;     DestDir: {app}\amd64;  Flags: 64bit ignoreversion; Components: StCore; Check: IsWin64
Source: {#SVIEW_DISTR_PATH_x86}\StDiagnostics.dll;   DestDir: {app};       Flags: 32bit ignoreversion; Components: StCore
Source: {#SVIEW_DISTR_PATH_AMD64}\StDiagnostics.dll; DestDir: {app}\amd64; Flags: 64bit ignoreversion; Components: StCore; Check: IsWin64
Source: {#SVIEW_DISTR_PATH_x86}\shaders\StGLWidgets\*;         DestDir: {app}\shaders\StGLWidgets;       Flags: 32bit ignoreversion; Components: StCore
Source: {#SVIEW_DISTR_PATH_x86}\lang\Korean\language.lng;      DestDir: {app}\lang\Korean;               Flags: 32bit ignoreversion; Components: StCore
Source: {#SVIEW_DISTR_PATH_x86}\lang\ChineseS\language.lng;    DestDir: {app}\lang\ChineseS;             Flags: 32bit ignoreversion; Components: StCore
; MSVC C-Runtime libraries (mask compatible for vc100)
Source: {#SVIEW_DISTR_PATH_x86}\msvc*.dll;        DestDir: {app};        Flags: 32bit ignoreversion; Components: StCore
Source: {#SVIEW_DISTR_PATH_AMD64}\msvc*.dll;      DestDir: {app}\amd64;  Flags: 64bit ignoreversion; Components: StCore; Check: IsWin64
; FreeType2 library commonly used
Source: {#SVIEW_DISTR_PATH_x86}\freetype.dll;     DestDir: {app};        Flags: 32bit ignoreversion; Components: StCore
Source: {#SVIEW_DISTR_PATH_AMD64}\freetype.dll;   DestDir: {app}\amd64;  Flags: 64bit ignoreversion; Components: StCore; Check: IsWin64
; FFmpeg libraries are commonly used
Source: {#SVIEW_DISTR_PATH_x86}\av*.dll;          DestDir: {app};        Flags: 32bit ignoreversion; Components: StCore
Source: {#SVIEW_DISTR_PATH_x86}\sw*.dll;          DestDir: {app};        Flags: 32bit ignoreversion; Components: StCore
Source: {#SVIEW_DISTR_PATH_AMD64}\av*.dll;        DestDir: {app}\amd64;  Flags: 64bit ignoreversion; Components: StCore; Check: IsWin64
Source: {#SVIEW_DISTR_PATH_AMD64}\sw*.dll;        DestDir: {app}\amd64;  Flags: 64bit ignoreversion; Components: StCore; Check: IsWin64

; Browser plugin
Source: {#SVIEW_DISTR_PATH_x86}\npStBrowserPlugin.dll;   DestDir: {app};       Flags: 32bit ignoreversion; Components: StBrowserPlugins or StActiveXPlugin
Source: {#SVIEW_DISTR_PATH_AMD64}\npStBrowserPlugin.dll; DestDir: {app}\amd64; Flags: 64bit ignoreversion; Components: StBrowserPlugins or StActiveXPlugin; Check: IsWin64
Source: {#SVIEW_DISTR_PATH_x86}\mfc*.dll;                DestDir: {app};       Flags: 32bit ignoreversion; Components: StBrowserPlugins or StActiveXPlugin
Source: {#SVIEW_DISTR_PATH_AMD64}\mfc*.dll;              DestDir: {app}\amd64; Flags: 64bit ignoreversion; Components: StBrowserPlugins or StActiveXPlugin; Check: IsWin64

; StRenderers -> StOutAnaglyph
Source: {#SVIEW_DISTR_PATH_x86}\StOutAnaglyph.dll;                DestDir: {app};                              Flags: 32bit ignoreversion;                Components: StRenderers\StOutAnaglyph
Source: {#SVIEW_DISTR_PATH_x86}\lang\*StOutAnaglyph.lng;          DestDir: {app}\lang;                         Flags: 32bit ignoreversion recursesubdirs; Components: StRenderers\StOutAnaglyph
Source: {#SVIEW_DISTR_PATH_x86}\shaders\StOutAnaglyph\*;          DestDir: {app}\shaders\StOutAnaglyph;        Flags: 32bit ignoreversion recursesubdirs; Components: StRenderers\StOutAnaglyph
Source: {#SVIEW_DISTR_PATH_AMD64}\StOutAnaglyph.dll;              DestDir: {app}\amd64;                        Flags: 64bit ignoreversion;                Components: StRenderers\StOutAnaglyph; Check: IsWin64
; StRenderers -> StOutDual
Source: {#SVIEW_DISTR_PATH_x86}\StOutDual.dll;                    DestDir: {app};                              Flags: 32bit ignoreversion;                Components: StRenderers\StOutDual
Source: {#SVIEW_DISTR_PATH_x86}\lang\*StOutDual.lng;              DestDir: {app}\lang;                         Flags: 32bit ignoreversion recursesubdirs; Components: StRenderers\StOutDual
Source: {#SVIEW_DISTR_PATH_AMD64}\StOutDual.dll;                  DestDir: {app}\amd64;                        Flags: 64bit ignoreversion;                Components: StRenderers\StOutDual; Check: IsWin64
; StRenderers -> StOutInterlace
Source: {#SVIEW_DISTR_PATH_x86}\StOutInterlace.dll;               DestDir: {app};                              Flags: 32bit ignoreversion;                Components: StRenderers\StOutInterlace
Source: {#SVIEW_DISTR_PATH_x86}\lang\*StOutInterlace.lng;         DestDir: {app}\lang;                         Flags: 32bit ignoreversion recursesubdirs; Components: StRenderers\StOutInterlace
Source: {#SVIEW_DISTR_PATH_x86}\shaders\StOutInterlace\*;         DestDir: {app}\shaders\StOutInterlace;       Flags: 32bit ignoreversion recursesubdirs; Components: StRenderers\StOutInterlace
Source: {#SVIEW_DISTR_PATH_AMD64}\StOutInterlace.dll;             DestDir: {app}\amd64;                        Flags: 64bit ignoreversion;                Components: StRenderers\StOutInterlace; Check: IsWin64
; StRenderers -> StOutIZ3D
Source: {#SVIEW_DISTR_PATH_x86}\StOutIZ3D.dll;                    DestDir: {app};                              Flags: 32bit ignoreversion;                Components: StRenderers\StOutIZ3D
Source: {#SVIEW_DISTR_PATH_x86}\lang\*StOutIZ3D.lng;              DestDir: {app}\lang;                         Flags: 32bit ignoreversion recursesubdirs; Components: StRenderers\StOutIZ3D
Source: {#SVIEW_DISTR_PATH_x86}\shaders\StOutIZ3D\*;              DestDir: {app}\shaders\StOutIZ3D;            Flags: 32bit ignoreversion recursesubdirs; Components: StRenderers\StOutIZ3D
Source: {#SVIEW_DISTR_PATH_AMD64}\StOutIZ3D.dll;                  DestDir: {app}\amd64;                        Flags: 64bit ignoreversion;                Components: StRenderers\StOutIZ3D; Check: IsWin64
; StRenderers -> StOutPageFlip
Source: {#SVIEW_DISTR_PATH_x86}\StOutPageFlip.dll;                DestDir: {app};                              Flags: 32bit ignoreversion;                Components: StRenderers\StOutPageFlip
Source: {#SVIEW_DISTR_PATH_x86}\lang\*StOutPageFlip.lng;          DestDir: {app}\lang;                         Flags: 32bit ignoreversion recursesubdirs; Components: StRenderers\StOutPageFlip
Source: {#SVIEW_DISTR_PATH_AMD64}\StOutPageFlip.dll;              DestDir: {app}\amd64;                        Flags: 64bit ignoreversion;                Components: StRenderers\StOutPageFlip; Check: IsWin64
; StRenderers -> StOutDistorted
Source: {#SVIEW_DISTR_PATH_x86}\StOutDistorted.dll;               DestDir: {app};                              Flags: 32bit ignoreversion;                Components: StRenderers\StOutDistorted
Source: {#SVIEW_DISTR_PATH_x86}\lang\*StOutDistorted.lng;         DestDir: {app}\lang;                         Flags: 32bit ignoreversion recursesubdirs; Components: StRenderers\StOutDistorted
Source: {#SVIEW_DISTR_PATH_AMD64}\StOutDistorted.dll;             DestDir: {app}\amd64;                        Flags: 64bit ignoreversion;                Components: StRenderers\StOutDistorted; Check: IsWin64

; StDrawers
Source: {#SVIEW_DISTR_PATH_x86}\textures\*;                    DestDir: {app}\textures;        Flags: 32bit ignoreversion; Components: StDrawers\StImageViewer or StDrawers\StMoviePlayer or StBrowserPlugins or StActiveXPlugin
Source: {#SVIEW_DISTR_PATH_x86}\icons\sView_Media.ico;         DestDir: {app}\icons;           Flags: 32bit ignoreversion; Components: StDrawers\StImageViewer or StDrawers\StMoviePlayer
; StDrawers -> Image Viewer
Source: {#SVIEW_DISTR_PATH_x86}\StImageViewer.dll;             DestDir: {app};                 Flags: 32bit ignoreversion; Components: StCore
Source: {#SVIEW_DISTR_PATH_x86}\lang\*StImageViewer.lng;       DestDir: {app}\lang;            Flags: 32bit ignoreversion recursesubdirs; Components: StCore
Source: {#SVIEW_DISTR_PATH_x86}\icons\sView_JPS.ico;           DestDir: {app}\icons;           Flags: 32bit ignoreversion; Components: StDrawers\StImageViewer
Source: {#SVIEW_DISTR_PATH_x86}\icons\sView_PNS.ico;           DestDir: {app}\icons;           Flags: 32bit ignoreversion; Components: StDrawers\StImageViewer
Source: {#SVIEW_DISTR_PATH_x86}\demo.jps;                      DestDir: {app};                 Flags: 32bit ignoreversion; Components: StDrawers\StImageViewer
Source: {#SVIEW_DISTR_PATH_AMD64}\StImageViewer.dll;           DestDir: {app}\amd64;           Flags: 64bit ignoreversion; Components: StCore; Check: IsWin64
; FreeImage library (should be optional)
Source: {#SVIEW_DISTR_PATH_x86}\FreeImage.dll;                 DestDir: {app};                 Flags: 32bit ignoreversion; Components: StDrawers\StImageViewer
Source: {#SVIEW_DISTR_PATH_AMD64}\FreeImage.dll;               DestDir: {app}\amd64;           Flags: 64bit ignoreversion; Components: StDrawers\StImageViewer; Check: IsWin64
; DevIL libraries (should be optional)
Source: {#SVIEW_DISTR_PATH_x86}\DevIL.dll;                     DestDir: {app};                 Flags: 32bit ignoreversion; Components: StDrawers\StImageViewer
Source: {#SVIEW_DISTR_PATH_x86}\ILU.dll;                       DestDir: {app};                 Flags: 32bit ignoreversion; Components: StDrawers\StImageViewer
Source: {#SVIEW_DISTR_PATH_AMD64}\DevIL.dll;                   DestDir: {app}\amd64;           Flags: 64bit ignoreversion; Components: StDrawers\StImageViewer; Check: IsWin64
Source: {#SVIEW_DISTR_PATH_AMD64}\ILU.dll;                     DestDir: {app}\amd64;           Flags: 64bit ignoreversion; Components: StDrawers\StImageViewer; Check: IsWin64

; StDrawers -> Movie Player
Source: {#SVIEW_DISTR_PATH_x86}\StMoviePlayer.dll;             DestDir: {app};                 Flags: 32bit ignoreversion; Components: StCore
Source: {#SVIEW_DISTR_PATH_x86}\lang\*StMoviePlayer.lng;       DestDir: {app}\lang;            Flags: 32bit ignoreversion recursesubdirs; Components: StCore
Source: {#SVIEW_DISTR_PATH_x86}\web\*.htm;                     DestDir: {app}\web;             Flags: 32bit ignoreversion recursesubdirs; Components: StDrawers\StMoviePlayer
Source: {#SVIEW_DISTR_PATH_x86}\OpenAL32.dll;                  DestDir: {app};                 Flags: 32bit ignoreversion; Components: StCore
Source: alsoft51.ini;                                          DestDir: {userappdata};   DestName: "alsoft.ini";     Tasks: flagOpenAL51; Components: StCore
Source: {#SVIEW_DISTR_PATH_AMD64}\StMoviePlayer.dll;           DestDir: {app}\amd64;           Flags: 64bit ignoreversion; Components: StCore; Check: IsWin64
Source: {#SVIEW_DISTR_PATH_AMD64}\OpenAL32.dll;                DestDir: {app}\amd64;           Flags: 64bit ignoreversion; Components: StCore; Check: IsWin64

; StDrawers -> Tiny CAD viewer
Source: {#SVIEW_DISTR_PATH_x86}\StCADViewer.dll;               DestDir: {app};                 Flags: 32bit ignoreversion; Components: StCore
Source: {#SVIEW_DISTR_PATH_AMD64}\StCADViewer.dll;             DestDir: {app}\amd64;           Flags: 64bit ignoreversion; Components: StCore; Check: IsWin64

[Icons]
Name: {group}\sView - Image Viewer; Filename: {app}\{#SVIEW_EXE_NAME};       Components: StDrawers\StImageViewer; IconFilename: {app}\{#SVIEW_EXE_NAME};       Comment: "{cm:StImageViewer}";  IconIndex: 0; Parameters: "--in=image - ""{app}\demo.jps"""; Check: not IsWin64
Name: {group}\sView - Image Viewer; Filename: {app}\amd64\{#SVIEW_EXE_NAME}; Components: StDrawers\StImageViewer; IconFilename: {app}\amd64\{#SVIEW_EXE_NAME}; Comment: "{cm:StImageViewer}";  IconIndex: 0; Parameters: "--in=image - ""{app}\demo.jps"""; Check: IsWin64
Name: {group}\sView - Movie Player; Filename: {app}\{#SVIEW_EXE_NAME};       Components: StDrawers\StMoviePlayer; IconFilename: {app}\{#SVIEW_EXE_NAME};       Comment: "{cm:StMoviePlayer}";  IconIndex: 0; Parameters: "--in=video"; Check: not IsWin64
Name: {group}\sView - Movie Player; Filename: {app}\amd64\{#SVIEW_EXE_NAME}; Components: StDrawers\StMoviePlayer; IconFilename: {app}\amd64\{#SVIEW_EXE_NAME}; Comment: "{cm:StMoviePlayer}";  IconIndex: 0; Parameters: "--in=video"; Check: IsWin64
Name: {group}\Extras\sView - Failsafe;                    Filename: {app}\{#SVIEW_EXE_NAME};       Components: StDrawers\StImageViewer and StRenderers\StOutAnaglyph; IconFilename: {app}\{#SVIEW_EXE_NAME};       Comment: Failsafe sView launch; IconIndex: 0; Parameters: --out=StOutAnaglyph --in=image
Name: {group}\Extras\sView - Autodetection;               Filename: {app}\{#SVIEW_EXE_NAME};       Components: StDrawers\StImageViewer; IconFilename: {app}\{#SVIEW_EXE_NAME};       Comment: Failsafe sView launch; IconIndex: 0; Parameters: --out=Auto --in=image
Name: {group}\Extras\sView - Movie Player 32bit;          Filename: {app}\{#SVIEW_EXE_NAME};       Components: StDrawers\StMoviePlayer; IconFilename: {app}\{#SVIEW_EXE_NAME};       Comment: "{cm:StMoviePlayer}";  IconIndex: 0; Parameters: "--in=video";         Check: IsWin64
Name: {group}\Extras\sView - Movie Player (Last File);    Filename: {app}\{#SVIEW_EXE_NAME};       Components: StDrawers\StMoviePlayer; IconFilename: {app}\{#SVIEW_EXE_NAME};       Comment: "{cm:StMoviePlayer}";  IconIndex: 0; Parameters: "--last";             Check: not IsWin64
Name: {group}\Extras\sView - Movie Player (Last File);    Filename: {app}\amd64\{#SVIEW_EXE_NAME}; Components: StDrawers\StMoviePlayer; IconFilename: {app}\amd64\{#SVIEW_EXE_NAME}; Comment: "{cm:StMoviePlayer}";  IconIndex: 0; Parameters: "--last";             Check: IsWin64
Name: {group}\Extras\sView - Diagnostics;                 Filename: {app}\{#SVIEW_EXE_NAME};       Components: StCore;                  IconFilename: {app}\{#SVIEW_EXE_NAME};       Comment: sView Diagnostics;     IconIndex: 0; Parameters: "--in=StDiagnostics"; Check: not IsWin64
Name: {group}\Extras\sView - Diagnostics;                 Filename: {app}\amd64\{#SVIEW_EXE_NAME}; Components: StCore;                  IconFilename: {app}\amd64\{#SVIEW_EXE_NAME}; Comment: sView Diagnostics;     IconIndex: 0; Parameters: "--in=StDiagnostics"; Check: IsWin64
Name: {group}\Extras\Monitors Dump;                       Filename: {app}\StMonitorsDump.exe;       Components: StCore;                 Comment: Information about connected displays; Check: not IsWin64
Name: {group}\Extras\Monitors Dump;                       Filename: {app}\amd64\StMonitorsDump.exe; Components: StCore;                 Comment: Information about connected displays; Check: IsWin64

;Name: {group}\user manual;          Filename: {app}\info\manual.pdf;         Comment: User manual; Tasks: ; Languages:
Name: {group}\{cm:UninstallProgram,{#SVIEW_NAME}}; Filename: {uninstallexe}
Name: {commondesktop}\{#SVIEW_NAME};Filename: {app}\{#SVIEW_EXE_NAME};       Tasks: desktopicon; Components: StDrawers\StImageViewer; IconFilename: {app}\{#SVIEW_EXE_NAME};       Comment: "{cm:StImageViewer}"; IconIndex: 0; Parameters: "--in=image - ""{app}\demo.jps"""; Check: not IsWin64
Name: {commondesktop}\{#SVIEW_NAME};Filename: {app}\amd64\{#SVIEW_EXE_NAME}; Tasks: desktopicon; Components: StDrawers\StImageViewer; IconFilename: {app}\amd64\{#SVIEW_EXE_NAME}; Comment: "{cm:StImageViewer}"; IconIndex: 0; Parameters: "--in=image - ""{app}\demo.jps"""; Check: IsWin64
Name: {userappdata}\Microsoft\Internet Explorer\Quick Launch\{#SVIEW_NAME}; Filename: {app}\{#SVIEW_EXE_NAME};       Tasks: quicklaunchicon; Components: StDrawers\StImageViewer; IconFilename: {app}\{#SVIEW_EXE_NAME};       Comment: "{cm:StImageViewer}"; IconIndex: 0; Parameters: "--in=image - ""{app}\demo.jps"""; Check: not IsWin64
Name: {userappdata}\Microsoft\Internet Explorer\Quick Launch\{#SVIEW_NAME}; Filename: {app}\amd64\{#SVIEW_EXE_NAME}; Tasks: quicklaunchicon; Components: StDrawers\StImageViewer; IconFilename: {app}\amd64\{#SVIEW_EXE_NAME}; Comment: "{cm:StImageViewer}"; IconIndex: 0; Parameters: "--in=image - ""{app}\demo.jps"""; Check: IsWin64

[Run]
;Filename: {app}\{#SVIEW_EXE_NAME}; WorkingDir: {app}; Components: StDrawers\StImageViewer; Description: {cm:LaunchProgram,{#SVIEW_NAME}}; Parameters: --in=image - demo.jps; Flags: nowait postinstall skipifsilent
; ActiveX Control registration
Filename: regsvr32; Parameters: "/s ""{app}\npStBrowserPlugin.dll""";       Components: StActiveXPlugin; Flags: 32bit
Filename: regsvr32; Parameters: "/s ""{app}\amd64\npStBrowserPlugin.dll"""; Components: StActiveXPlugin; Flags: 64bit; Check: IsWin64

[Registry]
; Install/Uninstall info
Root: HKCU; Subkey: SOFTWARE\sView;       ValueType: none; Flags: uninsdeletekey; Tasks: ; Languages: ; ValueData: sView 2011
Root: HKCU; Subkey: Software\sView\sView; ValueType: string; ValueName: language; ValueData: русский;  Tasks: ; Languages: russian; Flags: uninsdeletekey
Root: HKCU; Subkey: Software\sView\sView; ValueType: string; ValueName: language; ValueData: English;  Tasks: ; Languages: english; Flags: uninsdeletekey
Root: HKCU; Subkey: Software\sView\sView; ValueType: string; ValueName: language; ValueData: français; Tasks: ; Languages: french;  Flags: uninsdeletekey
Root: HKCU; Subkey: Software\sView\sView; ValueType: string; ValueName: language; ValueData: Deutsch;  Tasks: ; Languages: german;  Flags: uninsdeletekey
Root: HKCU; Subkey: Software\sView\sView; ValueType: string; ValueName: language; ValueData: Korean;   Tasks: ; Languages: korean;  Flags: uninsdeletekey
Root: HKCU; Subkey: Software\sView\sView; ValueType: string; ValueName: language; ValueData: ChineseS; Tasks: ; Languages: chinese; Flags: uninsdeletekey
; Associations JPS
Root: HKCR; SubKey: .jps;                                      ValueType: string; ValueData: JPEG Stereo Image;             Tasks: flagAssocStereoImages; Flags: uninsdeletekey
Root: HKCR; SubKey: JPEG Stereo Image;                         ValueType: string; ValueData: JPEG Stereo Image;             Tasks: flagAssocStereoImages; Flags: uninsdeletekey
Root: HKCR; SubKey: JPEG Stereo Image\Shell\Open\Command;      ValueType: string; ValueData: """{app}\{#SVIEW_EXE_NAME}"" --in=image - ""%1"""; Tasks: flagAssocStereoImages; Flags: uninsdeletevalue; Check: not IsWin64
Root: HKCR; SubKey: JPEG Stereo Image\Shell\Open\Command;      ValueType: string; ValueData: """{app}\amd64\{#SVIEW_EXE_NAME}"" --in=image - ""%1"""; Tasks: flagAssocStereoImages; Flags: uninsdeletevalue; Check: IsWin64
Root: HKCR; Subkey: JPEG Stereo Image\DefaultIcon;             ValueType: string; ValueData: {app}\icons\sView_JPS.ico;     Tasks: flagAssocStereoImages; Flags: uninsdeletevalue
; Associations PNS
Root: HKCR; SubKey: .pns;                                      ValueType: string; ValueData: PNG Stereo Image;              Tasks: flagAssocStereoImages; Flags: uninsdeletekey
Root: HKCR; SubKey: PNG Stereo Image;                          ValueType: string; ValueData: PNG Stereo Image;              Tasks: flagAssocStereoImages; Flags: uninsdeletekey
Root: HKCR; SubKey: PNG Stereo Image\Shell\Open\Command;       ValueType: string; ValueData: """{app}\{#SVIEW_EXE_NAME}"" --in=image - ""%1"""; Tasks: flagAssocStereoImages; Flags: uninsdeletevalue; Check: not IsWin64
Root: HKCR; SubKey: PNG Stereo Image\Shell\Open\Command;       ValueType: string; ValueData: """{app}\amd64\{#SVIEW_EXE_NAME}"" --in=image - ""%1"""; Tasks: flagAssocStereoImages; Flags: uninsdeletevalue; Check: IsWin64
Root: HKCR; Subkey: PNG Stereo Image\DefaultIcon;              ValueType: string; ValueData: {app}\icons\sView_PNS.ico;     Tasks: flagAssocStereoImages; Flags: uninsdeletevalue
; Associations MPO
Root: HKCR; SubKey: .mpo;                                      ValueType: string; ValueData: Multi Picture Object;          Tasks: flagAssocStereoImages; Flags: uninsdeletekey
Root: HKCR; SubKey: Multi Picture Object;                      ValueType: string; ValueData: Multi Picture Object;          Tasks: flagAssocStereoImages; Flags: uninsdeletekey
Root: HKCR; SubKey: Multi Picture Object\Shell\Open\Command;   ValueType: string; ValueData: """{app}\{#SVIEW_EXE_NAME}"" --in=image - ""%1"""; Tasks: flagAssocStereoImages; Flags: uninsdeletevalue; Check: not IsWin64
Root: HKCR; SubKey: Multi Picture Object\Shell\Open\Command;   ValueType: string; ValueData: """{app}\amd64\{#SVIEW_EXE_NAME}"" --in=image - ""%1"""; Tasks: flagAssocStereoImages; Flags: uninsdeletevalue; Check: IsWin64
Root: HKCR; Subkey: Multi Picture Object\DefaultIcon;          ValueType: string; ValueData: {app}\icons\sView_Media.ico;   Tasks: flagAssocStereoImages; Flags: uninsdeletevalue

; Associations Images
Root: HKCR; SubKey: sView Image;                               ValueType: string; ValueData: sView Image;                   Tasks: flagAssocImages;       Flags: uninsdeletekey
Root: HKCR; SubKey: sView Image\Shell\Open\Command;            ValueType: string; ValueData: """{app}\{#SVIEW_EXE_NAME}"" --in=image - ""%1""";       Tasks: flagAssocImages; Flags: uninsdeletevalue; Check: not IsWin64
Root: HKCR; SubKey: sView Image\Shell\Open\Command;            ValueType: string; ValueData: """{app}\amd64\{#SVIEW_EXE_NAME}"" --in=image - ""%1"""; Tasks: flagAssocImages; Flags: uninsdeletevalue; Check: IsWin64
Root: HKCR; Subkey: sView Image\DefaultIcon;                   ValueType: string; ValueData: {app}\icons\sView_Media.ico;   Tasks: flagAssocImages;       Flags: uninsdeletevalue

Root: HKCR; SubKey: .bmp;                                      ValueType: string; ValueData: sView Image;                   Tasks: flagAssocImages;       Flags: uninsdeletekey
Root: HKCR; SubKey: .jpg;                                      ValueType: string; ValueData: sView Image;                   Tasks: flagAssocImages;       Flags: uninsdeletekey
Root: HKCR; SubKey: .tga;                                      ValueType: string; ValueData: sView Image;                   Tasks: flagAssocImages;       Flags: uninsdeletekey
Root: HKCR; SubKey: .png;                                      ValueType: string; ValueData: sView Image;                   Tasks: flagAssocImages;       Flags: uninsdeletekey
Root: HKCR; SubKey: .exr;                                      ValueType: string; ValueData: sView Image;                   Tasks: flagAssocImages;       Flags: uninsdeletekey
Root: HKCR; SubKey: .hdr;                                      ValueType: string; ValueData: sView Image;                   Tasks: flagAssocImages;       Flags: uninsdeletekey
Root: HKCR; SubKey: .webp;                                     ValueType: string; ValueData: sView Image;                   Tasks: flagAssocImages;       Flags: uninsdeletekey
Root: HKCR; SubKey: .webpll;                                   ValueType: string; ValueData: sView Image;                   Tasks: flagAssocImages;       Flags: uninsdeletekey

; Associations Video
Root: HKCR; SubKey: sView Video;                               ValueType: string; ValueData: sView Video;                   Tasks: flagAssocMovies; Flags: uninsdeletekey
Root: HKCR; SubKey: sView Video\Shell\Open\Command;            ValueType: string; ValueData: """{app}\{#SVIEW_EXE_NAME}"" --in=video - ""%1"""; Tasks: flagAssocMovies; Flags: uninsdeletevalue; Check: not IsWin64
Root: HKCR; SubKey: sView Video\Shell\Open\Command;            ValueType: string; ValueData: """{app}\amd64\{#SVIEW_EXE_NAME}"" --in=video - ""%1"""; Tasks: flagAssocMovies; Flags: uninsdeletevalue; Check: IsWin64
Root: HKCR; Subkey: sView Video\DefaultIcon;                   ValueType: string; ValueData: {app}\icons\sView_Media.ico;   Tasks: flagAssocMovies; Flags: uninsdeletevalue

Root: HKCR; SubKey: .avi;                                      ValueType: string; ValueData: sView Video;                   Tasks: flagAssocMovies; Flags: uninsdeletekey
Root: HKCR; SubKey: .mkv;                                      ValueType: string; ValueData: sView Video;                   Tasks: flagAssocMovies; Flags: uninsdeletekey
Root: HKCR; SubKey: .mk3d;                                     ValueType: string; ValueData: sView Video;                   Tasks: flagAssocMovies; Flags: uninsdeletekey
Root: HKCR; SubKey: .webm;                                     ValueType: string; ValueData: sView Video;                   Tasks: flagAssocMovies; Flags: uninsdeletekey
Root: HKCR; SubKey: .wmv;                                      ValueType: string; ValueData: sView Video;                   Tasks: flagAssocMovies; Flags: uninsdeletekey
Root: HKCR; SubKey: .ts;                                       ValueType: string; ValueData: sView Video;                   Tasks: flagAssocMovies; Flags: uninsdeletekey

; Associations Audio
Root: HKCR; SubKey: sView Audio;                               ValueType: string; ValueData: sView Audio;                   Tasks: flagAssocMusic;  Flags: uninsdeletekey
Root: HKCR; SubKey: sView Audio\Shell\Open\Command;            ValueType: string; ValueData: """{app}\{#SVIEW_EXE_NAME}"" --in=video - ""%1"""; Tasks: flagAssocMusic; Flags: uninsdeletevalue; Check: not IsWin64
Root: HKCR; SubKey: sView Audio\Shell\Open\Command;            ValueType: string; ValueData: """{app}\amd64\{#SVIEW_EXE_NAME}"" --in=video - ""%1"""; Tasks: flagAssocMusic; Flags: uninsdeletevalue; Check: IsWin64
Root: HKCR; Subkey: sView Audio\DefaultIcon;                   ValueType: string; ValueData: {app}\icons\sView_Media.ico;   Tasks: flagAssocMusic; Flags: uninsdeletevalue

Root: HKCR; SubKey: .mp3;                                      ValueType: string; ValueData: sView Audio;                   Tasks: flagAssocMusic;  Flags: uninsdeletekey
Root: HKCR; SubKey: .m4a;                                      ValueType: string; ValueData: sView Audio;                   Tasks: flagAssocMusic;  Flags: uninsdeletekey
Root: HKCR; SubKey: .aac;                                      ValueType: string; ValueData: sView Audio;                   Tasks: flagAssocMusic;  Flags: uninsdeletekey
Root: HKCR; SubKey: .ogg;                                      ValueType: string; ValueData: sView Audio;                   Tasks: flagAssocMusic;  Flags: uninsdeletekey
Root: HKCR; SubKey: .opus;                                     ValueType: string; ValueData: sView Audio;                   Tasks: flagAssocMusic;  Flags: uninsdeletekey
Root: HKCR; SubKey: .wav;                                      ValueType: string; ValueData: sView Audio;                   Tasks: flagAssocMusic;  Flags: uninsdeletekey
Root: HKCR; SubKey: .flac;                                     ValueType: string; ValueData: sView Audio;                   Tasks: flagAssocMusic;  Flags: uninsdeletekey
Root: HKCR; SubKey: .ape;                                      ValueType: string; ValueData: sView Audio;                   Tasks: flagAssocMusic;  Flags: uninsdeletekey

; StCoreXX environment variables
Root: HKLM32; SubKey: SYSTEM\CurrentControlSet\Control\Session Manager\Environment; ValueType: string; ValueName: StShare;  ValueData: {app}\; Flags: uninsdeletevalue
Root: HKCU32; SubKey: Environment;                                                  ValueType: string; ValueName: StShare;  ValueData: {app}\; Flags: uninsdeletevalue
Root: HKLM32; SubKey: SYSTEM\CurrentControlSet\Control\Session Manager\Environment; ValueType: string; ValueName: StCore32; ValueData: {app}\; Flags: uninsdeletevalue
Root: HKCU32; SubKey: Environment;                                                  ValueType: string; ValueName: StCore32; ValueData: {app}\; Flags: uninsdeletevalue
Root: HKLM64; SubKey: SYSTEM\CurrentControlSet\Control\Session Manager\Environment; ValueType: string; ValueName: StCore64; ValueData: {app}\amd64\; Flags: uninsdeletevalue; Check: IsWin64
Root: HKCU64; SubKey: Environment;                                                  ValueType: string; ValueName: StCore64; ValueData: {app}\amd64\; Flags: uninsdeletevalue; Check: IsWin64
; x86 NPAPI Browser plugin registration
Root: HKLM32; SubKey: SOFTWARE\MozillaPlugins\@sview.ru/sView;   ValueType: string; ValueName: Path;        ValueData: {app}\npStBrowserPlugin.dll; Components: StBrowserPlugins; Flags: uninsdeletekey
Root: HKLM32; SubKey: SOFTWARE\MozillaPlugins\@sview.ru/sView;   ValueType: string; ValueName: ProductName; ValueData: sView;                  Components: StBrowserPlugins; Flags: uninsdeletekey
Root: HKLM32; SubKey: SOFTWARE\MozillaPlugins\@sview.ru/sView;   ValueType: string; ValueName: Vendor;      ValueData: sView;                  Components: StBrowserPlugins; Flags: uninsdeletekey
Root: HKLM32; SubKey: SOFTWARE\MozillaPlugins\@sview.ru/sView;   ValueType: string; ValueName: Description; ValueData: sView - Browser Plugin; Components: StBrowserPlugins; Flags: uninsdeletekey
; AMD64 NPAPI Browser plugin registration
Root: HKLM64; SubKey: SOFTWARE\MozillaPlugins\@sview.ru/sView;   ValueType: string; ValueName: Path;        ValueData: {app}\amd64\npStBrowserPlugin.dll; Components: StBrowserPlugins; Flags: uninsdeletekey; Check: IsWin64
Root: HKLM64; SubKey: SOFTWARE\MozillaPlugins\@sview.ru/sView;   ValueType: string; ValueName: ProductName; ValueData: sView;                  Components: StBrowserPlugins; Flags: uninsdeletekey; Check: IsWin64
Root: HKLM64; SubKey: SOFTWARE\MozillaPlugins\@sview.ru/sView;   ValueType: string; ValueName: Vendor;      ValueData: sView;                  Components: StBrowserPlugins; Flags: uninsdeletekey; Check: IsWin64
Root: HKLM64; SubKey: SOFTWARE\MozillaPlugins\@sview.ru/sView;   ValueType: string; ValueName: Description; ValueData: sView - Browser Plugin; Components: StBrowserPlugins; Flags: uninsdeletekey; Check: IsWin64
; NPAPI Browser plugin MIME types
Root: HKLM; SubKey: SOFTWARE\MozillaPlugins\@sview.ru/sView\MimeTypes\image/jps;   ValueType: string; ValueName: Description; ValueData: JPS - jpeg stereo image;    Components: StBrowserPlugins; Flags: uninsdeletekey
Root: HKLM; SubKey: SOFTWARE\MozillaPlugins\@sview.ru/sView\MimeTypes\image/jps;   ValueType: string; ValueName: Suffixes;    ValueData: jps;                        Components: StBrowserPlugins; Flags: uninsdeletekey
Root: HKLM; SubKey: SOFTWARE\MozillaPlugins\@sview.ru/sView\MimeTypes\image/x-jps; ValueType: string; ValueName: Description; ValueData: JPS - jpeg stereo image;    Components: StBrowserPlugins; Flags: uninsdeletekey
Root: HKLM; SubKey: SOFTWARE\MozillaPlugins\@sview.ru/sView\MimeTypes\image/x-jps; ValueType: string; ValueName: Suffixes;    ValueData: jps;                        Components: StBrowserPlugins; Flags: uninsdeletekey
Root: HKLM; SubKey: SOFTWARE\MozillaPlugins\@sview.ru/sView\MimeTypes\image/pns;   ValueType: string; ValueName: Description; ValueData: PNS - png stereo image;     Components: StBrowserPlugins; Flags: uninsdeletekey
Root: HKLM; SubKey: SOFTWARE\MozillaPlugins\@sview.ru/sView\MimeTypes\image/pns;   ValueType: string; ValueName: Suffixes;    ValueData: pns;                        Components: StBrowserPlugins; Flags: uninsdeletekey
Root: HKLM; SubKey: SOFTWARE\MozillaPlugins\@sview.ru/sView\MimeTypes\image/x-pns; ValueType: string; ValueName: Description; ValueData: PNS - png stereo image;     Components: StBrowserPlugins; Flags: uninsdeletekey
Root: HKLM; SubKey: SOFTWARE\MozillaPlugins\@sview.ru/sView\MimeTypes\image/x-pns; ValueType: string; ValueName: Suffixes;    ValueData: pns;                        Components: StBrowserPlugins; Flags: uninsdeletekey
Root: HKLM; SubKey: SOFTWARE\MozillaPlugins\@sview.ru/sView\MimeTypes\image/mpo;   ValueType: string; ValueName: Description; ValueData: MPO - multi picture object; Components: StBrowserPlugins; Flags: uninsdeletekey
Root: HKLM; SubKey: SOFTWARE\MozillaPlugins\@sview.ru/sView\MimeTypes\image/mpo;   ValueType: string; ValueName: Suffixes;    ValueData: mpo;                        Components: StBrowserPlugins; Flags: uninsdeletekey
Root: HKLM; SubKey: SOFTWARE\MozillaPlugins\@sview.ru/sView\MimeTypes\image/x-mpo; ValueType: string; ValueName: Description; ValueData: MPO - multi picture object; Components: StBrowserPlugins; Flags: uninsdeletekey
Root: HKLM; SubKey: SOFTWARE\MozillaPlugins\@sview.ru/sView\MimeTypes\image/x-mpo; ValueType: string; ValueName: Suffixes;    ValueData: mpo;                        Components: StBrowserPlugins; Flags: uninsdeletekey
Root: HKLM; SubKey: SOFTWARE\MozillaPlugins\@sview.ru/sView\Suffixes;              ValueType: string; ValueName: jps;                                                Components: StBrowserPlugins; Flags: uninsdeletekey
Root: HKLM; SubKey: SOFTWARE\MozillaPlugins\@sview.ru/sView\Suffixes;              ValueType: string; ValueName: pns;                                                Components: StBrowserPlugins; Flags: uninsdeletekey
Root: HKLM; SubKey: SOFTWARE\MozillaPlugins\@sview.ru/sView\Suffixes;              ValueType: string; ValueName: mpo;                                                Components: StBrowserPlugins; Flags: uninsdeletekey
; ActiveX Control MIME types registration
Root: HKCR; SubKey: MIME\Database\Content Type\image/jps;   ValueType: string; ValueName: CLSID;     ValueData: {{027792d0-5136-4ea3-9bec-34276dfe4362}; Components: StActiveXPlugin; Flags: uninsdeletekey
Root: HKCR; SubKey: MIME\Database\Content Type\image/jps;   ValueType: string; ValueName: Extension; ValueData: .jps;                                    Components: StActiveXPlugin; Flags: uninsdeletekey
Root: HKCR; SubKey: MIME\Database\Content Type\image/x-jps; ValueType: string; ValueName: CLSID;     ValueData: {{027792d0-5136-4ea3-9bec-34276dfe4362}; Components: StActiveXPlugin; Flags: uninsdeletekey
Root: HKCR; SubKey: MIME\Database\Content Type\image/x-jps; ValueType: string; ValueName: Extension; ValueData: .jps;                                    Components: StActiveXPlugin; Flags: uninsdeletekey
Root: HKCR; SubKey: MIME\Database\Content Type\image/pns;   ValueType: string; ValueName: CLSID;     ValueData: {{027792d0-5136-4ea3-9bec-34276dfe4362}; Components: StActiveXPlugin; Flags: uninsdeletekey
Root: HKCR; SubKey: MIME\Database\Content Type\image/pns;   ValueType: string; ValueName: Extension; ValueData: .pns;                                    Components: StActiveXPlugin; Flags: uninsdeletekey
Root: HKCR; SubKey: MIME\Database\Content Type\image/x-pns; ValueType: string; ValueName: CLSID;     ValueData: {{027792d0-5136-4ea3-9bec-34276dfe4362}; Components: StActiveXPlugin; Flags: uninsdeletekey
Root: HKCR; SubKey: MIME\Database\Content Type\image/x-pns; ValueType: string; ValueName: Extension; ValueData: .pns;                                    Components: StActiveXPlugin; Flags: uninsdeletekey
Root: HKCR; SubKey: MIME\Database\Content Type\image/mpo;   ValueType: string; ValueName: CLSID;     ValueData: {{027792d0-5136-4ea3-9bec-34276dfe4362}; Components: StActiveXPlugin; Flags: uninsdeletekey
Root: HKCR; SubKey: MIME\Database\Content Type\image/mpo;   ValueType: string; ValueName: Extension; ValueData: .mpo;                                    Components: StActiveXPlugin; Flags: uninsdeletekey
Root: HKCR; SubKey: MIME\Database\Content Type\image/x-mpo; ValueType: string; ValueName: CLSID;     ValueData: {{027792d0-5136-4ea3-9bec-34276dfe4362}; Components: StActiveXPlugin; Flags: uninsdeletekey
Root: HKCR; SubKey: MIME\Database\Content Type\image/x-mpo; ValueType: string; ValueName: Extension; ValueData: .mpo;                                    Components: StActiveXPlugin; Flags: uninsdeletekey

[INI]
Filename: {app}\sview.ru.url; Section: InternetShortcut; Key: URL; String: {#SVIEW_URL}

[UninstallDelete]
Type: files; Name: {app}\sview.ru.url

[UninstallRun]
; ActiveX Control de-registration
Filename: regsvr32; Parameters: "/u /s ""{app}\npStBrowserPlugin.dll""";       Components: StActiveXPlugin; Flags: 32bit
Filename: regsvr32; Parameters: "/u /s ""{app}\amd64\npStBrowserPlugin.dll"""; Components: StActiveXPlugin; Flags: 64bit; Check: IsWin64

[Dirs]

[InstallDelete]
Name: {app}\;                  Type: filesandordirs
Name: {group}\;                Type: filesandordirs
Name: {app}\input\;            Type: filesandordirs
Name: {app}\StRenderers\;      Type: filesandordirs
Name: {app}\StBrowserPlugins\; Type: filesandordirs
