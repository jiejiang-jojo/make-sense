#!/usr/bin/env python
# -*- coding: utf-8 -*-

"""
Some useful functions
"""
from sqlalchemy.engine.url import URL

def get_db_url(config, db_name=None):
    return URL(drivername='postgresql',
               host=config['db_server'],
               port=config['db_server_port'],
               username=config['db_user'],
               password=config['db_password'],
               database=config['db_name'] if db_name is None else db_name
              )
