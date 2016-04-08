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

# cursor = bench.aggregate([
#     {
#         '$match': {}
#     },
#     # {
#     #     '$limit': 10
#     # },
#     {
#         '$group': {
#             '_id': '$node_family',
#             # '_id': '$node_family',
#             '_node_name': {'$push': '$node_name'},
#
#             'io_lat_read': {'$push': '$io.lat.read.k_files_per_sec'},
#             'io_lat_write': {'$push': '$io.lat.write.k_files_per_sec'},
#             'io_lat_del': {'$push': '$io.lat.del.k_files_per_sec'},
#
#
#             'io_band_read': {'$push': {'$multiply': ['$io.band.read.MB_per_sec.4096', 1/24.]}},
#             'io_band_write': {'$push': {'$multiply': ['$io.band.write.MB_per_sec.4096', 1/24.]}},
#
#             # 'cpu_read_l1': {'$push': '$cpu.read.MB_per_sec.16'},
#             # 'cpu_write_l1': {'$push': '$cpu.write.MB_per_sec.16'},
#             'cpu_rw_l1': {'$push': '$cpu.rw.MB_per_sec.16'},
#
#             # 'cpu_read_l2': {'$push': '$cpu.read.MB_per_sec.128'},
#             # 'cpu_write_l2': {'$push': '$cpu.write.MB_per_sec.128'},
#             'cpu_rw_l2': {'$push': '$cpu.rw.MB_per_sec.128'},
#
#             # 'cpu_read_l3': {'$push': '$cpu.read.MB_per_sec.512'},
#             # 'cpu_write_l3': {'$push': '$cpu.write.MB_per_sec.512'},
#             'cpu_rw_l3': {'$push': '$cpu.rw.MB_per_sec.512'},
#
#             'cpu_read_ram': {'$push': '$cpu.read.MB_per_sec.8192'},
#             'cpu_write_ram': {'$push': '$cpu.write.MB_per_sec.8192'},
#             'cpu_rw_ram': {'$push': '$cpu.rw.MB_per_sec.8192'},
#
#
#             'cpu_reg': {'$push': '$cpu.reg.MB_per_sec'},
#         }
#     },
#     # {
#     #     '$limit': 1
#     # },
# ])

# results = [node for node in cursor]
# keys = [k for k in results[0].keys() if not k.startswith('_')]
# keys = [k for k in keys if k not in ('cpu_reg')]
# keys = sorted(keys)
# excel = [['id']]
# excel[0].extend(keys)
#
# for node in results:
#     vals = [node['_id']]
#
#     for k in keys:
#         vals.append(np.average(node[k]))
#
#     excel.append(vals)
#
# excel_str = ""
# for line in excel:
#     excel_str += '\t'.join([str(l) for l in line]).replace('.', ',') + '\n'
# print excel_str
# clipboard.copy(excel_str)

cursor = bench.aggregate([
    {
        '$match': {}
    },
    # {
    #     '$limit': 20
    # },
    {
        '$group': {
            '_id': '$node_family',
            # '_id': '$node_family',
            'cpu': {'$push': '$cpu'},
            'io': {'$push': '$io'},
        }
    },
    # {
    #     '$limit': 1
    # },
])


def get_values(dot_path, lst):
    double = False

    if dot_path.endswith('.*'):
        double = True
        dot_path = dot_path[:-2]

    sections = dot_path.split(".")
    values = []
    for item in lst:
        o = item
        for s in sections:
            o = o.get(s)
        values.append(o)
    if double:
        v = {}
        for item in values:
            for key, val in item.items():
                if not v.has_key(key):
                    v[key] = []
                v[key].append(val)
        return v
    return values


clip = []
for node in cursor:
    cpu_section = node['cpu']
    io_section = node['io']

    node_clip = dict()

    node_clip['_id'] = node['_id']

    # for k, v in get_values("read.MB_per_sec.*", cpu_section).items():
    #     node_clip['cpu_read_{:06d}'.format(int(k))] = np.average(v)
    #
    # for k, v in get_values("write.MB_per_sec.*", cpu_section).items():
    #     node_clip['cpu_write_{:06d}'.format(int(k))] = np.average(v)
    #
    # for k, v in get_values("rw.MB_per_sec.*", cpu_section).items():
    #     node_clip['cpu_rw_{:06d}'.format(int(k))] = np.average(v)

    # io section
    # for k, v in get_values("band.read.MB_per_sec.*", io_section).items():
    #     # node_clip['io_read_band_{:07d}'.format(int(k))] = np.average(v)
    #     node_clip['{:07d}'.format(int(k))] = np.average(v)

    for k, v in get_values("band.write.MB_per_sec.*", io_section).items():
        # node_clip['io_write_band_{:07d}'.format(int(k))] = np.average(v)
        node_clip['{:07d}'.format(int(k))] = np.average(v)

    clip.append(node_clip)


keys = [k for k in clip[0].keys() if not k.startswith('_')]
keys = [k for k in keys if k not in ('cpu_reg', )]
keys = sorted(keys)
keys = ['_id'] + keys

excel = '\t'.join(keys) + '\n'
for l in clip:
    excel += '\t'.join([str(l.get(p, 0)) for p in keys]).replace('.', ',') + '\n'

clipboard.copy(excel)
print excel