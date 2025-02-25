sView - Changelog
=================
This document describes changes between tagged sView versions.

Not all changes are documented here - please check git log for complete list.

sView ??.?? (in development)
-----------------------------------------------------------------------------------------------------------------------

- Movie Player now supports new **FFmpeg 7.1** API for decoding frame sequence stereoscopic video.
- Image Viewer now detects side-by-side stereopairs from **QooCam EGO** camera.
- Image Viewer now supports floating-point RGB(A) formats of FFmpeg (e.g. from `EXR`, `HDR` image files).
- Linux, **zenity** is now used instead of `GTK2` for open-file dialog.
- Linux, enable building with `libopenvr-dev` for **OpenVR** support (untested).
- Windows, removed 32-bit binaries from package.
- Windows, removed FreeImage and DevIL libraries from package.
- Android, taps on touchsreen now hide/show user interface.
- Android, OpenAL soft has been upgraded to 1.24.2.
- macOS, DMG packages are now built for ARM64.
- Removed legacy **NPAPI**/**ActiveX** plugin (browsers dropped support for native plugins since 2015-2018).
- Configuration, building environment moved to *CMake*.
- Fixed building with **FFmpeg 7.0+**.

sView 23.02 (2023-02-14)
-----------------------------------------------------------------------------------------------------------------------

- Upgraded to **FFmpeg 5.1** (Windows)
- Added setting for **OpenAL soft** device layout output hint.
- Added feature displaying 2 subtitiles streams in parallel.
- Added option to load subtitles on opening file.

sView 22.01 (2022-01-16)
-----------------------------------------------------------------------------------------------------------------------

- Added Traditional Chinese translation.
- Added smoothed (optimized) output option for interlaced stereoscopic displays (improves small text readability).
- Added handling of **deep color** (**30-bit RGB**) on Windows and Linux platforms (on supported configurations).
- Windows, added multi-touch screen input support.
- Windows, changed non-exclusive fullscreen mode to workaround performance issues on Radeon + `4K` display.

sView 20.08 (2020-08-01)
-----------------------------------------------------------------------------------------------------------------------

- Added support for **Equiangular Cubemaps** (**EAC**).
- Fixed various issues / distortions on **HTC Vive**.
- Android, fixed accessibility of some controls (scrolling long menus in landscape mode, subtitles size and position).
- macOS, ported from `VDA` to `VideoToolbox` hardware accelerated decoding.
- macOS, minimal supported system is now macOS 10.10+ Yosemite
  (previous release sView 17.04 supported OS X 10.6+ Snow Leopard).

sView 20.05 (2020-05-20)
-----------------------------------------------------------------------------------------------------------------------

- Added **Spanish** translation files.
- Image Viewer - added trilinear texture filtering option generating mip-maps.
- Image Viewer - added support for `1x6` cubemap layout.
- Image Viewer - panorama is now automatically enabled for `JPEG` files with `GPano:ProjectionType` tag.
- Image Viewer - panorama is now automatically enabled for `JPEG` files with `360Stereo`/`360Mono` tags (game screenshots).
- Image Viewer - added support for `DDS`-packed cubemaps.
- Movie Player - image-based subtitles size can be now adjusted.
- Movie Player - workaround subtitles seeking issues for attached `SRT` files.
- Movie Player - fixed `ASS` subtitles formatting issue on files without closure tags.
- Movie Player - added handling of image-based subtitles encoded as stereoscopic pair (side-by-side).
- Image Viewer - fixed poor quality on saving `JPEG` (`yuv420p`) image into `PNG` (`rgb`).
- Image Viewer - fixed displaying `WebP` images with alpha channel (`yuva` pixel formats).
- **fontconfig** library is now used to retrieve paths to system fonts on Linux.

sView 19.08 (2019-08-04)
-----------------------------------------------------------------------------------------------------------------------

- Introduced **VR180** (hemisphere) and cylindrical panorama input support.
- Image Viewer now redirects video file to Movie Player and vice versa.
- Added single-finger swipe gesture for opening next/previous file in playlist.
- Added **FFmpeg 4.1** support.
- Android, sView now supports background audio playback.
- Android, sView now indicates ultra-wide screen support.
- Android, sView now requests SD card permissions in runtime on startup to meet updated Google Play requirements.
- Android, workaround crash on a couple of Huawei devices received upgrade to Android 9.
- Android, workaround texture update performance issues on devices with Adreno graphics.
- Android, sView now handles mouse scroll input.
- Windows, fixed scrolling/zooming on touchpads supporting smooth scrolling.
- Fixed compatibility with Ubuntu 19.04.
- Fixed sporadic text formatting issues.

sView 17.10 (2017-10-23)
-----------------------------------------------------------------------------------------------------------------------

- Image Viewer - added slide-show delay parameter to Settings.
- Movie Player - added general information about the stream (bitrate, codec, `PAR`, `DAR`) in File Info dialog.
- Android, added support for Commander 3D glasses-free tablet.
- Android, added **PPTV King 7S** to auto-detection logic.
- Android, Image Viewer now uses Volume keys for navigating files in Playlist.
- Movie Player - fixed seeking to the very beginning of the file.
- Movie Player - fixed (workaround) displaying attached image within some `mp3` files.
- Fixed text rendering artifacts when using glyphs with kerning defined.
- Fixed compilation on `glibc 2.26+` (Ubuntu 17.10+).
- Android, Open File dialog now shows second SD card in hot menu.


sView 17.04 (2017-04-16)
-----------------------------------------------------------------------------------------------------------------------

- Movie Player now handles rotation info stored within video stream.
- Image Viewer, Movie Player - added overlay control for adjusting image color.
- Image Viewer, Movie Player - fixed too slow opening File Info dialog in case of long metadata.

sView 17.01 (2017-01-10)
-----------------------------------------------------------------------------------------------------------------------

- Added **OpenVR** library support, including **head tracking** for spherical panoramas (tested on **HTC Vive**).
- Movie Player - added audio positioning while playing spheric panorama video.
- Fixed saving `PNG` images when using up-to-date FFmpeg.
- Improved mixed `HiDPI`/normal screens mixture support on Windows 10
  (new Windows 10 API is now used, which has been introduced within '2016 summer update).
- Install image for OS X now comes with OpenAL Soft (system-provided OpenAL has been used before).
- Movie Player - added option to force **HRTF** mixing in **OpenAL Soft**.
- Dropped compatibility with Windows XP.

sView 16.06 (2016-07-02)
-----------------------------------------------------------------------------------------------------------------------

- Updated **FFmpeg** to **3.1.1**.
- Android, added handling of multi-touch input.
- Android, improved compatibility with file managers and mail clients.
- Android, added experimental hardware-accelerated video decoding (disabled by default).
- Movie Player - implemented GPU-accelerated conversion of `NV12` and `XYZ12` formats.
- Image Viewer now puts original filename into save file dialog.
- Movie Player - fixed saving playback time of recent file.
- Movie Player - fixed tracking playback state for keeping screen on.
- Android, fixed crash when closing application on some devices.
- Improved language change without application restart.
- Prevented creation of multiple modal dialogs.

sView 15.11 (2015-11-22)
-----------------------------------------------------------------------------------------------------------------------

- Movie Player - added support of extended remote control from command line.
- Image Viewer now reads/writes `sTER` chunk in `PNG` files for identifying side-by-side format.
- Image Viewer now shows metadata read by FFmpeg.
- File Drag&Drop now supports list of files (two files are handled as stereo pair).
- Android, added handling of keyboard input.
- Updated **OpenAL Soft** to **1.17**.
- Updated **FFmpeg** to **2.8.2**.
- Ported code onto **Oculus Rift SDK 0.8**.
- Reduced peak memory usage, improved performance using reference-counted frames.
- Windows, updated experimental **DXVA2** decoder (supports more codecs).
- Windows, fixed cursor hiding within NVIDIA **3D Vision** output (Direct3D).
- Android, fixed compatibility with some OpenGL ES 2.0 drivers supporting `GL_EXT_texture_rg` extension.
- `Esc` key is now handled within any context menu.

sView 15.10 (2015-10-04)
-----------------------------------------------------------------------------------------------------------------------

- Added support of cubemaps in format `3:2` (two rows per 3 quad sides).
- Added button to activate 360 panorama view (from cubemap or spherical panorama videos and images) on toolbar.
- Added **Czech** translation files.
- Android, implemented **device orientation tracking** within panorama mode (requires precise sensors).
- Android, added handling of device back button to close active dialog.
- Added option to automatically workaround aspect ratio of anamorphic `1080p` and `720p` videos (turned on by default).
- Windows, sView now turns off screen sleep blocking when user session has been locked.
- Improved handling of recent files when file has been selected within automatically generated playlist.
- Android, improved handling of last opened file and settings storage.
- Improved settings processing on Linux and Android - eliminated redundant file store/restore operations
  and add recovery for broken files.

sView 15.08 (2015-08-29)
-----------------------------------------------------------------------------------------------------------------------

- Introduced **Cubemap** images support (6 sides of the cube stacked horizontally).
- Added hot-keys configuration dialog.
- Added volume bar and shuffle/loop buttons to Media Player.
- Fixed issue with **HiDPI** displays within multi-monitor configurations on Windows 8.1+.
- Updated User Interface - more compact interface, new icons in menu, add scrollbars, improved **HiDPI** support.
- Improved integration into Ubuntu - fixed missing icons on taskbar.
- Improved touch-screens support.
- Many minor corrections.

sView 14.11 (2014-11-30)
-----------------------------------------------------------------------------------------------------------------------

- Porting to **FFmpeg 2.5**.
- Windows, **3D Vision** output using `Direct3D` now uses `WGL_NV_DX_interop`
  extension when available to improve performance.
- Introduced preliminary support of **Android** platform.
- Added **Chinese** (simplified) translation.
- Movie Player now prefers audio stream for active GUI language.
- Movie Player - added support for `3.0` audio streams.
- Windows - fixed window resize issues on monitors reconfiguration.
- OS X - sView now restores window title when switch from full-screen mode.
- Movie Player - improved shuffle playback.
- Movie Player - improved `m3u` playlist support.
- Movie Player - fixed seeking of `mp3` files with attached image.
- Movie Player - fixed issues on audio streams with unsupported/broken configuration.
- Image Viewer now downscales big images which do not fit into texture limits.
- Image Viewer - fixed issue reading some broken `JPEG` files.

sView 14.02 (2014-03-01)
-----------------------------------------------------------------------------------------------------------------------

- Movie Player now restores playback position on re-opening of big audio/video files.
- Movie Player, Image Viewer now detect stereoscopic format from file name using keywords
  `abq`/`lrq`/`halfou`/`halfsbs` (if file doesn't contain appropriate information).
- Movie Player, Image Viewer - disabled ambiguous side-by-side stereoscopic format detection from aspect ratio.

sView 14.01 (2014-01-30)
-----------------------------------------------------------------------------------------------------------------------

- Windows 8.1 - adjusted dimensions when window moved to monitor with another scale factor.
- Windows, Interlaced Output - sView now aligns window position to avoid temporary L/R reversion during window moves.
- Movie Player now switches to new audio/subtitles track right after attachment.
- Movie Player - added preliminary support of image-based subtitles.
- Movie Player now processes trivial `HTML` formatting tags (`<b>` and `<i>`) in text subtitles.
- Movie Player now copies subtitle text to the clipboard on `Ctrl + C`.
- Movie Player now automatically attaches audio/subtitles tracks from files with similar name.
- Image Viewer now parse/saves `_JPSJPS_` extension section in **JPEG** files.
- Image Viewer now shows stereoscopic metadata issues (missing, mismatch) in File Info dialog.
- Added support for new `stereo3d` API in FFmpeg.
- Added partial **German** translation.
- Added partial **Korean** translation.
- **Korean** and **CJK** fonts are now loaded in addition to Western when required.
- Improved and extended information provided by File Info dialog.
- Image Viewer now reads pixel aspect ratio property from image, when available.
- Image Viewer now handles **MPO** files with big thumbnails (loads full images).
- Image Viewer - improved **JPEG** parser robustness.
- Movie Player - fixed playback of `5.0` and `7.1` audio streams.
- **Oculus Rift** is now defined as a dedicated device to simplify configuration.
- Fixed text characters positioning issue (displacement at second+ displaying time).
- OS X 10.9 - added workaround for the crash in OpenAL framework.
- Improved font searching robustness on Linux.
- Linux - fixed loading of recent files from Gnome recent files menu (URLs with file protocol `file://` prefix).

sView 13.10 (2013-10-05)
-----------------------------------------------------------------------------------------------------------------------

- Windows - implemented **DPI-awareness** for Windows Vista+ and per-monitor DPI for Windows 8.1.
- Linux, `XLib` - sView now reads `Xft.dpi` property to scale interface appropriately.
- Distorted Output, Dual Output - added option to display Mono sources in Stereo.
- Movie Player - added subtitles font size control and parallax control.
- Movie Player - fixed regression, key `'W'` doesn't reverse left/right views.
- Optimized font initialization time and GPU memory usage.

sView 13.08 (2013-08-18)
-----------------------------------------------------------------------------------------------------------------------

- Mac OS X - introduced support for high pixel density displays (**Retina**).
- Added option to scale GUI.
- Movie Player - added new Audio/Video synchronization control.
- Movie Player, Web UI - added audio volume control.
- Added icons for source format menu items.
- Fixed memory corruption on deletion of last item in playlist (file deletion).
- Mac OS X - fixed application hang during fullscreen->windowed on Mountain Lion.
- Mac OS X - corrected events' timing (wrong behavior of holding key events).
- Various improvements in GUI.
- Fixed version information in DLL files.

sView 13.07 (2013-07-28)
-----------------------------------------------------------------------------------------------------------------------

- Porting to **FFmpeg 2.0** (new APIs, planar audio frames,
  disable cropping on detected tiled dual 720p in 1080p streams).
- Detect source stereo format in per-frame basis, when provided by codec.
- Mac OS X - added experimental support of decoding using `VDA` (accelerated by GPU).
- Accelerate high-range planar YUV (9/10/16 bits per component) to RGB color conversion (using `GLSL`).
- Display dual-stream video as stereo only when chosen "Autodetection" source format.
- Workaround problems with `wglMakeCurrent()` in some OpenGL drivers on Windows (possible memory leaks).

sView 13.06 (2013-06-15)
-----------------------------------------------------------------------------------------------------------------------

- Movie Player - added tiny **Web UI**.
- Added preliminary **Oculus Rift** support (stereo output).
- Minor fixes.

sView 13.05 (2013-05-25)
-----------------------------------------------------------------------------------------------------------------------

- Movie Player - added menu with recently played files.
- Movie Player - added hot keys to switch subtitles (`'T'`) and audio treks (`'L'`).
- Movie Player - added simple playlist widget (`Ctrl + L`).
- Movie Player now reads stereoscopic tags from `WMV` files.
- Added **Distorted output** for anamorphic side-by-side, over/under outputs.
- Added **Over/Under** output mode.
- Linux - fixed `XRandr` compatibility issues.
- Added FPS meter widget.
- Dual output - extended single window in full-screen mode when possible to improve synchronization.
- Browser plugin - fixed full-screen monitor binding.
- Browser now doesn't load plugin until it is not visible.
- Improved libraries packaging.
- File playback is now preserved during output device change.
- sView now doesn't create depth buffer when there is no need in it (slightly reduce GPU memory usage).
- Improved keyboard input handling.
- Improved message box widget (added Close button, added keyboard focus).
- sView now doesn't append `m3u` files to playlist; other `m3u` parser fixes.

sView 13.01 (2013-01-06)
-----------------------------------------------------------------------------------------------------------------------

- Adjust stereo separation within `'<'` and `'>'` keys to cover notebook keyboards.
- Use undo/redo buffer to keep played items order in shuffle mode.
- Detect display(s) reconfiguration during application work (to correctly launch full-screen mode).
- Movie Player - added new option to prevent system going to sleep during playback.
- Movie Player now handles Audio device disconnection during application work.
- sView now shows information about opened file via `'I'` hot-key.
- Shuffle playback now generates random order rather than the same pseudo-random order.
- Movie Player - fixed seeking in paused state.
- Movie Player - fixed switching between audio streams with different frequency.
- Fixed saving of `BMP` image in `PNG` format.
- Mac OS X - fixed opening file within Finder when sView already started.
- Mac OS X - fixed key combinations within `Shift` and `Ctrl` buttons.
- Fixed displaying of stereo images with different dimensions per view.
- Fixed regression - window position didn't restored after application restart.
- Linux - workaround bugs in some drivers (Catalyst) when window content
  doesn't redrawn after switching back from full-screen mode.

sView 12.10 (2012-10-03)
-----------------------------------------------------------------------------------------------------------------------

- PageFlip output plugin now shows on-screen warning ('require full screen mode').
- Browser plugin now downloads preview image when not in full-screen mode.
- Browser plugin - improved thread-safety.
- Browser plugin - improved browsers compatibility.
- Output plugins - reduced memory usage.
- Movie Player - fixed possible crash on exit.
- Removed dependency from `GLEW`.

sView 12.09 (2012-09-16)
-----------------------------------------------------------------------------------------------------------------------

- Added support for `webp`/`webpll` images (using `libwebp 0.2`).
- Added audio gain control.
- Added icon for sView window on Linux (`XLib`).
- Interlace output - added option to disable binding to compatible monitor at application start.
- Dual output - added menu for slave monitor selection.
- Display Aspect Ratio is now displayed in Image Viewer.
- Preliminary support for reading playlist in `M3U` format.
- Added experimental **ActiveX** component (likewise **NPAPI** plugin).
- Improved compatibility with Max OS X 10.8 Mountain Lion (removed redundant dependency from `X11` libraries).
- Improved backward seeking in Movie Player.
- Switch audio device (**OpenAL**) without application restart.
- Improved compatibility with some **OpenAL** implementations.
- Image Viewer - reduced FPS for inactive window.
- (Internal) removed dependency from patched `FTGL`.
- Improved text layout.

sView 12.06 (2012-06-06)
-----------------------------------------------------------------------------------------------------------------------
- Introduced support for **Mac OS X**, with some limitations (Intel `x86_64` CPU is required).
- Added **AMD HD3D** output support for shutter glasses (Windows-only).
- Added simple Blend deinterlacing filter to Movie Player.
- Added **French** localization.
- Added tiled stereoformat `720p-in-1080p` (used by some Europe broadcasters).
- Introduced UTF-8 subtitles output support.
- Introduced **Debian** source package (builds for Ubuntu are now available
  in [PPA](https://launchpad.net/~sview/+archive/stable) ).
- Added shuffle play-list playback to Movie Player.
- Added option to restore Aspect Ratio overridden value after program restart.
- Added **Amber-Blue Dubious** filter to Anaglyph output.
- Added inverse option for Interlaced output (for displays with different left/right lines order).
- Added full-screen switch button to Image Viewer (for consistency with Movie Player GUI).
- Orientation information now read from **JPEG** photos to apply automatic rotation.
- Added option to attach files as additional Audio/Subtitles streams to Movie Player.
- Added support for `8-bit` and `32-bit` audio formats.
- Added playback support for `mk3d` files (**matroska 3D-stereo**) within stereo format flag.
- Read parallax value stored in `MPO` images by **Fujifilm** cameras.
- Anaglyph source formats now decomposed via GPU nor CPU (performance improvement).
- Improved **NVIDIA 3DVision** output support (launch speed, `24-bit` color quality and stability).
- Improved compatibility with recent FFmpeg API.
- FFmpeg binaries migrated to version `0.10.2` (Windows builds).
- Improved compatibility with BUGs in recent AMD Catalyst releases (Interlaced and Dual outputs).
- Fixed misprint in Mirror Output that cause Y-flip mis-function.
- Slightly improved compatibility with Safari browser on Windows (`NPAPI` plugin).
- Improved compatibility with `Unicode` file paths.
- Improved compatibility with another `OpenAL` implementations (`Apple`).
- Small fixes in Drag&Drop support on `Linux` (Nautilus compatibility).
- General improvements and fixes.


sView 11.02 (2011-02-20)
-----------------------------------------------------------------------------------------------------------------------
- Added **Green/Magenta anaglyph** filter.
- Added menu item to change GUI language.
- Implemented changeable monitors configuration to allow configurations like AMD Eyefinity for advanced user.
- Added `64bit` version of **NPAPI** browser plugin (to be used with 64bit Browsers like Firefox 3.6.3 x86_64).
- **NPAPI** plugin is now registered within DEB-package installation
  (consider disabling it in the browser settings in case of stability issues).
- Extended list of MIME-types for **NPAPI** browser plugin: added `image/jps`, `image/pns` and `image/mpo`
  (improves compatibility with some galleries).
- Small fixes of GUI elements.
- Fixed BUG that may cause significant slowdown of application GUI in some cases (long system inactivity).
- Updated demo image.
- Fixed multichannel audio playback on the Linux distributives with modern FFmpeg version (Ubuntu 10.10+).
- Fixed errors that may cause OpenGL calls to release video memory when OpenGL context was already destroyed
  (may cause application crash on some OpenGL-drivers).
- Added more standard paths for standard fonts (Gentoo and Fedora).

sView 10.11 (2010-11-14)
-----------------------------------------------------------------------------------------------------------------------
- Added experimental diagnostics module (for stereoscopic displays calibration).
- Fixed left/right reversion for anaglyph and **iZ3D** outputs.
- Fixed video playback issue which may cause wrong FPS and seek problems for some videos.
- Fixed out-of-window mouse unclick problem on Windows.
- Improved compatibility with old GPU hardware without GL_RED texture support (YUV video playback).
- Slave window is now hidden on Windows with only single monitor connected (Dual Output and iZ3D output).

sView 10.08 (2010-08-22)
-----------------------------------------------------------------------------------------------------------------------
First official release.

sView 9.05 (2009-05-24)
-----------------------------------------------------------------------------------------------------------------------
First public experimental version based on new code. Changes relative to sView 2008:

- Added support of new systems: Windows Vista, Window 7 and Linux Ubuntu.
- Introduced Internet Browser plugin (**NPAPI**) for embedded stereo.
- Introduced audio/video playback via **FFmpeg** and **OpenAL** libraries.
- Enhanced devices support list, output mode auto-detection mechanism (for some devices).
- OpenGL 2 and **GLSL** (OpenGL shader language) are now used to speedup rendering on modern hardware.
- Added mono output support.
- Added spheric panorama output (`'P'` key).
- Improved fit to the screen feature.
- Introduced native **Unicode** support.
- Improved multi-monitors configurations support.
- Module architecture, another developers could provide own stereo-devices support extensions.
- Added Drag&Drop support.
- Dropped file's count limit.
- Added Windows `AMD64` builds.
- Language translations are now loaded from external text files.
- Changed version scheme, now following Ubuntu-style (YY.mm).
- Improved multi-threading support.
- Other improvements in user experience.

sView 2008
-----------------------------------------------------------------------------------------------------------------------

- **sView 0.4.0.9** (2008-01-11)
  - Improved common-picture rotation for several stereo-outputs (for mirrored output still WRONG).
  - Added QuadBuffered stereo output (hardware-accelerated).
- **sView 0.4.0.8** (2008-01-10)
  - Improved extensions in open-file dialog.
  - Changed walk-speed to FAST at install.
  - Added another anaglyph algorithm.
  - Optimized anaglyph and iZ3D modes.
  - Removed half-color anaglyph mode.
- **sView 0.4.0.6** (2007-12-13)
  - Added walk-speed choice.
  - Fixed second window position for Dual-window modes.
- **sView 0.4.0.5** (2007-12-13)
  - Improved error handling.
  - Improved iZ3D - default L/R were swapped.
  - Improved iZ3D - back color for optimal.
- **sView 0.4** (2007-12-08)
  - Added menus (in 2 languages: Russian and English).
  - Added new stereoscopic output modes (**Horizontal Mirror**, **Dual output**, **iZ3D**, **Anaglyph**, mono).
  - Added slide-show mode.
  - Main program icons have been updated to fully support Vista OS.
  - Decreased CPU usage (added sleeps).
  - Removed full-screen mode.
  - Removed BlueLine support.
  - Other improvements.
- **sView 0.3.4** (3578)
  - Fixed File Searching (last file).
  - Improved some file-change routine.
- **sView 0.3.4** (3374)
  - Added `*.pns` searching.
  - sView now tries to check OpenGL extension `"wglSwapIntervalEXT"` to force Vertical Sync ON (if possible).
  - Improved speed for load Images (for Images less Current Texture Size NO rescale).
  - Improved WinAPI BlueLine (added resolution checks).
  - DevIL library is now used for image decoding.

sView 0.1.x (2007-06-04)
-----------------------------------------------------------------------------------------------------------------------
First public win32 alpha version without graphical user interface. Key features:

- Rendering of side-by-side stereo-pair for shutter glasses (software-emulated Quadbuffered pageflip).
- Windowed and fullscreen modes.
- Navigation through the image files in the current directory (next/previous).
- Keyboard control (hot-keys).
- In first prototype console called external program to convert the image to `BMP` format.
