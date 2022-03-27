#!/bin/bash

if [ ! -d "src" ]; then
    echo "Run script from project root directory"
    exit 1
fi
echo "Cleaning Makefiles..."

rm -rf Makefile
rm -rf src/Makefile

