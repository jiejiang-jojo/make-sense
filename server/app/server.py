#!/usr/bin/env python
# -*- coding: utf-8 -*-
""" Server for recieving records from sensor boxes and energy sensors"""
import json
import binascii

from flask import request
import server_config as sc
from server_config import APP

import data_manager


@APP.route('/currenttime', methods=['GET'])
def currenttime():
    return "homesense"

def decode_record(data):
    coded = binascii.a2b_base64(data)
    buf = sc.get_crypter().decrypt(coded)
    end_sym = buf.find('}]')
    payload =  buf[:end_sym + 2]
    # The following line seems redundant
    # result = record.replace('\n', '').replace("'", '"')
    records = json.loads(payload)
    return records

@APP.route('/box-record', methods=['POST'])
def parse_box_request_record():
    records = decode_record(request.data)
    data_manager.insert_to_db(records, 'box')
    data_manager.insert_to_es(records, 'box')
    return "ok"

@APP.route('/energy-record', methods=['POST'])
def parse_energy_request_record():
    records = decode_record(request.data)
    data_manager.insert_to_db(records, 'energy')
    data_manager.insert_to_es(records, 'energy')
    return "ok"
