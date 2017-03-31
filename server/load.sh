#!/bin/bash

SRC=${1:-server-hs.img.gz}
IMAGE=${2:-server-hs}
echo "Going to load $SRC as $IMAGE in 3 seconds..."
sleep 3
zcat "$SRC" | docker load "$IMAGE"
