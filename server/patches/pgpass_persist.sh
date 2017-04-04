#!/bin/bash


PGPASS_FILE="$PGDATA/postgres_passwd"

if [ ! -e "$PGPASS_FILE" ]; then
  echo "$POSTGRES_PASSWORD" > "$PGPASS_FILE"
else
  awk "/^db_password/ {print \"db_password: $(cat $PGPASS_FILE)\"} {print \$0}" /config.yml > /config.yml.tmp
  mv /config.yml /config.yml.bak && cat /config.yml.tmp > /config.yml
fi
