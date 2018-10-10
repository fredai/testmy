#!/bin/bash
source ./VERSION
cp -r release/ex-alarm .
tar zcvf ${V}.tar.gz ex-alarm
rm -rf ex-alarm
