from distutils.spawn import find_executable
from distutils.dep_util import newer_group
from setuptools import setup, Extension
from setuptools.command.build_ext import build_ext

import re
import os
import subprocess
import fnmatch
import json

def c_files_in(directory):
    paths = []
    names = os.listdir(directory)
    for f in fnmatch.filter(names, '*.c'):
        paths.append(os.path.join(directory, f))
    return paths


def process_gperf_file(gperf_file, entities_file, output_file):
    if not find_executable("gperf"):
        raise Exception("Couldn't find `gperf`, is it installed?")

    # Do not rerun gperf if no change to input files
    if not newer_group((gperf_file, entities_file), output_file):
        return

    gperf = subprocess.Popen(["gperf", "--output-file", output_file],
                             stdin=subprocess.PIPE)

    # Send gperf template
    with open(gperf_file) as f:
        gperf.stdin.write(f.read())

    # Send entity list extracted from HTML5 spec JSON
    with open(entities_file) as f:
        for entity, entityinfo in sorted(json.load(f).items()):
            if not entity.endswith(';'):
                continue
            gperf.stdin.write(entity + "\n")

    # Wait for gperf to complete
    gperf.stdin.close()
    gperf.wait()
    if gperf.returncode != 0:
        raise subprocess.CalledProcessError(gperf.returncode, None, None)

version = None
version_re = re.compile(r'^#define\s+SNUDOWN_VERSION\s+"([^"]+)"$')
with open('snudown.c', 'r') as f:
    for line in f:
        m = version_re.match(line)
        if m:
            version = m.group(1)
assert version


class GPerfingBuildExt(build_ext):
    def run(self):
        process_gperf_file("src/html_entities.gperf", "src/html_entities.json",
                           "src/html_entities.h")
        build_ext.run(self)

setup(
    name='snudown',
    version=version,
    author='Vicent Marti',
    author_email='vicent@github.com',
    license='MIT',
    test_suite="test_snudown.test_snudown",
    cmdclass={'build_ext': GPerfingBuildExt,},
    ext_modules=[
        Extension(
            name='snudown',
            sources=['snudown.c'] + c_files_in('src/') + c_files_in('html/'),
            depends=['src/html_entities.h'],
            include_dirs=['src', 'html']
        )
    ],
)
