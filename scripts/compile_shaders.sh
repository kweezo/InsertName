#!/bin/bash

for filename in client_data/shaders/src/*; do
    glslc $filename -o client_data/shaders/bin/$(basename $filename).spv
done