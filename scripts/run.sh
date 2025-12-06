#!/bin/bash
#
cd build || exit 1
make || exit 1
cd ../code || exit 1
../build/mydesertcolony_main
