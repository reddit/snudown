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

def send_html_entities(entities_file, outfh, seen_entities):
    # Convert entity list from HTML5 spec JSON and send it to gperf
    with open(entities_file) as entitiesfh:
        for entity, entityinfo in sorted(json.load(entitiesfh).items()):
            if not entity.endswith(';'):
                continue
            if entity in seen_entities:
                continue
            seen_entities.add(entity)

            # Some sanity checks on the codepoints
            ncodepoints = len(entityinfo['codepoints'])
            assert ncodepoints <= 2 # MAX_ENTITY_CODEPOINTS
            for bad_range in [xrange(0, 9), xrange(11, 13), xrange(14, 32),
                              xrange(55296, 57344), xrange(65534, 65536)]:
                for codepoint in entityinfo['codepoints']:
                    assert codepoint not in bad_range
            codepoints = ",".join(str(x) for x in entityinfo['codepoints'])

            # Output the entity
            outfh.write("%s, %d, {%s}\n" % (entity, ncodepoints, codepoints))


def process_gperf_file(gperf_file, entities_file, output_file, force=False):
    if not find_executable("gperf"):
        raise Exception("Couldn't find `gperf`, is it installed?")

    # Do not rerun gperf if no change to input files
    if not force and not newer_group((gperf_file, entities_file), output_file):
        return

    # Combine HTML5 entity data into the gperf input file. HTML5
    # entities are translated to numeric entities at runtime, as
    # opposed to the entities already in the gperf file which are
    # output verbatim.
    gperf_temp_file = gperf_file + ".generated"
    seen_entities = set()
    found_separator = 0
    with open(gperf_temp_file, 'w') as outfh:
        with open(gperf_file) as f:
            for line in f:
                entity = line.strip()
                # gperf files are divided into three sections, divided
                # by `%%` lines. The first is for declarations, the
                # second is the list of keywords, and the third is for
                # functions (which are included verbatim in the
                # output). We want to add the HTML5 entities to the
                # end of the second section (i.e. right before the
                # third section).
                if entity == '%%':
                    found_separator += 1
                    if found_separator == 2:
                        send_html_entities(entities_file, outfh, seen_entities)
                elif found_separator == 1:
                    # Track the entities we've seen so far so that we
                    # don't repeat them.
                    entity = entity.split()[0]
                    seen_entities.add(entity)
                outfh.write(line)
            if found_separator < 2:
                # The gperf file contains no third section.
                send_html_entities(entities_file, outfh, seen_entities)

    subprocess.check_call(["gperf", gperf_temp_file,
                           "--output-file", output_file])

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
                           "src/html_entities.h", force=self.force)
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
