#!/usr/bin/env python
""" Reading data from the energy sensor, store them and send them to server.
"""

import os
import sys
import traceback
import time
import json
import logging
from logging_config import get_logging_config

from xml.etree.ElementTree import fromstring
from xmljson import badgerfish as bf
from serial import Serial
from serial.serialutil import SerialException
from logging_handlers import encode
import yaml


CONFIG = './config.yml'


def get_or_default(obj, path, typ=str, default=None):
    """ get the value from the dict with proper type"""
    path.reverse()
    elem = obj
    while elem is not None and len(path) > 0:
        p = path.pop()
        elem = elem.get(p, None)
    if elem is not None:
        return typ(elem)
    return default


def parse_record(line, appendix):
    """ parse the record from XML"""
    xml = bf.data(fromstring(line))
    msg = xml['msg']
    record = {
        'src': get_or_default(msg, ['src', '$']),
        'dsb': get_or_default(msg, ['dsb', '$'], int),
        'timestamp': int(time.time()),
        'tmpr': get_or_default(msg, ['tmpr', '$'], float),
        'sensor': get_or_default(msg, ['sensor', '$'], int),
        'id': get_or_default(msg, ['id', '$']),
        'type': get_or_default(msg, ['type', '$'], int),
        'watts': get_or_default(msg, ['ch1', 'watts', '$'], int)
    }
    record.update(appendix)
    return record


def find_usb_tty(prefix='/dev/ttyUSB{0}'):
    """ Find the usb.
    """
    for i in range(10):
        dev = prefix.format(i)
        if os.path.exists(dev):
            return dev


def main():
    """ reading and forwarding
    """
    with open(CONFIG) as fin:
        config = yaml.load(fin)
        logging.config.dictConfig(get_logging_config(config))
        household = config['household']
    logger = logging.getLogger('SensorReading')
    dev = find_usb_tty()
    while True:
        try:
            print 'Hooking up to {0}'.format(dev)
            with Serial(dev, 57600, bytesize=8, parity='N', stopbits=1) as serial_port:
                for line in serial_port:
                    try:
                        rec = parse_record(line, {'household': household})
                        logger.info(json.dumps(rec))
                    except Exception:  # pylint: disable-msg=broad-except
                        traceback.print_exc(file=sys.stdout)
        except SerialException:
            traceback.print_exc(file=sys.stdout)


def throttled_mock_record():
    for rec in mock_records():
        yield rec
        time.sleep(1)


def mock_records():
    from random import randint
    import time
    while True:
        yield {
            'src': 'mock cc',
            'dsb': randint(1, 10),
            'timestamp': int(time.time()),
            'tmpr': randint(12, 30),
            'sensor': randint(0, 7),
            'id': randint(0, 100),
            'type': 1,
            'household': 1234,
            'watts': randint(0, 1000)
        }


def mock_main():
    with open(CONFIG) as fin:
        logging.config.dictConfig(get_logging_config(yaml.load(fin)))
    logger = logging.getLogger('SensorReading')
    for rec in throttled_mock_record():
        logger.info(json.dumps(rec))


def mock_payload():
    with open(CONFIG) as fin:
        config = yaml.load(fin)
    aes_key, aes_iv = config['aes_key'], config['aes_iv']
    data = [rec for _, rec in zip(range(10), mock_records())]
    print encode(json.dumps(data), aes_key, aes_iv)


if __name__ == '__main__':
    import sys
    if len(sys.argv) > 1:
        if sys.argv[1] == 'mock':
            mock_main()
        elif sys.argv[1] == 'payload':
            mock_payload()
        else:
            print "Error the only argument should be either `mock` or `payload`."
            print "Or none should be provided."
    else:
        main()
