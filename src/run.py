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

import tul.flow123d.benchmark as benchmark

# call benchmark methods
benchmark.install_requirements()
json_file = benchmark.run_tests()
benchmark.save_to_db(json_file)
