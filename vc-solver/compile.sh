#!/bin/bash

RED='\033[0;31m'
GREEN='\033[0;32m'
NC='\033[0m'

function usage_info {
  echo "${RED}"Invalid command."${NC}"
  echo Use one of the following commands instead:
  echo bash compile.sh --release
  echo bash compile.sh -r
  echo bash compile.sh --debug
  echo bash compile.sh -d
  echo bash compile.sh --profile
  echo bash compile.sh -p
  exit 0
}

function build_failed {
  echo -e "${RED}"BUILD FAILED - No executable was created."${NC}"
  exit 1
}

if [[ $# -eq 0 ]] ; then
    usage_info
fi

if [ "$1" = "--debug" ] || [ "$1" = "-d" ];
then
  LOCATION_NAME="debug"
  OPERATION_NAME="Debug"
elif [ "$1" = "--release" ] || [ "$1" = "-r" ];
then
  LOCATION_NAME="release"
  OPERATION_NAME="Release"
elif [ "$1" = "--profile" ] || [ "$1" = "-p" ];
then
  LOCATION_NAME="profile"
  OPERATION_NAME="RelWithDebInfo"
else
  usage_info
fi

mkdir -p build

if ! cd build;
then
  build_failed
fi

mkdir -p "${LOCATION_NAME}"

if ! cd "${LOCATION_NAME}";
then
  build_failed
fi

if ! cmake -DCMAKE_BUILD_TYPE="${OPERATION_NAME}" ../..;
then
  build_failed
fi

if cmake --build .;
then
    echo -e "${GREEN}"BUILD SUCCESSFUL."${NC}"
    echo -e The solver can be run by typing: ./build/"${LOCATION_NAME}"/vc
else
    build_failed
fi
