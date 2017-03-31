#!/bin/bash

IMAGE=${1:-server-hs}
DEST=${2:-server-hs.img.gz}
docker save "$IMAGE" | gzip > "$DEST"
