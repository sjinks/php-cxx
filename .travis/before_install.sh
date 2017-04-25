#!/bin/sh

set -e

sudo add-apt-repository -y ppa:ondrej/php
sudo apt-get -qq update
sudo apt-get -qq install valgrind libgtest-dev libphp$PHPVER-embed php$PHPVER-dev
ccache -V
cat ~/.ccache/ccache.conf | grep -q 'sloppiness' || echo "sloppiness = pch_defines,time_macros" >> ~/.ccache/ccache.conf
