#!/usr/bin/env bash
filename=$0

error() {
	echo "$filename: Error: $1"
	exit 0
}

if [[ ! -f ./berylscript ]]; then
	error 'Project has not been built.' 
fi

if [[ -d ~/bin ]]; then
	cp ./berylscript ~/bin/berylscript
elif [[ -d ~/.local/bin ]]; then
	cp ./berylscript ~/.local/bin/berylscript
else
	echo 'Neither ~/bin or ~/.local/bin are available. Do you wish to install to /usr/local/bin (requires root)? (y/n)'
	read choice
	if [[ $choice == y || $choice == Y || $choice == yes || $choice == Yes ]]; then
		sudo cp ./berylscript /usr/local/bin/berylscript
	else
		echo 'Aborting installation'
		exit 0
	fi
fi

if [[ ! (-d ~/.berylscript) ]]; then # Make the ~/.berylscript dir if it doesn't exist, and set the env variables that point to it
	mkdir ~/.berylscript
	mkdir ~/.berylscript/libs
	cp ./env/init.beryl ~/.berylscript/init.beryl
	
	printf '\n# Added by berylscript installer \n' >> ~/.profile
	printf 'export BERYL_SCRIPT_INIT="$HOME/.berylscript/init.beryl"\n' >> ~/.profile
	printf 'export BERYL_SCRIPT_LIBS="$HOME/.berylscript/libs"\n' >> ~/.profile
fi

cp -r -t ~/.berylscript/libs ./env/libs/* # Copy all the compiled/exported libraries in the env/libs/ directory to the new .berylscript/libs directory
