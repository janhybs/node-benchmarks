#!/usr/bin/python
# -*- coding: utf-8 -*-
# author:   Jan Hybs

import subprocess


def create(command, *args, **kwargs):
    """
    :type command: str | list
    """
    if type(command) is str:
        result = command.split()
    else:
        result = command

    if args:
        result.extend(args)

    return result


def run_command(command, *args, **kwargs):
    print('[ RUN ]', ' '.join(command))
    print('[ CWD ]', kwargs.get('cwd', '.'))

    try:
        process = subprocess.Popen(command, *args, **kwargs)
        returncode = process.wait()
    except FileNotFoundError as e:
        print(e)
        returncode = 1

    if returncode == 0:
        print("[  OK ] | process exited with 0")
        return process
    else:
        print("[ ERROR ] | process exited with %d" % returncode)
        exit(returncode)