#!/bin/bash

# save active directory
pushd .

#sudo apt-get install build-essential devscripts

# go to the script directory
aScriptPath=${BASH_SOURCE%/*}
if [ -d "$aScriptPath" ]; then
    cd "$aScriptPath"
fi

YEAR=$(date +"%y")
MONTH=$(date +"%m")
DAY=$(date +"%d")
aDebVersion=${YEAR}.${MONTH}-1
aDebSrcRoot=temp/sview-${aDebVersion}
aCurrentDate=`date --rfc-2822`

# Ubuntu 14.04 LTS (Trusty Tahr)
# Ubuntu 15.04     (Vivid Vervet)
# Ubuntu 15.10     (Wily Werewolf)
# Ubuntu 16.04 LTS (Xenial Xerus)
# Ubuntu 16.10     (Yakkety Yak)
# Ubuntu 17.04     (Zesty Zapus)
# Ubuntu 17.10     (Artful Aardvark)
# Ubuntu 18.04 LTS (Bionic Beaver)
# Ubuntu 18.10     (Cosmic Cuttlefish)
# Ubuntu 19.04
aDistribs=("xenial" "zesty" "artful" "bionic" "cosmic")

# Debian
#aDistribs=("stable" "unstable" "testing-proposed-updates" "experimental")

# remove temporary files
rm -f temp/*.build
rm -f temp/*.buildinfo
rm -f temp/*.changes
rm -f temp/*.dsc
rm -f temp/*.tar.gz
rm -f temp/*.tar.xz

cd ${aDebSrcRoot}

for aDistr in "${aDistribs[@]}"
do
  echo "Prepare source package for '$aDistr'"
  sed "s/unknown_distrib/${aDistr}/g"                  "debian/changelog.tmpl" > "debian/changelogtmp1"
  sed "s/unknown_version/1:${aDebVersion}~${aDistr}/g" "debian/changelogtmp1"  > "debian/changelogtmp2"
  sed "s/unknown_date/${aCurrentDate}/g"               "debian/changelogtmp2"  > "debian/changelog"
  rm -f debian/changelogtmp*

  mkdir -p ../$aDistr
  debuild -S
  mv -f ../*.build     ../$aDistr
  mv -f ../*.buildinfo ../$aDistr
  mv -f ../*.changes   ../$aDistr
  mv -f ../*.dsc       ../$aDistr
  mv -f ../*.tar.*z    ../$aDistr
  #mv -f ../*.tar.gz    ../$aDistr
  #mv -f ../*.tar.xz    ../$aDistr
done

#for aDistr in "${aDistribs[@]}"
#do
#  dput ppa:sview/stable ../$aDistr/sview_${aDebVersion}~${aDistr}_source.changes
#done

# turn back to original directory
popd
