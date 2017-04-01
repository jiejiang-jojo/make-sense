FROM debian:jessie

RUN DEBIAN_FRONTEND=noninteractive apt-get -q -y update && \
    apt-get -q -y install python-pip python-dev python-psycopg2 && \
    mkdir /app

ADD ./app/srv_reqirements.txt /app/srv_reqirements.txt
RUN pip install -r /app/srv_reqirements.txt

ADD ./app /app

WORKDIR /app
ENTRYPOINT ["/app/entrypoint.sh"]
