#!/bin/bash

IMAGE=${1:-server-hs}
DEST=${2:-server-hs.img.gz}
echo 'Going to save $IMAGE to $DEST in 3 seconds...'
sleep 3
docker save "$IMAGE" | gzip > "$DEST"
