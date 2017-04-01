#!/bin/bash

ES=elasticsearch-hs
DB=db-hs
KB=kibana-hs
APP=server-hs

ES_IMG=docker.elastic.co/elasticsearch/elasticsearch:5.3.0
DB_IMG=postgres-hs
KB_IMG=kibana-hs
APP_IMG=server-hs

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
      docker run --name $ES -d $ES_IMG
      ;;
    d)
      docker run --name $DB -e "POSTGRES_PASSWORD=$DB_PASSWD" -d $DB_IMG
      ;;
    k)
      docker run --name $KB --link ${ES}:elasticsearch -p 5601:5601 -d $KB_IMG
      ;;
    s)
      touch ./config.yml
      docker run --name $APP --link ${ES}:elasticsearch --link ${DB}:postgres -v $(pwd)/config.yml:/app/config.yml -p ${SERVER_PORT}:80 -d $APP_IMG
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
