#!/usr/bin/env bash

for file in test_scripts/*; do
	echo "Test: $file"
	./berylscript $file
done
