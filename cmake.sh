#!/bin/bash

cmake . -DDEBUG=YES -DBUILD_CLIENT=YES -DBUILD_ENGINE=YES -DSANITIZE_ADDRESS=YES