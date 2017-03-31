#!/usr/bin/env python
# -*- coding: utf-8 -*-

import json
import threading
import sqlalchemy as sa
from elasticsearch import Elasticsearch
from Crypto.Cipher import AES
from flask import Flask
from flask import g
from util import get_db_url

APP = Flask(__name__)

CONFIG_FILE = './config.json'

THREAD_STACK = threading.local()

def local_cached(func):
    """ Wrapping the function with local thread caches enabled by flask.g """
    varname = '_' + func.__name__
    def wrapper():
        """ A wrapper function """
        cached = getattr(THREAD_STACK, varname, None)
        if cached is None:
            setattr(THREAD_STACK, varname, func())
            with open('./cache.log', 'a') as fout:
                print >>fout, 'created', func.__name__
        return getattr(THREAD_STACK, varname)
    return wrapper

@local_cached
def get_config():
    """ Read configuration from the file and cache it with flask
    """
    with open(CONFIG_FILE) as fin:
        return json.load(fin)


def get_crypter():
    """ Return a crypter engine
    """
    config = get_config()
    return AES.new(config['aes_key'], AES.MODE_CBC, config['aes_iv'])


@local_cached
def get_db():
    """ Return an sa engine"""
    config = get_config()
    return sa.create_engine(get_db_url(config))


@local_cached
def get_es():
    """ Return an es client """
    config = get_config()
    return Elasticsearch(config['es_servers'])


def get_es_op():
    """ Return index op for bulk es insert """
    config = get_config()
    return {
        'index': {
            '_index': config.get('es_index', 'sensor-suite-test'),
            '_type': config.get('es_type', 'sensor-record')
        }
    }

