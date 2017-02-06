#!/usr/bin/python
# -*- coding: utf-8 -*-
# author:   Jan Hybs

import os
from tul.flow123d.utils.popen import create, run_command
command = create('pip3 install -r requirements.txt --target .')


def install_requirements_libs(cwd, skip_if_exists=True):
    if skip_if_exists:
        files = set(os.listdir(cwd)) - {'.gitignore', 'requirements.txt'}
        if len(files) > 0:
            return True
    return run_command(command, cwd=cwd)