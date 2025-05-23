from setuptools import setup, Extension
from setuptools.command.build_ext import build_ext
import sysconfig

try:
    from distutils.spawn import find_executable
except ImportError:
    from shutil import which as find_executable

import os
import subprocess
import fnmatch


def c_files_in(directory):
    paths = []
    names = os.listdir(directory)
    for f in fnmatch.filter(names, '*.c'):
        paths.append(os.path.join(directory, f))
    return paths


def process_gperf_file(gperf_file, output_file):
    if not find_executable("gperf"):
        raise Exception("Couldn't find `gperf`, is it installed?")
    subprocess.check_call(["gperf", gperf_file, "--output-file=%s" % output_file])


# version is defined in environment
version = os.environ.get("SNUDOWN_VERSION", "0.0.0")
if version.startswith("v"):
    version = version[1:]

# make sure the version string is escaped properly
extra_compile_args = ["-DSNUDOWN_VERSION=\"\\\"%s\\\"\"" % version]
extra_compile_args.extend(sysconfig.get_config_var('CFLAGS').split())


class GPerfingBuildExt(build_ext):
    def run(self):
        process_gperf_file("src/html_entities.gperf", "src/html_entities.h")
        build_ext.run(self)


setup(
    name='snudown',
    version=version,
    author='Vicent Marti',
    author_email='vicent@github.com',
    license='MIT',
    test_suite="test_snudown.test_snudown",
    cmdclass={'build_ext': GPerfingBuildExt},
    ext_modules=[
        Extension(
            name='snudown',
            sources=['snudown.c'] + c_files_in('src/') + c_files_in('html/'),
            include_dirs=['src', 'html'],
            extra_compile_args=extra_compile_args,
        )
    ],
)
