#!/usr/bin/python
# -*- coding: utf-8 -*-
# author: Jan Hybs
import os
import sys
import json
import socket
import getpass
import time
import random
import uuid
import shutil

from tul.flow123d.utils.popen import create, run_command
from tul.flow123d.utils.install_libs import install_requirements_libs
from tul.flow123d.utils.timer import Timer

__dir__ = os.path.join(os.path.dirname(__file__))
__root__ = os.path.abspath(os.path.join(__dir__, '../../../'))


def install_requirements():
    # install libs and then move on
    libs_path = os.path.abspath(os.path.join(__root__, 'libs'))
    sys.path.append(libs_path)
    install_requirements_libs(libs_path)


# declare variables
benchmark = os.path.join(__root__, 'benchmark')
orig_dir = os.path.join(benchmark, 'src')
hostname = socket.gethostname()
username = getpass.getuser()
now = int(time.time())


class BenchmarkConfig(object):
    def __init__(self, version='1.2.1', per_line=0, spread=0, repeat=50, **kwargs):
        self.spread = spread
        self.per_line = per_line
        self.version = version
        self.repeat = repeat
        self.tag = kwargs


class Benchmark(object):
    """
    :type configs : list[BenchmarkConfig]
    """
    def __init__(self, configs=None, random_copy=True):
        from tul.flow123d.db.mongo import Mongo
        self.configs = configs or [BenchmarkConfig()]
        self.benchmark = benchmark
        self.random_dir = False
        self.json_file = None
        self.mongo = Mongo()

        if random_copy:
            self.random_dir = 'copy-' + str(uuid.uuid4())
            self.benchmark = os.path.join(benchmark, self.random_dir)

    def compile(self):
        # call makefile
        with Timer("Step 'make compile'"):
            command = create('make compile')
            run_command(command, cwd=self.benchmark)

    def run_tests(self, save_to_db=True):
        for config in self.configs:
            for repeat in range(config.repeat):
                print('{:02d} of {:02d}'.format(repeat+1, config.repeat))
                with Timer("Step 'run tests' {c.version} ({c.per_line}, {c.spread})".format(c=config)):
                    json_name = '{now}_{hostname}_{username}_{repeat}.json'.format(now=now, hostname=hostname, username=username, repeat=repeat)
                    json_file = os.path.join(self.benchmark, json_name)
                    command = create('./O3.out', json_name, config.version, config.per_line, config.spread)
                    print(command)
                    run_command(command, cwd=self.benchmark)
                    self.json_file = json_file

                    if save_to_db:
                        self.save_to_db(self.json_file, **config.tag)

    def save_to_db(self, json_file=None, **kwargs):
        json_file = json_file or self.json_file

        # save to db
        with Timer("Step 'save to db'"):
            with open(json_file, 'r') as fp:
                data = json.load(fp)
                data['hostname'] = hostname
                data['username'] = username
                data['machine'] = hostname.split('.')[0].rstrip('1234567890')

                data.update(kwargs)

                # insert to db
                self.mongo.bench.insert_one(data)

    def __enter__(self):
        if self.random_dir:
            shutil.copytree(orig_dir, self.benchmark)
        return self

    def __exit__(self, exc_type, exc_val, exc_tb):
        if self.random_dir:
            shutil.rmtree(self.benchmark)
        return False
