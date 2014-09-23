#!/bin/bash

current_dir=$(pwd) > /dev/null 2>&1
pushd $current_dir > /dev/null 2>&1

cd /tmp

echo "Generating test file with random data..."
head -c 40960000 /dev/urandom > rnd

echo "Streaming file through /dev/testdrv to another file..."
dd if=rnd of=/dev/testdrv bs=4096 count=10000 > /dev/null 2>&1 &
dd if=/dev/testdrv of=rnd_ bs=4096 count=10000 > /dev/null 2>&1 &
wait

echo "Comparing content of files..."
file1_sha=$(shasum rnd |awk '{ print $1 }')
file2_sha=$(shasum rnd_ |awk '{ print $1 }')

echo "file1: " $file1_sha
echo "file2: " $file2_sha

rm -f rnd
rm -f rnd_

popd > /dev/null 2>&1

