#!/bin/sh
set -e

ver=$GITHUB_SHA
make all "GITVER=CI-${ver}" $@