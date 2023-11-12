#!/bin/bash

# Generated with ChatGPT - chat.openai.com

declare -A dirs
dirs=( ["build_master"]="Master" ["build_slave_one"]="Slave_One" ["build_slave_two"]="Slave_Two" )

for dir in "${!dirs[@]}"; do
    if [ ! -d "$dir" ]; then
        mkdir "$dir"
    fi
done

for dir in "${!dirs[@]}"; do
    pushd $dir > /dev/null
    cmake -DMODE=${dirs[$dir]} ..
    make
    
    popd > /dev/null
done

echo "Build process completed."
