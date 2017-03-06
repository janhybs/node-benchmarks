#!/usr/bin/python
# -*- coding: utf-8 -*-
# author:   Jan Hybs

import sys
import os


# declare common paths
__dir__ = os.path.join(os.path.dirname(__file__))
__root__ = os.path.abspath(os.path.join(__dir__, '../'))

# add common paths to sys.path
sys.path.append(os.path.join(__root__, 'libs'))
sys.path.append(os.path.join(__root__, 'dist-packages'))

# change language
os.environ['LC_ALL'] = 'C'

from tul.flow123d.benchmark import install_requirements, Benchmark, BenchmarkConfig as bc

# call benchmark methods
install_requirements()

# values from program arguments
version, tag, repeat, per_line, spread = sys.argv[1:]

# create one simple config
configs = [
    bc(version=version, per_line=int(per_line), spread=int(spread), repeat=int(repeat), tag=tag)
]

# run benchmark in separated space
with Benchmark(configs) as b:
    b.compile()
    b.run_tests()

# benchmark.make_all()
# json_file = benchmark.run_tests()
# benchmark.save_to_db(json_file)
