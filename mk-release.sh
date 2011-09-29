#!/bin/sh
set -e

svn update
URL=`svn info | awk '$1 == "URL:" {print $2}'`
REV=`svn info | awk '$1 == "Revision:" {print $2}'`

TD=`mktemp -d /tmp/XXXXXXXXXXXXX`

( cd $TD 
  svn export $URL
  mv wessels dnspy-$REV
  rm dnspy-$REV/mk-release.sh
  tar czvf /tmp/dnspy-$REV.tar.gz dnspy-$REV
)

rm -rfv $TD
echo "install -m 644 -o $USER /tmp/dnspy-$REV.tar.gz /usr/local/www/dnspy"
install -m 644 -o $USER /tmp/dnspy-$REV.tar.gz /usr/local/www/dnspy
(cd /usr/local/www/dnspy/; md5 *.tar.gz > MD5s.txt)

