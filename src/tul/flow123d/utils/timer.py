#!/usr/bin/python
# -*- coding: utf-8 -*-
# author:   Jan Hybs

import time


class Timer(object):
    """
    Class Timer measures elapsed time between tick and tock
    :type app_timer: Timer
    """
    app_timer = None

    def __init__(self, name=None, print=True):
        self.time = 0
        self.name = name
        self.duration = 0
        self.print = print

    def tick(self):
        self.time = time.time()

    def tock(self):
        self.duration = time.time() - self.time

    def __enter__(self):
        self.tick()
        return self

    def __exit__(self, exc_type, exc_val, exc_tb):
        self.tock()
        if self.print:
            print(self)
        return False

    def __repr__(self):
        if self.name is None:
            return "{:6.3f}".format(self.duration)
        return "{:20s}: {:6.3f} sec".format(self.name, self.duration)


Timer.app_timer = Timer("app")