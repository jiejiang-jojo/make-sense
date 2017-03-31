#!/bin/bash

set -e

python update_config.py config.json

exec uwsgi --ini /app/uwsgi.ini
