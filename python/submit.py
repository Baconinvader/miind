#!/usr/bin/env python
'''Expects the job name - not a path. Will fail if there is no directory
in '${MIIND_ROOT}/build/jobs/[jobname]' with a corresponding job name.'''

import directories
import sys
import os
import subprocess

class cd:
    """Context manager for changing the current working directory"""
    def __init__(self, newPath):
        self.newPath = os.path.expanduser(newPath)

    def __enter__(self):
        self.savedPath = os.getcwd()
        os.chdir(self.newPath)

    def __exit__(self, etype, value, traceback):
        os.chdir(self.savedPath)

if __name__ == "__main__":
    
    if len(sys.argv) != 2:
        print 'submit requirs exactly one argument: the subdirectory under which the jobs are organized.'
        sys.exit()
    dir=sys.argv[1]

    build = os.path.join(directories.miind_root(),'build')
    path  = os.path.join(build,'jobs',dir)

    with cd(build):
       	subprocess.call(["ls","-l"])
        subprocess.call(['make'])

    with open(os.path.join(path,'joblist')) as f:
       	lines = f.readlines()
       	with cd(path):
	       	for line in lines:
	       		name = line.split()[0]
	       		subprocess.call([name, '>&log&'])

