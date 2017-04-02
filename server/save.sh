#!/bin/bash

IMAGES="server-hs kibana-hs postgres-hs elasticsearch-hs"

function save_image() {
  IMAGE=$1
  DEST=$IMAGE.img.gz
  echo "Saving $IMAGE to $DEST ..."
  docker save "$IMAGE" | gzip > "$DEST"
}

if [ -z "$1" ]; then
  for img in $IMAGES; do
    save_image $img
  done
else
  save_image $1
fi
