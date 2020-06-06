#!/bin/sh
echo "Cleaning up old files"
./clean-all.sh
echo "Running autoreconf"
autoreconf --install
if [ "$?" -eq 0 ]; then
   echo "Now run ./configure"
else
   echo "autoreconf failed."
fi
