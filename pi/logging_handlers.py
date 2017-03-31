"""
File: reading_handlers.py
Author: Wen Li
Email: wen.li@ucl.ac.uk
Github: http://github.com/spacelis
Description:
    Writing reocrds to files with rotating every day.
"""
import os
import time
import gzip
import json
import shutil
import logging
import binascii
from logging.handlers import TimedRotatingFileHandler
from multiprocessing import Process
import requests as rq
from Crypto.Cipher import AES


def compress(filename):
    """ Compress the file

    :filename: TODO
    :returns: TODO

    """
    with open(filename, 'rb') as f_in, gzip.open(filename + '.gz', 'wb') as f_out:
        shutil.copyfileobj(f_in, f_out)
    os.remove(filename)


def start_compress(filename):
    """ Compress the file with a new process.

    :filename: a path to a file to compress
    :returns: TODO

    """
    proc = Process(target=compress, args=(filename,))
    proc.start()


class TimedRotatingCompressedFileHandler(TimedRotatingFileHandler):

    """Hanlder compress the data after rotating."""

    def __init__(self, *args, **kwargs):
        """

        :*args: See logging.handlers.TimedRotatingFileHandler
        :**kwargs: See logging.handlers.TimedRotatingFileHandler

        """
        super(TimedRotatingCompressedFileHandler, self).__init__(*args, **kwargs)

    def doRollover(self):
        """ Rotating with compression.
        :returns: None

        """
        # pylint: disable-msg=invalid-name

        # get the time that this sequence started at and make it a TimeTuple
        # copied from logging.handlers.TimedRotatingFileHandler.doRollover
        currentTime = int(time.time())
        dstNow = time.localtime(currentTime)[-1]
        t = self.rolloverAt - self.interval
        if self.utc:
            timeTuple = time.gmtime(t)
        else:
            timeTuple = time.localtime(t)
            dstThen = timeTuple[-1]
            if dstNow != dstThen:
                if dstNow:
                    addend = 3600
                else:
                    addend = -3600
                timeTuple = time.localtime(t + addend)
        dfn = self.baseFilename + "." + time.strftime(self.suffix, timeTuple)
        # Copy ends here
        # Now we have the file going to be left after rotating
        super(TimedRotatingCompressedFileHandler, self).doRollover()
        start_compress(dfn)


def send_http_request(method, url, data):
    """ Send the data to the server at URL

    :method: 'POST' or 'GET'
    :url: The endpoint url for submtting the data
    :data: the logging records
    :returns: TODO

    """
    if method == 'POST':
        rq.post(url, data=data)
    elif method == 'GET':
        rq.get(url, data=data)


def start_send_http_request(method, url, data):
    """ Submitting data in a separate process

    :method: 'POST' or 'GET'
    :url: The endpoint url for submtting the data
    :data: the logging records
    :returns: None

    """
    proc = Process(target=send_http_request, args=(method, url, data))
    proc.start()


def encode(data, aes_key, aes_iv):
    """ Encrypting the data for sending to server

    :data: TODO
    :returns: TODO

    """
    padding = '\x00' * (16 - (len(data) % 16))
    crypter = AES.new(aes_key, AES.MODE_CBC, aes_iv)
    aes_coded = crypter.encrypt(data + padding)
    b64coded = binascii.b2a_base64(aes_coded)
    return b64coded


class AccumulatedHTTPHandler(logging.Handler):
    """ Sending log to a HTTP server
    """
    def __init__(self, url, aes_key, aes_iv, bufsize=10, method="POST"):
        """
        """
        super(AccumulatedHTTPHandler, self).__init__()
        method = method.upper()
        if method not in ["GET", "POST"]:
            raise ValueError("method must be GET or POST")
        self.url = url
        self.aes_key = aes_key
        self.aes_iv = aes_iv
        self.method = method
        self.bufsize = bufsize
        self.buffer = list()

    def emit(self, record):
        """ Accumulating a record or send the accumulated records to the HTTP
            server
        """
        try:
            self.buffer.append(json.loads(self.format(record)))
            if len(self.buffer) >= self.bufsize:
                payload = encode(json.dumps(self.buffer), self.aes_key, self.aes_iv)
                start_send_http_request(self.method, self.url, payload)
                self.buffer = list()
        except (KeyboardInterrupt, SystemExit):
            raise
        except:  # pylint: disable-msg=bare-except
            self.handleError(record)
