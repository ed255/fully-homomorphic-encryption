#!/bin/sh

set -ex

VERSION=v2

sudo docker build --no-cache --file docker/phantom.Dockerfile . -t phantom-zone:$VERSION
commit=$(sudo docker run --rm "phantom-zone:$VERSION" git log -1 --format="%h")
sudo docker tag phantom-zone:$VERSION ed255/phantom-zone:$VERSION
sudo docker tag phantom-zone:$VERSION ed255/phantom-zone:$commit
sudo docker push ed255/phantom-zone:$VERSION
sudo docker push ed255/phantom-zone:$commit
