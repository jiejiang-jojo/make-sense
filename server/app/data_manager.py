#!/usr/bin/env python
# -*- coding: utf-8 -*-

from datetime import datetime
import sqlalchemy
from sqlalchemy import create_engine
from sqlalchemy.ext.declarative import declarative_base
from sqlalchemy import Column, Integer, DateTime, Float, Text
from elasticsearch import Elasticsearch
from elasticsearch.exceptions import TransportError
import server_config as sc

Base = declarative_base()

class SensorboxReading(Base):  # pylint: disable-msg=no-init,too-few-public-methods
    """ Table containing readings from Desktop Egg sesnor boxes"""
    __tablename__ = "sensorbox_reading"
    id = Column(Integer, primary_key=True)
    timestamp = Column('timestamp', DateTime)
    sensorbox_id = Column('sensorbox_id', Integer)
    temperature = Column('temperature', Float)
    humidity = Column('humidity', Float)
    gesture = Column('gesture', Integer)
    light = Column('light', Float)
    dust_density = Column('dust_density', Float)
    sound = Column('sound', Float)
    range = Column('range', Float)


class EnergyReading(Base):  # pylint: disable-msg=no-init,too-few-public-methods
    """ Table containing readings from Current Cost energy sensors"""
    __tablename__ = "energy_reading"
    id = Column(Integer, primary_key=True)
    timestamp = Column('timestamp', DateTime)
    source = Column('source', Text)
    days_since_run = Column('days_since_run', Integer)
    temperature = Column('e_temperature', Float)
    energy_sensor = Column('energy_sensor', Integer)
    household = Column('household', Integer)
    radio_id = Column('e_radio_id', Text)
    energy_type = Column('energy_type', Integer)
    watts = Column('watts', Integer)


TABLES = {
    'energy': EnergyReading.__table__,
    'box': SensorboxReading.__table__
}

MAPPINGS = {
    'energy': {
        'src': 'source',
        'dsb': 'days_since_run',
        'timestamp': 'timestamp',
        'tmpr': 'e_temperature',
        'sensor': 'energy_sensor',
        'household': 'household',
        'id': 'e_radio_id',
        'type': 'energy_type',
        'watts': 'watts'
    },

    'box': {
        "B": "sensorbox_id",
        "T": "timestamp",
        "P": "temperature",
        "H": "humidity",
        "G": "gesture",
        "L": "light",
        "D": "dust_density",
        "S": "sound",
        "R": "range"
    }
}

PROCESSORS = {
    'energy': lambda r: convert_timestamp(enlong_keynames(r, MAPPINGS['energy'])),
    'box': lambda r: convert_timestamp(enlong_keynames(r, MAPPINGS['box']))
}

def enlong_keynames(rec, mapping):
    return {v: rec[k] for k, v in mapping.items()}


def convert_timestamp(rec):
    rec['timestamp'] = datetime.fromtimestamp(int(rec['timestamp']))
    return rec


def process_records(records, process):
    return [process(r) for r in records]


def insert_to_db(records, rec_type):
    sc.get_db().execute(TABLES[rec_type].insert(), process_records(records, PROCESSORS[rec_type]))


def insert_to_es(records, rec_type, retries=3):
    data = process_records(records, PROCESSORS[rec_type])
    bulk_data = list()
    for rec in data:
        bulk_data.append(sc.get_es_op())
        bulk_data.append(rec)
    while retries > 0:
        try:
            sc.get_es().bulk(body=bulk_data)
            return
        except TransportError as err:
            print err.status_code
            retries -= 1
    raise err
