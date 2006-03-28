#!/bin/sh
#
aclocal
automake -a -c # --foreign
autoconf

#./configure $*
echo "Now you are ready to run ./configure"
