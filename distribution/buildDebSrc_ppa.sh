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

# Ubuntu  8.04 LTS (Hardy Heron)
# Ubuntu 10.04 LTS (Lucid Lynx)
# Ubuntu 10.10     (Maverick Meerkat)
# Ubuntu 11.04     (Natty Narwhal)
# Ubuntu 11.10     (Oneiric Ocelot)
# Ubuntu 12.04 LTS (Precise Pangolin)
# Ubuntu 12.10     (Quantal Quetzal)
# Ubuntu 13.04     (Raring Ringtail)
# Ubuntu 13.10     (Saucy Salamander)
#aDistribs=("lucid" "natty" "oneiric" "precise" "quantal" "raring")
aDistribs=("lucid" "precise" "quantal" "raring")

# Debian
#aDistribs=("stable" "unstable" "testing-proposed-updates" "experimental")

# remove temporary files
rm -f temp/*.build
rm -f temp/*.changes
rm -f temp/*.dsc
rm -f temp/*.tar.gz

cd ${aDebSrcRoot}

for aDistr in "${aDistribs[@]}"
do
  echo "Prepare source package for '$aDistr'"
  sed "s/unknown_distrib/${aDistr}/g"                "debian/changelog.tmpl" > "debian/changelogtmp1"
  sed "s/unknown_version/${aDebVersion}~${aDistr}/g" "debian/changelogtmp1"  > "debian/changelogtmp2"
  sed "s/unknown_date/${aCurrentDate}/g"             "debian/changelogtmp2"  > "debian/changelog"
  rm -f debian/changelogtmp*

  mkdir -p ../$aDistr
  debuild -S
  mv -f ../*.build   ../$aDistr
  mv -f ../*.changes ../$aDistr
  mv -f ../*.dsc     ../$aDistr
  mv -f ../*.tar.gz  ../$aDistr
done

#for aDistr in "${aDistribs[@]}"
#do
#  dput ppa:sview/stable ../$aDistr/sview_${aDebVersion}~${aDistr}_source.changes
#done

# turn back to original directory
popd
