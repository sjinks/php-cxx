#!/bin/sh

set -e

sudo add-apt-repository -y ppa:ondrej/php
sudo apt-get -qq update
sudo apt-get -qq install valgrind libgtest-dev libphp$PHPVER-embed php$PHPVER-dev libpcre3-dev
