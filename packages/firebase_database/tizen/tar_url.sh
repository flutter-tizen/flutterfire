#!/bin/bash
set -e

USAGE=$(cat << EOF
Usage: $(basename "$0") <FILE_URL> [DEST_DIR] [STRIP_COMPONENTS_NUMBER]

Description:
A script to download a tar file from a given URL and extract it. It skips the extraction
processs if the contents of the file have already been extracted for the same URL.

Arguments:
  FILE_URL    The URL address of the tar file to download.
  DEST_DIR    The directory path to the tar file for extraction. (default: \$HOME/.firebaseSDK)
  STRIP_COMPONENTS_NUMBER
              The number of leading components to strip during the tar extraction. (default: 1)

Example:
  $(basename "$0") https://example.com/file.tar.gz /path/to 0
EOF
)

if [ -z "$1" ]; then
  echo "$USAGE"
  exit 1
fi

FILE_URL=$1
DEST_DIR=${2:-${FLUTTER_BUILD_DIR}/.firebaseSDK}
TAR_SNUM=${3:-1}

RECORD_FILE="${DEST_DIR}/VERSION"

if [ -e "$RECORD_FILE" ] && [ "$FILE_URL" = $(head -n 1 "$RECORD_FILE") ]; then
  echo "$RECORD_FILE exists with the same URL."
else
  [ ! -d "$DEST_DIR" ] && mkdir -v "$DEST_DIR"
  curl -L $FILE_URL | tar -xz --strip-components=$TAR_SNUM -C "$DEST_DIR"
  echo $FILE_URL > "$RECORD_FILE"
fi
