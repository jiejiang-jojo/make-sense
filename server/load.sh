#!/bin/bash

SRC=${1:-server-hs.img.gz}
echo "Going to load $SRC in 3 seconds..."
sleep 3
zcat "$SRC" | docker load
