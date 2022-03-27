#!/bin/bash

if [ ! -d "src" ]; then
    echo "Run script from project root directory"
    exit 1
fi
echo "Cleaning VS projects..."

rm -rf .vs/
rm -rf *.sln
rm -rf src/*.vcxproj*
