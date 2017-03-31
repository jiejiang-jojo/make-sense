#!/bin/bash

IMAGE=${1:-server-hs}
SRC=${2:-server-hs.img.gz}
echo "Going to load $SRC as $IMAGE in 3 seconds..."
sleep 3
zcat "$SRC" | docker load "$IMAGE"
