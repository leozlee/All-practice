#!/bin/bash
OUT_DIR=/home/leo/nfs/gtest/output
[ -d $OUT_DIR ] && rm -rf $OUT_DIR
mkdir -p $OUT_DIR
gcovr --object-directory=/home/leo/nfs/gtest/samples -o $OUT_DIR/coverage.html -r /home/leo/nfs/gtest/samples --html --html-details -b -s -exclude=*gtest* 


