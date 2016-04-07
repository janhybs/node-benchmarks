#!/usr/bin/python
# -*- coding: utf-8 -*-
# author:   Jan Hybs

import os, sys, json, time, datetime, re
import numpy as np
import clipboard
DATADIR = "../results"


def create_db():
    from pymongo import MongoClient
    client = MongoClient()
    db = client.get_database("bench")
    bench = db.get_collection("bench")
    return db, bench


def parse_filename(filename):
    timestamp = datetime.datetime.fromtimestamp(int(filename[-15:-5]))
    node_id = filename[:-16]
    node_name = node_id.split(".")[0]
    node_family = re.sub(r'[0-9]+$', '', node_name)
    return timestamp, node_id, node_name, node_family


def insert_results(bench):
    json_files = os.listdir(DATADIR)
    for json_file in json_files:
        timestamp, node_id, node_name, node_family = parse_filename(json_file)
        print timestamp, node_id, node_name, node_family
        with file(os.path.join(DATADIR, json_file), 'r') as fp:
            json_data = json.load(fp)
            json_data['timestamp'] = timestamp
            json_data['node_id'] = node_id
            json_data['node_name'] = node_name
            json_data['node_family'] = node_family
            bench.insert_one(json_data)


db, bench = create_db()
# insert_results(bench)

cursor = bench.aggregate([
    {
        '$match': {}
    },
    # {
    #     '$limit': 10
    # },
    {
        '$group': {
            '_id': '$node_family',
            # '_id': '$node_family',
            '_node_name': {'$push': '$node_name'},

            'io_lat_read': {'$push': '$io.lat.read.k_files_per_sec'},
            'io_lat_write': {'$push': '$io.lat.write.k_files_per_sec'},
            'io_lat_del': {'$push': '$io.lat.del.k_files_per_sec'},


            'io_band_read': {'$push': {'$multiply': ['$io.band.read.MB_per_sec.4096', 1/24.]}},
            'io_band_write': {'$push': {'$multiply': ['$io.band.write.MB_per_sec.4096', 1/24.]}},

            # 'cpu_read_l1': {'$push': '$cpu.read.MB_per_sec.16'},
            # 'cpu_write_l1': {'$push': '$cpu.write.MB_per_sec.16'},
            'cpu_rw_l1': {'$push': '$cpu.rw.MB_per_sec.16'},

            # 'cpu_read_l2': {'$push': '$cpu.read.MB_per_sec.128'},
            # 'cpu_write_l2': {'$push': '$cpu.write.MB_per_sec.128'},
            'cpu_rw_l2': {'$push': '$cpu.rw.MB_per_sec.128'},

            # 'cpu_read_l3': {'$push': '$cpu.read.MB_per_sec.512'},
            # 'cpu_write_l3': {'$push': '$cpu.write.MB_per_sec.512'},
            'cpu_rw_l3': {'$push': '$cpu.rw.MB_per_sec.512'},

            'cpu_read_ram': {'$push': '$cpu.read.MB_per_sec.8192'},
            'cpu_write_ram': {'$push': '$cpu.write.MB_per_sec.8192'},
            'cpu_rw_ram': {'$push': '$cpu.rw.MB_per_sec.8192'},


            'cpu_reg': {'$push': '$cpu.reg.MB_per_sec'},
        }
    },
    # {
    #     '$limit': 1
    # },
])

results = [node for node in cursor]
keys = [k for k in results[0].keys() if not k.startswith('_')]
keys = [k for k in keys if k not in ('cpu_reg')]
keys = sorted(keys)
excel = [['id']]
excel[0].extend(keys)

for node in results:
    vals = [node['_id']]

    for k in keys:
        vals.append(np.average(node[k]))

    excel.append(vals)

excel_str = ""
for line in excel:
    excel_str += '\t'.join([str(l) for l in line]).replace('.', ',') + '\n'
print excel_str
clipboard.copy(excel_str)