#!/bin/bash
set -e

USAGE=$(cat << EOF
Usage: $(basename "$0") [LIBRARY_NAME ...]

Description:
A script to copy files from '\$FLUTTER_BUILD_DIR/.firebaseSDK/lib' to './lib'.
If no argument is given, it copies all files and folders.

Arguments:
  LIBRARY_NAME   Library name. For exmaple, 'curl' is for 'libcurl.so'.

Example:
  $(basename "$0") firebase_app firebase_database
EOF
)

if [ "$1" = "-h" ]; then
  echo "$USAGE"
  exit 1
fi

FILE_LIST=("$@")
SRC_DIR=${FLUTTER_BUILD_DIR}/.firebaseSDK
DEST_DIR=./lib

SRC_RECORD_FILE="${SRC_DIR}"/VERSION
DEST_RECORD_FILE="${DEST_DIR}"/VERSION
ARCH_LIST=("armel" "i586")
NEED_UPDATE="True"

mkdir -p "$DEST_DIR"

# check if updating libs is needed
if [ -e "$SRC_RECORD_FILE" ] && [ -e "$DEST_RECORD_FILE" ]; then
  if [ $(head -n 1 "$DEST_RECORD_FILE") = \
       $(head -n 1 "$SRC_RECORD_FILE") ]; then
    NEED_UPDATE=""
  fi
fi

if [ "$NEED_UPDATE" = "True" ]; then
  if [ ${#FILE_LIST[@]} -eq 0 ]; then
    # copy all libs if no name is given as arguments
    cp -vr "${SRC_DIR}"/lib/* "${DEST_DIR}"/
  else
    # create arch directories if not exists
    for arch in "${ARCH_LIST[@]}"; do
      [ ! -d "${DEST_DIR}"/${arch} ] && mkdir -vp "${DEST_DIR}"/${arch}
    done

    # copy libs with given names
    for arch in "${ARCH_LIST[@]}"; do
      for file_name in "${FILE_LIST[@]}"; do
        src_file="${SRC_DIR}"/lib/${arch}/lib${file_name}.so
        [ -e "$src_file" ] && cp -v "$src_file" "${DEST_DIR}"/${arch}
      done
    done
  fi

  cp -v "$SRC_RECORD_FILE" "$DEST_RECORD_FILE"
fi
