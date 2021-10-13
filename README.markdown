Snudown
=======

`Snudown` is a reddit-specific fork of the [Sundown](http://github.com/vmg/sundown)
Markdown parser used by GitHub, with Python integration added.


Setup for development on Mac OS X
--------------------------------
1. From `~/src/snudown` run `$ python setup.py build`
2. If this is successful, there will now be a `snudown.so` file in the `/snudown/build/lib.< os info >-< python version number>` directory
3. From within the `/lib.< os info >-< python version number>` directory, start a python interpreter
```
<!-- Make sure you can import snudown -->
>>> import snudown
<!-- verify that the build you just made is being used -->
>>> print(snudown.__file__)
snudown.so
<!-- Test the functionality of the build -->
>>> snudown.markdown('[hi](http://www.reddit.com)')
'<p><a href="http://www.reddit.com">hi</a></p>\n'
<!-- Great! You can exit now. -->
>>> quit()
```
4. Verify that the tests pass
```
$ PYTHONPATH="$(pwd)" python ../../test_snudown.py
```
5. Verify that all the previous steps work for both Python 2 AND Python 3


Install for general use
-----------------------

Run `setup.py install` to install the module.

For Mac OS X:
1. Install `afl-fuzz` via homebrew: `brew install afl-fuzz`
2. You can now install the module via `python setup.py install`
3. You may also compile snudown using the Makefile directly if you so wish


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
