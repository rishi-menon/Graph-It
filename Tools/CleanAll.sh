#!/bin/bash

if [ ! -d "src" ]; then
    echo "Run script from project root directory"
    exit 1
fi

Tools/CleanBuild.sh
Tools/CleanMake.sh
Tools/CleanVS.sh