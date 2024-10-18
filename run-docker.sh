#!/bin/sh

set -e

VERSION=v4

if [ "$#" -ne 1 ]; then
	echo "Usage: ${0} PROJECT_NAME"
	return 1
fi

if [ "$SUDO" = 1 ]; then
	DOCKER="sudo docker"
else
	DOCKER="docker"
fi

project="$1"

mkdir -p projects
mkdir -p local
# $DOCKER run --rm -it --entrypoint /bin/bash \
#	--mount type=bind,source="$(pwd)/transpiler",target=/usr/src/fhe/transpiler \
#	--mount type=bind,source="$(pwd)/transpiler",target=/usr/src/fhe/transpiler \
$DOCKER run --rm -it --entrypoint /usr/src/fhe/compile-frogzone.sh \
	--mount type=bind,source="$(pwd)/local",target=/local \
	--mount type=bind,source="$(pwd)/projects",target=/projects \
	--mount type=bind,source="$(pwd)/compile.sh",target=/usr/src/fhe/compile.sh \
	--mount type=bind,source="$(pwd)/compile-frogzone.sh",target=/usr/src/fhe/compile-frogzone.sh \
	-e project="${project}" \
	"ed255/phantom-zone:$VERSION"

