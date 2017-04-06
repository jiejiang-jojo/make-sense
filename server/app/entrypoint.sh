#!/bin/bash

set -e

python configure.py init_db

exec uwsgi --ini /app/uwsgi.ini
