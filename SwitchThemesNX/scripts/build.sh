#!/bin/sh
set -e

# Remove the object file containing the version string
rm build/Version.o

ver=$(git describe --always --abbrev=40 --dirty)
make all "GITVER=${ver}" $@