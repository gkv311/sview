sView - User Manual
===================

sView combines functionality of Media Player and Image Viewer.
Being designed for 3D stereoscopic playback, it provides many features unusual to traditional players and viewers.
This short manual describes key features.
If you have questions you might find answers in [sView FAQ](faq.md).

## <a name="mouse">Mouse actions</a>
Many actions can be performed with the mouse. Besides main menu and tool-bar icons, there are some more useful tricks:

- File **Drag & Drop**. You can drag file from your file explorer and move it onto the opened sView window, and file will be opened immediately.
  Make sure you move image to Image Viewer and video file to Movie Player since them have own list of supported formats!
- **Zooming**. You can scale the image using scrolling wheel on your mouse (or scrolling gestures on your touch-pad). Cursor position defines center of zoom.
- **Panning**. Hold left mouse button, move image where you want, release the button.
- **Fullscreen / Windowed**. Press middle button on your mouse. This action can be also performed with keyboard keys 'Enter' and 'F'.

## <a name="short_keys">View short-keys</a>
All hotkeys can be re-assigned within application settings.
Following settings are defined individually for each file in playlist:

- **Backspace** - reset view settings (**Delete** on Macbook).
- **W** - swap Left/Right (will be applied during export!).
- **F** or **Enter** - switch Fullscreen/Windowed.
- **P** - turn on/off panorama image view.
- **+** or **Numpad +** - zoom in.
- **-** or **Numpad -** - zoom out.
- **Left** - walk left (Image Viewer), seek backward (Movie Player).
- **Right** - walk right (Image Viewer), seek forward (Movie Player).
- **Up** - walk up (only Image Viewer).
- **Down** - walk down (only Image Viewer).
- **[** - rotate image 90&deg; *counterclockwise*.
- **]** - rotate image 90&deg; *clockwise*.
- **Ctrl** + **[** - rotate image *counterclockwise* (smooth).
- **Ctrl** + **]** - rotate image *clockwise* (smooth).

## <a name="file_navigation">File navigation keys</a>
sView does not function like media-library automatically filled in from entire filesystem.
Instead, sView provides playlist implicitly filled in with active folder content - the folder containing opened file
(or folder content within sub-folders when opening folder).
It is also possible to open existing .m3u playlist.

The following keys can be used for fast navigation (in addition to buttons on toolbars):

- **Esc** - close the program.
- **Ctrl** + **O** - open new file.
- **Ctrl** + **S** - save snapshot.
- **Shift** + **delete** - physically delete current file without recycle bin (**Fn** + **Shift** + **Delete** on Macbook).
- **Home** - open first item in playlist (or within current folder).
- **Page Up** - open previous item in playlist (or within current folder).
- **Page Down** - open next item in playlist (or within current folder).
- **End** - open last item in playlist (or within current folder).
- **Space** - start/stop slideshow in Image Viewer, play/pause in Movie Player.
- **T** - select next subTitles stream (or **U**).
- **L** - select next audio (Language) stream (or **H**).
- **Ctrl** + **L** - show/hide playlist in Movie Player.

## <a name="img_keys">Image adjustment keys</a>
Following settings will be applied to all files:

- **Shift** + **G** - increase Gamma correction coefficient.
- **Ctrl** + **G** - decrease Gamma correction coefficient.
- **Shift** + **B** - increase Brightness.
- **Ctrl** + **B** - decrease Brightness.
- **Shift** + **T** - increase Saturation (colorify).
- **Ctrl** + **T** - decrease Saturation (grayscale).

## Source stereo format short-keys
Source stereo format is applied to all opened files.
However due to playback buffering within Movie Player the change will take effect only 16 frames later (e.g. no change in paused state).
Most popular formats can be activated by hotkeys:

- **A** - use source-format information stored in the file itself (default).
- **M** - Mono.
- **S** - Side By Side.
- **O** - Over/Under.

[This article](stereoformats.md) contains description of stereoscopic formats.

## <a name="sep_keys">Separation adjustment keys</a>
Some defects of stereoscopic pair can be corrected within sView:

