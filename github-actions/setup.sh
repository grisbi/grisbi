#!/bin/bash

if [ "$RUNNER_OS" == "Linux" ]; then
	echo "GNU/Linux"

	sudo apt update
	sudo apt install \
		autoconf \
		automake \
		autopoint \
		build-essential \
		gettext \
		libgoffice-0.10-dev \
		libgsf-1-dev \
		libgtk-3-dev \
		libofx-dev \
		libssl-dev \
		libtool-bin m4
elif [ "$RUNNER_OS" == "Windows" ]; then
	#choco install important_windows_software
	echo "Windows"
elif [ "$RUNNER_OS" == "macOS" ]; then
	echo "macOS"

	# use content of Brewfile file
	brew bundle

	# from brew
	export PKG_CONFIG_PATH=/usr/local/opt/libxml2/lib/pkgconfig:/usr/local/opt/openssl/lib/pkgconfig:/usr/local/opt/libffi/lib/pkgconfig
	export PATH="$PATH:/usr/local/opt/gettext/bin"

	mkdir m4
	ln -sf /usr/local/opt/gettext/share/aclocal/nls.m4 m4
else
	echo "$RUNNER_OS not supported"
	exit 1
fi


