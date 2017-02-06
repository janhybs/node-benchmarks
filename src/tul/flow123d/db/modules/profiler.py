#!/usr/bin/python
# -*- coding: utf-8 -*-
# author:   Jan Hybs

import re
import json
import hashlib
import datetime


def g(o: dict, p: str, d: object=None):
    t = o
    for k in p.split('.'):
        t = t.get(k, d)
    return t


def s(o: dict, p: str, v: object=None):
    t = o
    ps = p.split('.')
    for k in ps[:-1]:
        t = t.get(k, {})
    t[ps[-1]] = v
    return o


class ProfilerModule(object):
    # include = '**/profiler_info_*.log.json'
    include = '**/*.json'
    exclude = None

    _floats = [
        re.compile('cumul-time-.+'),
        re.compile('percent'),
        re.compile('timer-resolution'),
    ]

    _ints = [
        re.compile('call-count-.+'),
        re.compile('memory-.+'),
        re.compile('file-line'),
        re.compile('task-size'),
        re.compile('run-process-count'),
    ]

    _dates = [
        re.compile('run-started-at'),
        re.compile('run-finished-at'),
    ]

    _children = 'children'

    def __init__(self):
        pass

    @staticmethod
    def _parse_date(s:str) -> datetime.datetime:
        return datetime.datetime.strptime(s, '%m/%d/%y %H:%M:%S').timestamp()

    def process_file(self, f: str):
        with open(f, 'r') as fp:
            obj = json.load(fp)

        # convert fields to ints and floats
        self._convert_fields(obj, self._ints,   int)
        self._convert_fields(obj, self._floats, float)
        self._convert_fields(obj, self._dates, self._parse_date)

        # unwind
        parts = f.split('/')
        base, start = self._get_base(obj)
        base['test-name'] = parts[-4]
        base['case-name'] = parts[-2].split('.')[0]

        return self._unwind(start, list(), base)

    def _get_base(self, obj: dict):
        base = obj.copy()
        if self._children in base:
            del base[self._children]
            return base, obj[self._children][0]

    def _convert_fields(self, obj, fields, method):
        for key in obj:
            for f in fields:
                if f.match(key):
                    obj[key] = method(obj[key])

        if self._children in obj:
            for child in obj[self._children]:
                self._convert_fields(child, fields, method)

    def _unwind(self, obj: dict, result: list, base: dict = None, path: str = ''):
        item = obj.copy()
        if self._children in item:
            del item[self._children]

        tag = item['tag']
        new_path = path + '/' + tag
        indices = dict()
        indices['tag'] = tag
        indices['tag_hash'] = self._md5(tag)
        indices['path'] = new_path
        indices['path_hash'] = self._md5(new_path)
        indices['parent'] = path
        indices['parent_hash'] = self._md5(path)
        item['indices'] = indices
        item['base'] = base.copy()

        result.append(item)

        if self._children in obj:
            for o in obj[self._children]:
                self._unwind(o, result, base, new_path)
        return result

    def _md5(self, s):
        return hashlib.md5(s.encode()).hexdigest()

    @classmethod
    def randomize(cls, cursor, prop, value, prob=0.5):
        import random

        result = []
        for document in cursor:
            if random.random() <= prob:
                result.append(s(document, prop, value))

        return result
