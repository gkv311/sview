#!/bin/bash

# This script will create/update DEBian repository for sView project.
#
# Copyright © Kirill Gavrilov, 2010-2011

function genPackages {
  aSuit=$1
  anArch=$2
  aDir=dists/$aSuit/non-free/binary-$anArch
  mkdir -p $aDir

  apt-ftparchive packages pool/$aSuit/non-free-$anArch > $aDir/Packages
  gzip  -9c <$aDir/Packages >$aDir/Packages.gz
  bzip2 -9c <$aDir/Packages >$aDir/Packages.bz2
}

function genRelease {
  aSuit=$1
  anArch=$2
  aDir=dists/$aSuit/non-free/binary-$anArch

  cat > $aDir/Release << EOF
Archive:       $aSuit
Suite:         $aSuit
Component:     non-free
Origin:        sview.ru
Label:         sview.ru
Architecture:  $anArch
EOF

  apt-ftparchive release $aDir >> $aDir/Release
}

function genSuit {
  aSuit=$1
  aUserName=$2
  aPassphrase=$3

  # generate files for i386
  genPackages $aSuit i386
  genRelease  $aSuit i386

  # generate files for amd64
  genPackages $aSuit amd64
  genRelease  $aSuit amd64

  # generate common 'Release' file
  aDir=dists/$aSuit

  cat > $aDir/Release << EOF
Origin:        sview.ru
Label:         sview.ru
Suite:         $aSuit
Codename:      unknown
Architectures: i386 amd64
Components:    non-free
Description:   sview.ru official repository
EOF

  apt-ftparchive release $aDir >> $aDir/Release
  # sign the Release file
  echo $aPassphrase | gpg -abs --local-user "$aUserName" --yes --passphrase-fd 0 -o $aDir/Release.gpg $aDir/Release
}

# go to the script directory
scriptPath=${BASH_SOURCE%/*}
cd "$scriptPath"

# prepare root directory
mkdir -p repository/deb
cd repository/deb

# use Zenity to ask the passkey
aUserName="$USERNAME"
aPassphrase=$(zenity --entry --title="Password for key" --text="Enter your password:" --entry-text "" --hide-text)

genSuit stable       $aUserName $aPassphrase
genSuit experimental $aUserName $aPassphrase

# turn back...
cd ../..

# export the public key
gpg --export -a "$aUserName" > repository/deb/apt.key
