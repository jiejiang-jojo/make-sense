#!/usr/bin/env python
""" A configuration of logging handlers for energy sensors.
"""
import json
import time
import logging
from logging.config import dictConfig



def get_logging_config(config):
    """ Configuring logging with two handlers: one for local files roatating
        everyday, the other is sending to http server.

    :config: a config dict
            `aes_key`, `aes_iv` and `server_url` are mandatory
    :returns: A configuration dict for dictConfig()

    """
    return {
        'version': 1,
        'formatters': {
            'message_only': {
                'format': '%(message)s'
            }
        },
        'handlers': {
            'file': {
                'class': 'logging_handlers.TimedRotatingCompressedFileHandler',
                'level': 'INFO',
                'formatter': 'message_only',
                'when': 'M',
                'filename': config.get('filename', './energy/log'),
            },
            'http': {
                'class': 'logging_handlers.AccumulatedHTTPHandler',
                'level': 'INFO',
                'formatter': 'message_only',
                'aes_key': config['aes_key'],
                'aes_iv': config['aes_iv'],
                'url': config['server_url'],
                'bufsize': config.get('http_buf_size', None),
            }
        },
        'loggers': {
            'SensorReading': {
                'level': 'INFO',
                'handlers': ['file', 'http']
            }
        }

    }

if __name__ == "__main__":
    dictConfig(get_logging_config({
        'server_url': 'http://localhost:5000/sensor-record',
        'aes_iv': '0123456789abcdef',
        'aes_key': '0123456789abcdef'
    }))
    logger = logging.getLogger('SensorReading')
    while True:
        tmsp = time.strftime('%Y-%d-%m %H:%M:%S')
        logger.info(json.dumps({'t': tmsp, 'msg': 'test message'}))
        time.sleep(1)
