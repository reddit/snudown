from distutils.core import setup, Extension

import os
import fnmatch

def c_files_in(directory):
    paths = []
    names = os.listdir(directory)
    for f in fnmatch.filter(names, '*.c'):
        paths.append(os.path.join(directory, f))
    return paths

setup(
    name='snudown',
    version='1.0.5',
    author='Vicent Marti',
    author_email='vicent@github.com',
    license='MIT',
    ext_modules=[
        Extension(
            name='snudown', 
            sources=['snudown.c'] + c_files_in('src/') + c_files_in('html/'),
            include_dirs=['src', 'html']
        )
    ],
)
