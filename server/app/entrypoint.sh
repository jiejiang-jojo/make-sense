#!/bin/bash

set -e

python update_config.py config.yml

exec uwsgi --ini /app/uwsgi.ini
