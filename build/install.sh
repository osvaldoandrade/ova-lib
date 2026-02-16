#!/bin/bash
echo 'Installing ova_lib...'
mkdir -p /usr/local/lib
mkdir -p /usr/local/include/ova_lib
cp -r /home/runner/work/ova-lib/ova-lib/build/lib/* /usr/local/lib/
cp -r /home/runner/work/ova-lib/ova-lib/build/include/* /usr/local/include/ova_lib
echo 'Installation complete!'
