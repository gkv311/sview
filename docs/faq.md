sView - Frequently Asked Questions
==================================

This article contains answers for most frequently asked questions and troubleshooting.

## <a name="src_auto">Why source format auto-detection doesn't work?</a>
Because there is no way to automatically determine stereoscopic source format of arbitrary video if it does not contain extra marks.
In general auto-detection will work for files with two video streams, for *MKV* container with stereo flags or special metadata tags.
Unfortunately, these tags are still ignored by most content publishers.

If you are confused by stereo format options, please refer to [this article](stereoformats.md)

## <a name="codecs">Which video codecs sView supports?</a>
sView uses *FFmpeg* for audio/video decoding.
Please refer to [FFmpeg documentation](https://ffmpeg.org/general.html) for complete information
(actual list of enabled decoders and formats also depends on building options).

## <a name="iptv">Does sView support IPTV?</a>
Yes. Although there are no way to open video *URL* within graphical interface, you can open *M3U* file with channels list
(usually provided in such cases) or open *URL* directly from command prompt (`sView --in=video http://iptv:8030`).

## <a name="gpu">Is video playback accelerated by GPU?</a>
Yes and no. Scaling, color conversion (for most popular pixel formats) and color correction are performed on GPU using GLSL programs.

However sView doesn't use video decoding APIs like *DXVA*, *VDPAU* or any other.
The main reason is that such APIs do not fit well into existing video decoding/playback pipeline, requiring significant redesign.
With other issues like non-guarantied image quality, strict limitations on video codecs and dimensions,
and the fact that most modern CPUs are fast enough to decode most popular video formats - there no reason to make these efforts for decoding on GPU (save the low-end PCs).

## <a name="amd_white_screen">I see white window on the screen</a>
Most likely you have experienced an old issue in AMD Catalyst drivers for Windows 7,
when only full-screen stereo output worked properly.
Please update your GPU drivers to solve this problem (it is known that at least *12.6*-*12.10* releases had this problem and *13.4* not).
If driver update is not option for you, switch off pageflip stereo output using `sView`->`Extras`->`Failsafe` option in start menu.

## <a name="amd_diag">I see diagonal artifacts in full-screen mode</a>
This problem has been experienced within several AMD Catalyst driver releases.
Please try to update your GPU drivers.

## <a name="win7_aero">Why Windows 7 Aero is turned off each time I start sView?</a>
There are two known reasons why drivers decide to switch off Aero interface.
First of all Aero interface is incompatible with arbitrary technologies like 30-bit color and OpenGL Quad Buffer support (used for shutter glasses).
You might temporary switch to another stereo output mode to watch non-stereoscopic videos / images with Aero on.

Another reason might be a lack of GPU memory.
Aero interface itself requires significant amount of memory and driver might decide release these resources in favor of 3D application has been launched by user.
This is not an issue for modern hardware equipped with gigabytes of dedicated GPU memory,
but might be an issue for old ones with less than 256 MiB memory.
Notice that most drivers also switch off Aero when any 3D application goes into full-screen mode regardless of amount of on-board GPU memory.

## <a name="qb_type">I'm using shutter glasses. Which Quad Buffer type should I select?</a>
In general OpenGL Hardware is most preferable option for you.
Direct3D and OpenGL Software emulated pageflip should be used only when OpenGL Hardware Quad Buffer is unavailable on your system.

## <a name="soft_pageflip">Why Software Pageflip doesn't work?</a>
sView provides software emulated pageflip mode for shutter glasses support since it first public prototype.
Adapted application architecture, advanced techniques and special tricks have been implemented to make this functionality as stable as possible.
Unfortunately, all these efforts fall down on most modern systems due to many reasons.
To eliminate negative user experience, this mode is hidden by default under "Show extra options" in new sView releases.

This feature is unable to provide 100% stability for shutter glasses to work properly due to NO any hardware feedback used in this case.
Also you will need activate/deactivate glasses controller yourself (if manufacture provides that 'button' at all).
Without synchronization views can be randomly reversed during stereo viewing. Some recommendations to software pageflip users:

- Try to disable composition manager (*Aero* on *Windows Vista* / *Windows 7*).
- Close any other application before starting sView (especially 3D applications).
- It was noted that most problems happen during intensive I/O (file operations in background).
- Software emulation needs as less interfering factors as possible.
  If your hardware is slow you may try to reduce monitor resolution and/or frequency.
  Heavy images and videos will load your system stronger.
- Software emulation uses vertical sync feature provided by OpenGL driver.
  This means if your driver vendor provide bad implementation of this functionality image may be corrupted (sometimes in particular parts of the screen)!
- On multi-monitor systems drivers might lock vertical sync to wrong monitor (not for shutter glasses) especially if monitors use different vertical frequency.
  Software emulation will not work at all in this case. Try to disconnect some monitors or to update the drivers.

## <a name="audio">How can I configure multichannel playback?</a>
OpenAL soft supports surround playback.
However it may be disabled by default (especially on Windows Vista+).
You may need to configure OpenAL soft yourself.
In sView installer for Windows option *"OpenAL soft - force 5.1 channel output"* can be used to turn on surround layout.
You need to play multichannel audio in player to these all settings take effect.

## <a name="settings">Where sView stores it's settings?</a>
On the Windows settings are stored in system registry at path `"HKEY_CURRENT_USER\SOFTWARE\sView\"`.
Each subpath represents modules in sView and `"sView"` is reserved for global settings (language, stereo output, etc.).

*macOS* stores application settings in *XML* format at `"~/Library/Preferences/sview/"` directory.

On other systems (e.g. *Linux*) sView uses `libconfig+` library and stores all settings at `"~/.config/sview/"`.

## <a name="hidpi">What is HiDPI / DPI aware?</a>
DPI is a pixels density metric, which describes how many pixels used to construct image on the screen within specified area.
Higher density naturally improves image quality and eliminate discrete artifacts like aliasing.

For a long time traditional displays have not improved this characteristic and made aliasing artifacts
and anti-aliasing options (*SSAA*, *MSAA*, *FXAA*) become well-known even for people far away from technical details.

Nowadays 4K displays with traditional dimensions for desktop monitors (up to 27") break through this long time situation providing almost twice higher pixel density (HiDPI).
However being sticked to low-dpi for a long time, operating systems and applications have not been ready for this change.

In fact, most application developed in the last decade were designed for fixed pixels density in every aspect - icons and bitmaps of fixed size, font sizes, element alignment and so on.
Such applications being launched on HiDPI displays look just too small and unusable.
This problem has been managed by different operating systems in different manner:

- *Mac OS X 17* introduced support for Apple's Retina displays.
  Applications receive virtual resolution of real display (e.g. lower than it actually as!).
- *Windows 8.1* introduced support for HiDPI displays and combination of such displays within LowDPI displays.
- *Linux* has incomplete HiDPI support but some progress has been done in Gnome and Wayland projects.
  For *Xlib* backend *Gnome* supports only single scale factor for all monitors.
