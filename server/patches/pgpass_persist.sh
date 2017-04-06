#!/bin/bash


PGPASS_FILE="$PGDATA/postgres_passwd"

if [ ! -e "$PGPASS_FILE" ]; then
  echo "$POSTGRES_PASSWORD" > "$PGPASS_FILE"
  chmod 600 "$PGPASS_FILE"
fi
