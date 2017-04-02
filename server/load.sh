#!/bin/bash

IMAGES="server-hs.img.gz kibana-hs.img.gz postgres-hs.img.gz elasticsearch-hs.img.gz"

function load_image() {
  SRC=$1
  echo "Loading $SRC ..."
  sleep 3
  zcat "$SRC" | docker load
}

if [ -z "$1" ]; then
  for img in $IMAGES; do
    load_image $img
  done
else
  save_image $1
fi
