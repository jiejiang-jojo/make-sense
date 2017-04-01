#!/bin/bash

IMAGE=${1:-server-hs}

case $IMAGE in
  server-hs)
    echo "Building server-hs ..."
    docker build -f Dockerfile.server-hs -t server-hs .
    ;;
  kibana-hs)
    echo "Building kibana-hs ..."
    docker build -f Dockerfile.kibana-hs -t kibana-hs .
    ;;
esac
