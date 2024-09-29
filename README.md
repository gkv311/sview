sView - stereoscopic media player
=================================

[![Downloads](https://img.shields.io/github/downloads/gkv311/sview/total.svg)](https://github.com/gkv311/sview/releases)
[![License: GPL v3](https://img.shields.io/badge/License-GPLv3-blue.svg)](https://github.com/gkv311/sview/blob/master/docs/LICENSE.md)

sView is a cross-platform solution to view 3D stereoscopic videos and images.
Please visit official site for more information:<br/>
https://www.sview.ru

## sView SDK

*sView SDK* - is a set of libraries for development of a stereoscopic application, which sView programs (media player and others) are based on.

* `libStShared` threads, mutexes, template-based signals and slots, OpenGL tools, settings management, and other tools.
* `libStGLWidgets` compact C++ toolkit for writing GUI using OpenGL 2.1+ or OpenGL ES 2.0+.
* `libStCore` window system independent C++ toolkit for writing OpenGL applications.
* `libStOutAnaglyph` stereoscopic output in anaglyph format using GLSL programs.
* `libStOutDistorted` stereoscopic output in anamorph side-by-side format.
* `libStOutDual` stereoscopic output through two dedicated interfaces.
* `libStOutInterlace` stereoscopic output for row interlaced displays using GLSL programs.
* `libStOutIZ3D` stereoscopic output for iZ3D monitors using GLSL programs.
* `libStOutPageFlip` stereoscopic output for shutter glasses devices.

## Updates

To get up-to-date sources please clone official git repository:
~~~~~
  git clone https://github.com/gkv311/sview.git
~~~~~

## Documentation

Read the documentation in the **docs/** directory in git.
Online help is available on official site:<br/>
https://www.sview.ru/en/sview/usertips/

## Licensing

See the [docs/LICENSE](docs/LICENSE.md) file.

## Build and Install

See the [docs/INSTALL](docs/INSTALL.md) file.

## Continuous Integration

Building state of [master branch](https://github.com/gkv311/sview/actions?query=branch%3Amaster) (of this git repository https://github.com/gkv311/sview.git).

| Target platform      | Build Status |
|----------------------|--------------|
| Ubuntu 20.04 (amd64) | ![status](https://github.com/gkv311/sview/actions/workflows/build_ubuntu_20.04.yml/badge.svg?branch=master) |
| Ubuntu 22.04 (amd64) | ![status](https://github.com/gkv311/sview/actions/workflows/build_ubuntu_22.04.yml/badge.svg?branch=master) |
| Ubuntu 24.04 (amd64) | ![status](https://github.com/gkv311/sview/actions/workflows/build_ubuntu_24.04.yml/badge.svg?branch=master) |
| macOS 12 (amd64)     | ![status](https://github.com/gkv311/sview/actions/workflows/build_macos_12.yml/badge.svg?branch=master) |
| Android (armeabi-v7a)| ![status](https://github.com/gkv311/sview/actions/workflows/build_android.yml/badge.svg?branch=master) |
| Windows (amd64)      | [![status](https://ci.appveyor.com/api/projects/status/github/gkv311/sview)](https://ci.appveyor.com/project/gkv311/sview/build/messages) (AppVeyor) |
