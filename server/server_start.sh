#!/bin/bash

ES=elasticsearch-hs
DB=db-hs
KB=kibana-hs
APP=server-hs
RAND="$(openssl rand -base64 32)"
DB_PASSWD="${RAND:0:10}"
SERVER_PORT=${1:-80}

function container_exists(){
  container=$1
  docker container ls -a --format '{{ .Names }}' | grep $container > /dev/null
}

function container_ip(){
  container=$1
  docker exec $container ip r
}

function create_container(){
  case $1 in
    e)
      docker run --name $ES -d elasticsearch
      ;;
    d)
      docker run --name $DB -e "POSTGRES_PASSWORD=$DB_PASSWD" -d postgres
      sleep 3
      docker exec $DB sed -i 's/^max_connections.*/max_connections = 500/' /var/lib/postgresql/data/postgresql.conf
      docker restart $DB
      ;;
    k)
      docker run --name $KB --link ${ES}:elasticsearch -p 127.0.0.1:5601:5601 -d kibana
      ;;
    s)
      touch ./config.json
      docker run --name $APP --link ${ES}:elasticsearch --link ${DB}:postgres -v $(pwd)/config.json:/app/config.json -p ${SERVER_PORT}:80 -d server-hs
      ;;
    *)
      echo "Unknow container"
      ;;
  esac
}

for container in $ES $KB $DB $APP; do
  if container_exists $container; then
    docker start $container
  else
    create_container ${container:0:1}  # use the first letter in the container name
  fi
done
