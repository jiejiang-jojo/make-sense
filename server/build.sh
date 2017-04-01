#!/bin/bash

IMAGE=$1

IMAGES="server-hs kibana-hs elasticsearch-hs postgres-hs"

function build(){
  image=$1
  case $image in
    server-hs)
      echo "Building server-hs ..."
      docker build -f Dockerfile.server-hs -t server-hs .
      ;;
    kibana-hs)
      echo "Building kibana-hs ..."
      docker build -f Dockerfile.kibana-hs -t kibana-hs .
      ;;
    elasticsearch-hs)
      echo "Building elasticsearch-hs ..."
      docker build -f Dockerfile.elasticsearch-hs -t elasticsearch-hs .
      ;;
    postgres-hs)
      echo "Building postgres-hs ..."
      docker build -f Dockerfile.postgres-hs -t postgres-hs .
      ;;
    *)
      echo "Image $image is not one of"
      echo "$IMAGES"
  esac
}

if [ -z "$IMAGE" ]; then
  for img in $IMAGES; do
    build $img
  done
else
  build $IMAGE
fi
