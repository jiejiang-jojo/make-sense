#!/bin/bash

set -e

echo "Try to init db..."
python configure.py initdb

echo "Starting uwsgi..."
exec uwsgi --ini /app/uwsgi.ini
