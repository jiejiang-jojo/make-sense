#!/bin/bash

IMAGE=${1:-server-hs}
SRC=${2:-server-hs.img.gz}
zcat "$SRC" | docker load "$IMAGE"
