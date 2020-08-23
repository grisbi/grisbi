#!/bin/bash

#set -e

OUT=/tmp/grisbi_icons.txt
OUT2=$OUT.2

echo "Start grisbi from ~/gtk/inst/bin/grisbi"
echo "Press Ctrl-C when finished"

sudo opensnoop -n grisbi > $OUT

echo
grep inst/share/icons/Adwaita $OUT | sed "s/.*inst\///" | sed "s/ $//" | sort -u > $OUT2

grep .png $OUT2
grep .svg $OUT2
