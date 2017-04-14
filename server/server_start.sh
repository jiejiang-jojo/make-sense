#!/bin/bash

ES=elasticsearch-hs
DB=db-hs
KB=kibana-hs
APP=server-hs

ES_IMG=elasticsearch-hs
DB_IMG=postgres-hs
KB_IMG=kibana-hs
APP_IMG=server-hs

VOLUME=${1:-/mnt/data}
CONFIG="$(pwd)/config.yml"
if [ ! -e "$CONFIG" ]; then
  echo "ERROR: $CONFIG is not found, please ensure you have created one."
  exit 1
fi

pip install --user pyyaml
DB_PASSWD="$(python -c "import yaml; print yaml.load(open('$CONFIG'))['db_password']")"
if [ -z "DB_PASSWD" ]; then
  echo "ERROR: db_passwd is missing in $CONFIG"
  exit 1
fi

SERVER_PORT=80

function container_exists(){
  container=$1
  docker container ls -a --format '{{ .Names }}' | grep -E "^$container\$" > /dev/null
}

function container_ip(){
  container=$1
  docker exec $container ip r
}

function create_container(){
  case $1 in
    e)
      docker run --name $ES -v ${VOLUME}/elasticsearch:/usr/share/elasticsearch/data -d $ES_IMG
      ;;
    d)
      docker run --name $DB -e "POSTGRES_PASSWORD=$DB_PASSWD" -v ${VOLUME}/postgres:/var/lib/postgresql/data -v $CONFIG:/config.yml -d $DB_IMG
      ;;
    k)
      docker run --name $KB --link ${ES}:elasticsearch -p 5601:5601 -d $KB_IMG
      ;;
    s)
      docker run --name $APP --link ${ES}:elasticsearch --link ${DB}:postgres -v $CONFIG:/app/config.yml -p ${SERVER_PORT}:80 -d $APP_IMG
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
