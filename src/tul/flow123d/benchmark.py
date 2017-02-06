#!/usr/bin/python
# -*- coding: utf-8 -*-
# author: Jan Hybs
import os
import sys
import json
import socket
import getpass
import time

from tul.flow123d.utils.popen import create, run_command
from tul.flow123d.utils.install_libs import install_requirements_libs
from tul.flow123d.utils.timer import Timer

__dir__ = os.path.join(os.path.dirname(__file__))
__root__ = os.path.abspath(os.path.join(__dir__, '../../../'))


def install_requirements():
    # install libs and then move on
    install_requirements_libs(os.path.join(__root__, 'libs'))


# declare variables
benchmark = os.path.join(__root__, 'benchmark')
hostname = socket.gethostname()
username = getpass.getuser()
now = int(time.time())


def make_all():
    # call makefile
    with Timer("Step 'make all'"):
        command = create('make all')
        run_command(command, cwd=benchmark)


def run_tests():
    # call tests
    with Timer("Step 'run tests'"):
        json_name = '{now}_{hostname}_{username}.json'.format(now=now, hostname=hostname, username=username)
        json_file = os.path.join(benchmark, json_name)
        command = create('./O3.out', json_name)
        run_command(command, cwd=benchmark)
    return json_file


def save_to_db(json_file):
    from tul.flow123d.db.mongo import Mongo

    # save to db
    with Timer("Step 'save to db'"):
        with open(json_file, 'r') as fp:
            data = json.load(fp)
            data['hostname'] = hostname
            data['username'] = username
            data['machine'] = hostname.split('.')[0].rstrip('1234567890')

            # insert to db
            mongo = Mongo()
            mongo.bench.insert_one(data)
            # mongo.bench.delete_many({})
            print(list(mongo.bench.find({})))
