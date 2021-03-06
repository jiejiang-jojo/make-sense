#!/bin/bash


URL=${1:-http://localhost/energy-record}
REQUESTS=${2:-10000}
THREADS=${3:-200}
PAYLOAD=${4:-$(mktemp)}
if [ -z "$4" ]; then
  python2 energy_reading_forwarder.py payload > "$PAYLOAD"
fi
ab -p "$PAYLOAD" -c $THREADS -n $REQUESTS $URL
