## I. Installing 3rd-parties

sView requires several 3rd-party components for building:
* C/C++ compiler (g++, MSVC 2010+)
* Code::Blocks (https://www.codeblocks.org/)
* FFmpeg (https://www.ffmpeg.org)
* OpenAL soft (https://openal-soft.org/)
* libwebp, optional (https://developers.google.com/speed/webp/download)
* GTK2+, Linux only (https://www.gtk.org)
* libconfig++, Linux and Android (https://www.hyperrealm.com/libconfig/libconfig.html)
* libxpm, Linux only

On Debian/Ubuntu you might use the following command to install all dependencies at once:

~~~~~
sudo apt-get install \
  g++ \
  libgtk2.0-dev \
  libopenal-dev \
  libgl1-mesa-dev \
  libavcodec-dev libavdevice-dev libavformat-dev libavutil-dev libswscale-dev \
  libwebp-dev \
  libconfig++-dev libconfig-dev \
  libxpm-dev \
  codeblocks
~~~~~
 
The similar command for RPM-based distributives:

~~~~~
yum install gcc gcc-c++ \
  gtk+-devel gtk2-devel \
  mesa-libGLU-devel glew-devel \
  openal-devel \
  libconfig-devel

rpm -Uvh http://download1.rpmfusion.org/free/fedora/rpmfusion-free-release-stable.noarch.rpm
yum install ffmpeg-devel
~~~~~

ALT Linux:
~~~~~
apt-get install \
  gcc8-c++ gcc-c++ libGLU-devel libgtk+2-devel libXrandr-devel libfreetype-devel \
  libopenal-devel libconfig-devel libconfig-c++-devel libXpm-devel libwebp-devel \
  libavcodec-devel libavdevice-devel libavformat-devel libavutil-devel libswscale-devel
~~~~~

On Windows and macOS please refer to official documentation for each project.
Notice that DevIL and FreeImage libraries are optional and are not required for building sView
(libraries are loaded dynamically if available).

## II. Makefile (Linux, macOS, Android)

sView project defines a single `Makefile` for building an application and all libraries.
There are no configuration scripts (like `./configure`), although building options can be adjusted by passing arguments to `make` or by modification of `Makefile` file.

When building sView on Debian-based distributive - all you need is to install dependencies via package manager and to execute traditional commands:

~~~~~
  make && make install
~~~~~

### Android

sView for Android platform is built in the same way as for Linux platforms - via UNIX `Makefile`.
sView does not provide `.mk` files - it uses UNIX Makefile for building native code.

`Makefile` coming with sView performs all building steps - from building native `.so` libraries, to compiling `.java` source code and generation of APK package.
Paths to NDK and 3rd-party libraries can be specified through command-line options to **make** or by editing `Makefile` itself.
The building can be run on a Linux host with compatible Java SDK, Android NDK and Android SDK versions.

### Qt Creator

*distribution/qmake/sView.pro* defines a project file for Qt Creator.
This is not a self-sustained solution, but rather a wrapper over existing UNIX `Makefile`, allowing to develop and build sView for macOS, Linux and Android targets within IDE.
It has been introduced as an alternative to Code::Blocks having severe stability and usability problem on many platforms.

## III. Code::Blocks

For historical reasons, sView building environment was initially defined by a Code::Blocks (https://www.codeblocks.org/) workspace, allowing to develop and build application on multiple platforms.
Code::Blocks remains the main way for building sView on Windows platform, although development can be done using Visual Studio (see below).

There are several building targets depending on platform
(Mac OS X, Linux, Windows) and debugging possibilities:
* `WIN_vc_x86`,        32-bit target using Visual Studio compiler
* `WIN_vc_AMD64_DEBUG`,64-bit target with debugging options
* `WIN_vc_AMD64`,      64-bit target using Visual Studio compiler
* `LINUX_gcc`,         Linux target, g++ compiler
* `LINUX_gcc_DEBUG`,   Linux target with debugging options
* `MAC_gcc`,           Mac OS X target, g++ compatible compiler
* `MAC_gcc_DEBUG`,     Mac OS X target with debugging options

Notice that the following compilers should be configured within Code::Blocks:
* `gcc`,               configured to g++ or compatible compiler (on systems other than Windows)
* `msvc10`,            configured to Visual Studio 2010+ compiler, PSDK and DXSDK
* `windows_sdk_x86_64` (copy of msvc10) configured to 64-bit libraries and compiler toolchain

3rd-parties should be either configured as Code::Blocks global compiler options or placed into "3rdparty" folder.

## IV. Visual Studio

sView comes within Visual Studio solution file, compatible with Visual Studio 2015 and higher.
Visual Studio solution might require some extra steps for setting up paths to external libraries and for copying sView resources.

Notice that for historical and compatibility reasons, sView install packages for Windows are build using Code::Blocks, configured to Visual Studio 2010 compiler.

## V. Building options

Several preprocessor directives control building options.
Notice that by default "include/stconfig.conf" file is used to override these options
(this file will be used only when ST_HAVE_STCONFIG is defined).

* `ST_HAVE_WEBP` - should be defined to activate libwebp usage
  (notice that since next releases of FFmpeg might have built-in support for webp/webpll image files)
* `ST_HAVE_MONGOOSE` - should be defined to activate built-in web UI for remote Movie Player control
* `ST_DEBUG` - should be defined to activate debugging log output
* `ST_DEBUG_LOG_TO_FILE` - specifies file name or full path to duplicate debug log output

## VI. Distribution scripts

Several script were written to automate distribution routines.
All them were placed in "distribution" folder.

* `build.bat`, batch script for Windows. Performs re-building of **WIN_vc_x86** and **WIN_vc_AMD64** targets
  using Code::Blocks and pack result binaries using InnoSetup script.
  Notice that all dependencies should be available (including InnoSetup),
  and DLLs should be placed into **bin/WIN_vc_x86** and **bin/WIN_vc_AMD64** folders.
* `buildDebSrc.sh`, bash script for Linux to pack sources into Debian source package.
* `buildMac.sh`, bash script to pack binaries from **bin/MAC_gcc** folder into DMG image.
  3rd-party libraries should be already located in "bin/MAC_gcc_DEBUG/sView.app/Contents/Frameworks/"
  folder with correct search path (refer to `bind_frameworks.sh` auxiliary script).

~~~~~
su
yum install rpm-build

mkdir -p ${HOME}/workspace/redhat/{RPMS,SRPMS,SPECS,SOURCES,BUILD}
echo "%_topdir ${HOME}/workspace/redhat" > ${HOME}/.rpmmacros

wget https://launchpad.net/~sview/+archive/stable/+files/sview_12.05-1%7Eprecise.tar.gz
mv sview_12.05-1~precise.tar.gz $HOME/workspace/redhat/SOURCES/sview_12.05-1.tar.gz
rpmbuild -ba distribution/sView.rpm.spec
~~~~~
