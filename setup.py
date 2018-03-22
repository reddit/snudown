from distutils.spawn import find_executable
from distutils.cmd import Command
from setuptools import setup, Extension
from setuptools.command.build_ext import build_ext

import re
import os
import platform
import subprocess
import fnmatch
import distutils.command.build

# Change these to the correct paths
c2rust_path     = os.path.realpath(os.path.join(os.getcwd(), "..", "..", ".."))
plugin_path     = c2rust_path + "/cross-checks/c-checks/clang-plugin/build/plugin/CrossChecks.so"
runtime_path    = c2rust_path + "/cross-checks/c-checks/clang-plugin/build/runtime/libruntime.a"
fakechecks_path = c2rust_path + "/cross-checks/libfakechecks"
clevrbuf_path   = c2rust_path + "/cross-checks/ReMon/libclevrbuf"

os.environ["CC"] = c2rust_path + "/dependencies/llvm-6.0.0/build.{}/bin/clang".format(platform.uname()[1])
plugin_args = ['-Xclang', '-load',
               '-Xclang', plugin_path,
               '-Xclang', '-add-plugin',
               '-Xclang', 'crosschecks',
               '-Xclang', '-plugin-arg-crosschecks',
               '-Xclang', '-C../snudown_c.c2r',
               '-ffunction-sections', # Used by --icf
               ]

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

'''
extensions[0] -> extension for translating to rust
extensions[1] -> extension for translating and running rust xcheck
extensions[2] -> extension for running clang xcheck
'''
extensions = [
    Extension(
        name='snudown',
        sources=['snudown.c', 'src/bufprintf.c'] + c_files_in('html/'),
        include_dirs=['src', 'html'],
        libraries=['snudownrust'],
        library_dirs=['translator-build']
    ),
    Extension(
        name='snudown',
        sources=['snudown.c', 'src/bufprintf.c'] + c_files_in('html/'),
        include_dirs=['src', 'html'],
        libraries=['snudownrustxcheck', 'fakechecks'],
        #libraries=['snudownrustxcheck', 'clevrbuf'],
        library_dirs=['translator-build', fakechecks_path, clevrbuf_path],
        extra_link_args=['-Wl,-rpath,{},-rpath,{}'.format(fakechecks_path, clevrbuf_path)],
    ),
    Extension(
        name='snudown',
        sources=['snudown.c', '../xchecks.c'] + c_files_in('src/') + c_files_in('html/'),
        include_dirs=['src', 'html'],
        library_dirs=[fakechecks_path, clevrbuf_path],
        libraries=["fakechecks"],
        #libraries=["clevrbuf"],
        extra_compile_args=plugin_args,
        extra_link_args=['-fuse-ld=gold', '-Wl,--gc-sections,--icf=safe',
                            '-Wl,-rpath,{},-rpath,{}'.format(fakechecks_path, clevrbuf_path)],
        extra_objects=[runtime_path],
    )
]

version = None
version_re = re.compile(r'^#define\s+SNUDOWN_VERSION\s+"([^"]+)"$')
with open('snudown.c', 'r') as f:
    for line in f:
        m = version_re.match(line)
        if m:
            version = m.group(1)
assert version

class BuildSnudown(distutils.command.build.build):
    user_options = distutils.command.build.build.user_options + [
    ('translate', None,
    'translate from c to rust'),
    ('rust-crosschecks', None,
    'translate then run rust crosschecks'),
    ('clang-crosschecks', None,
    'translate then run rust crosschecks'),
    ]

    def initialize_options(self, *args, **kwargs):
        self.translate = self.rust_crosschecks = self.clang_crosschecks = None
        distutils.command.build.build.initialize_options(self, *args, **kwargs)

    def run(self, *args, **kwargs):
        if self.translate is not None:
            subprocess.check_call(["../translate.sh", "translate"])
            del extensions[1]
            del extensions[1]
        if self.rust_crosschecks is not None:
            subprocess.check_call(["../translate.sh", "rustcheck"])
            del extensions[0]
            del extensions[1]
        if self.clang_crosschecks is not None:
            subprocess.check_call(["../translate.sh"])
            del extensions[0]
            del extensions[0]
        distutils.command.build.build.run(self, *args, **kwargs)

class GPerfingBuildExt(build_ext):
    def run(self):
        #translate.py builds this manually
        #process_gperf_file("src/html_entities.gperf", "src/html_entities.h")
        build_ext.run(self)

setup(
    name='snudown',
    version=version,
    author='Vicent Marti',
    author_email='vicent@github.com',
    license='MIT',
    test_suite="test_snudown.test_snudown",
    cmdclass={'build': BuildSnudown, 'build_ext': GPerfingBuildExt},
    ext_modules=extensions,
)
