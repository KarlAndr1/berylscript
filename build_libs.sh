#!/usr/bin/env bash

for dir in libs/*; do
	if [[ -d $dir ]]; then
		cd $dir
		"./build.sh"
		cd ../../
	fi
done
