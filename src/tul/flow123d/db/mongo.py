#!/usr/bin/python
# -*- coding: utf-8 -*-
# author:   Jan Hybs

from pymongo import MongoClient
import numpy as np
import hashlib
import re
import datetime


def md5(s=''):
    return hashlib.md5(s.encode()).hexdigest()


def extract(o, k):
    return [i[k] for i in o if k in i]


def secure_tag(s=''):
    return re.sub(r'[^a-zA-Z0-9_]+', '-', s.lower())

number_regex = re.compile(r'[^0-9]*([0-9]+)[^0-9]*')


class Mongo(object):
    """
    Class Mongo manages connection and queries
    :type db          : pymongo.database.Database
    :type bench       : pymongo.database.Collection
    :type nodes       : pymongo.database.Collection
    :type flat        : pymongo.database.Collection
    :type mongo       : Mongo
    """

    mongo = None

    def __init__(self):
        self.client = MongoClient()
        self.db = self.client.get_database('bench')
        self.bench = self.db.get_collection('bench')
        self.nodes = self.db.get_collection('nodes')
        self.flat_copy = self.db.get_collection('flat_copy')
        self.flat = self.db.get_collection('flat')
        Mongo.mongo = self

    def create_flat(self):
        cursor = self.bench.find()

        frames = list()

        def process_one(item, path, measure_id):
            tag = item['tag']
            new_path = path + '/' + tag
            new_item = item.copy()
            indices = dict()
            indices['m_id'] = measure_id
            indices['tag'] = tag
            indices['path'] = new_path
            indices['path_hash'] = md5(new_path)
            indices['parent'] = path
            indices['parent_hash'] = md5(path)
            new_item['indices'] = indices

            if 'children' in item:
                del new_item['children']

            frames.append(new_item)

            if 'children' in item:
                for child in item['children']:
                    process_one(child, new_path, measure_id)

        total = self.bench.count()
        i = 1
        for d in cursor:
            print(i, total)
            i += 1
            if i % 1000 == 0 and i > 0:
                self.flat.insert_many(frames)
                frames = list()

            if 'children' in d:
                process_one(d['children'][0], '', d['_id'])

        self.flat.insert_many(frames)

        from pymongo import IndexModel
        print(self.flat.create_indexes([
            IndexModel('indices.path'),
            IndexModel('indices.path_hash'),
            IndexModel('indices.parent'),
            IndexModel('indices.parent_hash'),
        ]))

        return True

    def ensure_index(self):
        from pymongo import IndexModel
        return self.flat.create_indexes([
            IndexModel('indices.path'),
            IndexModel('indices.path_hash'),
            IndexModel('indices.parent'),
            IndexModel('indices.parent_hash'),
        ])

    @classmethod
    def _to_js_array(cls, items, t=str):
        return '[{}]'.format(', '.join([t(x) for x in items]))

    @classmethod
    def _to_js_date(cls, t: float) -> str:
        return datetime.datetime.fromtimestamp(t).strftime('new Date(%Y, %m, %d, %H, %M, %S)')
