For safety reasons, whenever you add or change something in Snudown,
you should add a few test-cases that demonstrate your change and do a
fuzzing run in `/fuzzing` by running `make afl`. Make sure you have `cmake`
installed and in your `PATH`!

This uses [American Fuzzy Lop](http://lcamtuf.coredump.cx/afl/) and a
modified [Google Gumbo](https://github.com/google/gumbo-parser/) to ensure
there is no way to generate invalid HTML, and that there are no unsafe
memory operations.

See [American Fuzzy Lop](http://lcamtuf.coredump.cx/afl/)'s instructions
for your platform to get started.
