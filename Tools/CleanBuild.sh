#!/bin/bash

if [ ! -d "src" ]; then
    echo "Run script from project root directory"
    exit 1
fi
echo "Cleaning Build Files..."

rm -rf bin/
rm -rf bin-int/
