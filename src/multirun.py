#!/usr/bin/python
# -*- coding: utf-8 -*-
# author:   Jan Hybs

import sys
import os
import time
import subprocess
from tul.flow123d.benchmark import install_requirements

install_requirements()

import yaml
os.environ['LC_ALL'] = 'C'

__dir__ = os.path.dirname(__file__)
__root__ = os.path.dirname(__dir__)
run_py = os.path.abspath(os.path.join(__dir__, 'run.py'))

all_clusters = 'ajax', 'exmag', 'hildor', 'luna', 'mudrc', 'tarkil', 'gram'
pbs_template = """
#!/bin/bash
#PBS -l walltime=1:59:00
#PBS -l select=1:ncpus=2:mem=2000mb:cl_{cluster}=True
#PBS -N {version}-{cluster}
# AUTO-GENERATED SCRIPT DO NOT EDIT #

python3 "{run_py}" "{version}" "{tag}" "{repeat}" "{per_line}" "{spread}"
""".strip()


# prefer given value
if len(sys.argv) > 1:
    arg = str(sys.argv[1])
    config_file = os.path.abspath(os.path.join(__root__, arg if arg.endswith('.yaml') else arg + '.json'))
else:
    config_file = os.path.abspath(os.path.join(__root__, 'bench.yaml'))


# check file exists
if not os.path.exists(config_file):
    sys.stderr.write("Test yaml file does not exists '%s'" % config_file)
    exit(1)
else:
    yaml_data = yaml.load(open(config_file, 'r'))

    configs = list()
    for config in yaml_data:
        version = str(config['version'])
        if version.find('{') == -1:
            configs.append(config)
        else:

            l, r = version.find('{'), version.find('}')
            expand = version[l+1:r]
            versions = list(eval(expand))
            for v in versions:
                config_copy = config.copy()
                config_copy['version'] = version[0:l] + str(v) + version[r+1:]
                configs.append(config_copy)

    files = list()
    for c in configs:
        for cluster in all_clusters:
            # {version} {tag} {repeat} {per_line} {spread}
            version = c.get('version')
            tag = c.get('tag')
            repeat = c.get('repeat')
            per_line = c.get('per_line')
            spread = c.get('spread')
            filename = os.path.abspath(os.path.join(__root__, 'pbs', 'pbs-{version}-{cluster}.sh'.format(**locals())))
            files.append(filename)

            # write files
            with open(filename, 'w') as fp:
                fp.write(pbs_template.format(**locals()))

    for i in range(0, len(files), len(all_clusters)):
        file_subset = files[i:i+len(all_clusters)]
        for f in file_subset:
            args = ['qsub', f]
            print(' '.join(args))
            subprocess.Popen(args)

        print('-' * 80)
        # sleep for 20 minutes
        time.sleep(20*60)