- **Numpad /** and __Numpad \*__ - change horizontal separation to adjust zero-parallax point.
- **Ctrl** + **Numpad /** and **Ctrl** + __Numpad \*__ - change vertical separation to fix vertical alignment issues.
- **Ctrl** + **;** - rotate *left* view *clockwise*, *right* view *counterclockwise*.
- **Ctrl** + **'** - rotate *left* view *counterclockwise*, *right* view *clockwise*.

## <a name="aspect_ratio">Aspect ratio</a>
Aspect ratio defines image proportions between width and height.
There are to quantities associated to each other - Pixel Aspect Ratio (PAR) and Display Aspect Ratio (DAR).
Display aspect ratio is more familiar to the users,
while Pixel Aspect Ratio is more convenient because it is preserved within most image manipulations like cropping and rotation.

The non-square Pixel Aspect Ratio has been popular in era of DVDs,
but is quite rare thing nowadays since 720p and 1080p with 16/9 Display Aspect Ratio became a new standard.
Usually there is no need to care about this option, because all necessary information should be stored in the file.

Unfortunately, there is one important exception - anamorphic 1080p stereo-pair (either horizontal or vertical).
This format has been originated from limitations of hardware existed before 3D stereo has been standardized,
when FullHD (1920x1080) standard has been already available for a long time.

Most 3D TVs have function to convert vertical or horizontal pair into stereo,
but being connected to old players the maximum resolution is 1920x1080.
And this is exactly the reason why anamorphic 1080p format became so popular.
It does not require more powerful hardware player able to play 2x1080p video
and supporting new HDMI extensions for stereoscopic image.

The problem, actually, occurs with players respecting aspect ratio information stored in the file.
Most software encoding anamorphic videos stores incorrect information causing sView to display them distorted.

Display ratio can be overridden from menu in sView to workaround such issues.

## <a name="fps">Playback smoothness</a>
sView provides several advanced options affecting playback smoothness (fps) and power consumption:

- **VSync** or Vertical Sync - general way to eliminate tearing artifacts and to limit upper bound of FPS to the maximal vertical refresh rate of used display.
- **Reduce CPU usage** - limit FPS to some sensible value. In case of video playback the limit is set to 2x of average frame rate. Works independent from VSync.

VSync option naturally reduces GPU load to sane limits (e.g. there is no much sense rendering more frames than connected monitor or projector is able to display).
However this option does not necessarily reduces CPU load - depending on driver CPU load might even increase.
To solve the problem, the second option activates extra mechanisms reducing overall resources utilized by sView.

Both options are activated by default to reduce system load and allow other applications using more resources.
However options might impact playback smoothness, so you can disable them to resolve performance issues.

sView also provides option to **keep Display turned on** during playback.
By default, program blocks sleeping during Video playback - but not within Audio playback nor Video in paused state.
You may disable this feature at all or turn on more aggressive behavior in settings.

## <a name="advanced">3rd party libraries</a>
sView uses 3rd-party components. Most of them are dynamically linked and licensed under LGPL.

_**OpenAL**_. sView uses OpenAL soft as handy cross-platform interface for audio playback.
On *Windows* you may replace OpenAL soft implementation with the library provided by Audio card vendor (from ASUS or Creative).
Just remove (or rename/backup) *"OpenAL.dll"* library within sView installation folder.

sView might also use non-default audio playback device, which can be changed in settings.

_**Multichannel playback**_. OpenAL soft supports multichannel surround playback.
However it may be disabled by default (like on *Windows Vista+*).
You may need to configure OpenAL soft yourself in this case.
sView Windows installer option *"OpenAL soft - force 5.1 channel output"* is an easy way to turn on most-popular surround layout.
Af course this setting will take effect only while playing multichannel audio streams.

_**FFmpeg**_. sView uses framework FFmpeg for cross-platform audio/video decoding and encoding.
FFmpeg involves steadily and you might be interested in new codecs supported by latest update.
You should take care that only update without ABI changes can be used as transparent replacement for installed sView.
Otherwise sView should be re-built.
