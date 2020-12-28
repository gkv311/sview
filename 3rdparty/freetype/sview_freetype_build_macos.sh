#!/bin/bash

# small help to build FreeType for macOS [for sView project]
# https://www.freetype.org

aFreeType=freetype-2.10.4

export MACOSX_DEPLOYMENT_TARGET=10.10
rm -r -f ${aFreeType}-macos
mkdir -p ${aFreeType}-macos/lib

function buildArch {
  anArch=$1
  pushd ./${aFreeType}-src
  ./configure CFLAGS="-mmacosx-version-min=10.10 -arch $anArch" LDFLAGS="-Wl,-install_name,@executable_path/../Frameworks/libfreetype.6.dylib" --enable-shared --enable-static
  aResult=$?; if [[ $aResult != 0 ]]; then exit $aResult; fi
  make clean
  make
  aResult=$?; if [[ $aResult != 0 ]]; then exit $aResult; fi

  rm -r -f ../${aFreeType}-macos-$anArch
  mkdir -p ../${aFreeType}-macos-$anArch/lib
  cp -rf ./include      ../${aFreeType}-macos-$anArch/
  cp ./docs/FTL.TXT     ../${aFreeType}-macos-$anArch/
  cp ./docs/GPLv2.TXT   ../${aFreeType}-macos-$anArch/
  cp ./docs/LICENSE.TXT ../${aFreeType}-macos-$anArch/
  cp ./docs/CHANGES     ../${aFreeType}-macos-$anArch/
  cp ./docs/README      ../${aFreeType}-macos-$anArch/

  cp ./objs/.libs/libfreetype.a       ../${aFreeType}-macos-$anArch/lib/
  cp ./objs/.libs/libfreetype.6.dylib ../${aFreeType}-macos-$anArch/lib/
  cp ./objs/.libs/libfreetype.la      ../${aFreeType}-macos-$anArch/lib/
  #cp ./objs/.libs/libfreetype.dylib   ../${aFreeType}-macos-$anArch/lib/
  ln -s libfreetype.6.dylib ../${aFreeType}-macos-$anArch/lib/libfreetype.dylib
  popd
}

for anArchIter in x86_64 arm64
do
  buildArch $anArchIter 
done

cp -rf ./${aFreeType}-src/include      ${aFreeType}-macos/
cp ./${aFreeType}-src/docs/FTL.TXT     ${aFreeType}-macos/
cp ./${aFreeType}-src/docs/GPLv2.TXT   ${aFreeType}-macos/
cp ./${aFreeType}-src/docs/LICENSE.TXT ${aFreeType}-macos/
cp ./${aFreeType}-src/docs/CHANGES     ${aFreeType}-macos/
cp ./${aFreeType}-src/docs/README      ${aFreeType}-macos/
lipo -create -output ${aFreeType}-macos/lib/libfreetype.a       ${aFreeType}-macos-x86_64/lib/libfreetype.a       ${aFreeType}-macos-arm64/lib/libfreetype.a
lipo -create -output ${aFreeType}-macos/lib/libfreetype.6.dylib ${aFreeType}-macos-x86_64/lib/libfreetype.6.dylib ${aFreeType}-macos-arm64/lib/libfreetype.6.dylib
ln -s libfreetype.6.dylib ${aFreeType}-macos/lib/libfreetype.dylib
lipo -info ${aFreeType}-macos/lib/libfreetype.a
lipo -info ${aFreeType}-macos/lib/libfreetype.6.dylib
