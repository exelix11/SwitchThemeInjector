#!/bin/sh
set -e

make clean

ver=$(git describe --always --abbrev=40 --dirty)
make all "GITVER=${ver}" $@
