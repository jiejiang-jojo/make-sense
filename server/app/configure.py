#!/usr/bin/env python
# -*- coding: utf-8 -*-
"""
File: gen_config.py
Description: Generating passwords for config files
"""

import os
import sys
import time
from random import choice
import sqlalchemy as sa
from data_manager import Base
import yaml
import click
from util import get_db_url

def char_range(a, z):
    return ''.join([chr(x) for x in range(ord(a), ord(z) + 1)])

ALPHANUM = '_-[]()<>{$}&^:' + char_range('a', 'z') + char_range('A', 'Z') + char_range('0', '9')


def mkpassword(length):
    return ''.join((choice(ALPHANUM) for _ in range(length)))


def is_inited(config, key):
    return config.get('aes_key', '--') != '--'


def update_aes(config):
    if not is_inited(config, 'aes_key'):
        config['aes_key'] = mkpassword(16)
        config['aes_iv'] = mkpassword(16)
    return config

def update_pgpass(config):
    alt_passwd = os.environ.get('POSTGRES_ENV_POSTGRES_PASSWORD', None)
    if alt_passwd is None:
        print >>sys.stderr, \
            "WARNING: No db is linked to this container, try --link some-postgres:postgres"
    while try_connect(config) is None:
        print "Try connect with both passwords."
        alt_passwd, config['db_password'] = config['db_password'], alt_passwd
        time.sleep(1)
    return config


def create_pgpass(config):
    config['db_password'] = mkpassword(16)
    return config


def update_config(filename):
    with open(filename) as fin:
        config = yaml.load(fin)
    config = update_pgpass(update_aes(config))
    save_config(filename, config)
    return config


def create_config(filename):
    config = {
        "aes_key": "--",
        "aes_iv": "--",
        "db_user": "postgres",
        "db_password": "--",
        "db_server": "postgres",
        "db_server_port": "5432",
        "db_name": "HomeSenseDB",
        "es_servers": ["elasticsearch"],
        "es_index": "sensor-suite",
        "es_type": "sensor-record",
        "server_url": "http://localhost/energy-record"
    }
    config = update_aes(create_pgpass(config))
    save_config(filename, config)
    return config


def save_config(filename, config):
    with open(filename, 'w') as fout:
        fout.write(yaml.dump(config,
                             default_flow_style=False,
                             allow_unicode=True,
                             encoding = None))


def try_connect(config):
    engine = sa.create_engine(get_db_url(config, 'postgres'),
                              isolation_level='AUTOCOMMIT')
    try:
        engine.execute('select 1')
        return engine
    except:
        return None

def init_db(config):
    engine = try_connect(config)
    while engine is None:
        print 'Waiting for Postgres'
        time.sleep(1)
        engine = try_connect(config)

    db_name = config['db_name']
    sql = "select count(*) from pg_catalog.pg_database where datname = '{0}';"
    if engine.execute(sql.format(db_name)).fetchone()[0] == 0:
        print 'Creating DB {0} ...'.format(db_name)
        engine.execute('create database "{0}";'.format(db_name))
    print 'Initializing DB {0} ...'.format(db_name)
    Base.metadata.create_all(sa.create_engine(get_db_url(config)))


@click.command()
@click.option('-c', '--config', 'filename', default="/app/config.yml",
              help="the config file, default: /app/config.yml")
@click.argument('action')
def console(filename, action):
    if action == 'create':
        create_config(filename)
    elif action == 'update':
        update_config(filename)
    elif action == 'initdb':
        with open(filename) as fin:
            config = yaml.load(fin)
        init_db(config)
    else:
        print 'Error: action {0} is not recognized!'.format(action)
        sys.exit(-1)


if __name__ == "__main__":
    console()
