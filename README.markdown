Snudown
=======

`Snudown` is a reddit-specific fork of the [Sundown](http://github.com/vmg/sundown)
Markdown parser used by GitHub, with Python integration added.


Setup for development on Mac OS X
--------------------------------
1. From `~/src/snudown` run `$ python setup.py develop`
2. If this is successful, there will now be a `snudown.so` file in the `/snudown/build/lib.< os info >-< python version number>` directory
3. Verify that the tests pass
```
$ python test_snudown.py
```
4. Verify that all the previous steps work for both Python 2 AND Python 3


Install for general use
-----------------------

Run `setup.py install` to install the module.

Building for production use
---------------------------

* Merge your approved PR into `master`
* Create a github version with the associated tag, ex. `v1.7.2`
* Let drone build and upload python linux packages to internal pypi registry

Building for local dev use
---------------------------

For MacOS builds:
* Run `make build-macos` to build the MacOS wheels
* Upload the wheels (in `dist/`) to internal dev pypi registry
* Your MacOS build should now be available for local development.


Thanks
------

Many thanks to @vmg for implementing the initial version of this fork!


License
-------

Permission to use, copy, modify, and distribute this software for any
purpose with or without fee is hereby granted, provided that the above
copyright notice and this permission notice appear in all copies.

THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
