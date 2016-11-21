FROM debian:testing
MAINTAINER Ludovic Rousseau

RUN apt-get -y update \
	&& apt-get install -y \
		autoconf \
		automake \
		build-essential \
		intltool \
		libgoffice-0.10-dev \
		libgsf-1-dev \
		libgtk-3-dev \
		libofx-dev \
		libssl-dev \
		libtool-bin \
		m4 \
	&& rm -rf /var/lib/apt/lists/*

RUN mkdir /build
WORKDIR /build
