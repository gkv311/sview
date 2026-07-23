Stereoscopic Formats
====================

Stereoscopic pair consist of two slightly different images of the same size captured from for left and right eyes (e.g. from two camera positions).
This pair can be stored in various ways in the file:

- *Mono* - no stereoscopic information, only single view is provided;
- *Dual Stream* - each view is stored in independent stream or file;
- *Cross-eyed*, *Parallel pair* - two images packed horizontally, also known as Side-by-Side;
- *Over/Under* - two images packed vertically, also known as Top-Bottom;
- (Obsolete) *Interlaced* - row interleaved;
- (Obsolete) *Anaglyph* - image stored for glasses with color filters;
- (Obsolete) *Frame-sequential* - views (frames) interleaved in time (first frame Left view, second frame Right view and so on);
- (Obsolete) 2x720p in 1080p tiled - special layout used by some TV broadcasters in Europe some time ago.

Program will be able to show image properly only when stereoscopic format has been properly selected.
sView is able to determine stereoscopic format using:

- Multiple streams (`*.mpo`, `*.mkv`, `*.wmv`).
  Two image / video streams of the same size are automatically detected as stereoscopic pair with Left view in the first stream.
- *JPEG* image containing `JPS` marker ([VRex extension](https://paulbourke.net/stereoscopy/stereoimage/)).
  *JPS* marker defines stereoscopic format.
- *PNG* image containing `sTER` chunk ([Extensions to the PNG](http://www.libpng.org/pub/png/spec/register/pngext-1.4.0-pdg.html)).
  Only parallel pair with views order can be defined by `sTER` indicator.
- *WMV* metadata (`*.wmv`).
  The following metadata fields will be interpreted by sView:
  - `StereoscopicLayout` defining one of layouts (`SideBySideRF`, `SideBySideLF`, `OverUnderLT`, `OverUnderRT`)
  - `StereoscopicHalfHeight` and `StereoscopicHalfWidth` defining anamorphic video
  - `StereoscopicHorizontalParallax` defining parallax in pixels
- *MKV* metadata (`*.mkv`, `*.mkv3d`).
  [Matroska specification](https://www.matroska.org/technical/specs/index.html) includes dedicated fields defining stereoscopic layout (`StereoMode`).
  FFmpeg library converts this data in form of stream metadata with name `STEREO_MODE`
  (with values `mono`, `right_left`, `left_right`, `bottom_top`, `top_bottom`,
   `row_interleaved_rl`, `row_interleaved_lr`, `block_lr`, `block_rl`, `anaglyph_cyan_red`, `anaglyph_green_magenta`).
  Note that detection code in sView is generalized and specified metadata tags will be read from any video file (not only *.mkv).
- *h264 SEI* messages (per frame side data, `*.mp4`, `*.mkv`).
  Some codecs stores stereoscopic identification data at every frame.
  This technically allows to switch from mono to stereo3d within the same stream.
  *FFmpeg* provides this information in form of `AVStereo3D` structure - thus sView will be able to read this information
  for all decoders in FFmpeg supporting this feature.
- File extension (`*.pns`, `*.jps`).
  Image files with extensions `*.pns` (*PNG* file) and `*.jps` (*JPEG* file) will be interpreted
  as stereoscopic pair in Side-by-side format with Right view first (cross-eyed).
- File name. sView supports the following name convention for stereoscopic format identification:
  - `half-ou`, `-hou`, `-abq` define anamorphic Over/Under pair
  - `half-sbs`, `-hsbs`, `-lrq`, `-rlq` define anamorphic Side-by-side pair
  - `-ba`, `-ab` define Over/Under pair
  - `-sbs`, `-lr`, `-rl` define Side-by-side pair

*Anamorphic* stereo pair with Side-by-side and Over/Under layout is a special format introduced
for compatibility with build-in TV players and existing hardware players without *HDMI 1.4a+* support.
Video stored in this format has broken pixel proportions - it identifies itself as normal *16:9* video with *1080p HD* resolution (*1920x1080*).
Both are important - old hardware players have been unable to decode videos of greater side, and *16:9* proportions
allow transferring video image through HDMI without extra scaling or cropping which should be normally applied.
So in general this format has been created as a temporary hack entrenched for much longer time than expected...

<br/>

Normally files should contain appropriate stereoscopic metadata to be properly displayed by players without user involvement.
Unfortunately, many files still created without this information requiring manual configuration by user.

If your file has been opened wrong you should try to change "source stereo format".
In most cases this is just layout which defines how left and right views are stored in *single* frame.
At first step you should select "Mono" format to see the original frame.

<table cellpadding='1' cellspacing='1' width='100%'>
<tr>
<td width='20%'><a href='/docs/images/boxSideBySide.png'><img src='/docs/images/boxSideBySide.png' width='125' height='164' border='0' /></a></td>
<td>
<h2 class='indent'>Side-by-side</h2>
<p>Left-eye and right-eye views are stored as a whole horizontally.
Since views order is not standardized you should try both "Cross-eyed" (Right view at left side, Left view at right side) and "Parallel Pair".
If image looks correctly but you experience discomfort - you have probably chosen wrong views order.
Notice that images with extensions "JPS" (JPEG stereo) and "PNS" (PNG stereo) are expected in "Cross-eyed" order whilst movies and other images might be stored in any order.</p>
</td>
</tr>

<tr>
<td width='20%'><a href='/docs/images/boxOverUnderBig.png'><img src='/docs/images/boxOverUnder.png' width='125' height='164' border='0' /></a></td>
<td>
<h2 class='indent'>Over/Under</h2>
<p>Left-eye and right-eye views are stored as wholes and placed vertically.
If image looks correctly but you experience discomfort - you have probably chosen wrong views order.
This format is commonly used in videos.</p>
</td>
</tr>

<tr>
<td width='20%'><a href='/docs/images/boxInterlacedBig.png'><img src='/docs/images/boxInterlaced.png' width='125' height='164' border='0' /></a></td>
<td>
<h2 class='indent'>Interlaced</h2>
<p>In this format left and right views are interlaced line by line: each line correspond to each view vertically <i>in turn</i>.
In expressive stereo-volume, this picture in the source (means played not in stereo-viewer) seen like with "noise".</p>
</td>
</tr>

<tr>
<td width='20%'><a href='/docs/images/boxAnaglyphBig.png'><img src='/docs/images/boxAnaglyph.png' width='125' height='164' border='0' /></a></td>
<td>
<h2 class='indent'>Anaglyph</h2>
<p>This format comes specially for viewing devices - color glasses.
Views are stored in MIX (meaning each pixel stores both views), separation provided by color filters in glasses (most used and most old - red-blue filters).
This format is easy-to-recognize by color noise. Unfortunately, materials stored in this format are unusable by most devices (not color-glasses) due to serious color losses.</p>
</td>
</tr>

</table>
