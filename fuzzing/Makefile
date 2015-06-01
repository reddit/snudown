# Copyright (c) 2015, reddit inc.
#
# Permission to use, copy, modify, and distribute this software for any
# purpose with or without fee is hereby granted, provided that the above
# copyright notice and this permission notice appear in all copies.
#
# THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
# WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
# MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
# ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
# WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
# ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
# OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

all:		gumbo_snudown snudown-validator

.PHONY:		all clean gumbo_snudown snudown-validator build_dir

build_dir:
	mkdir -p build

# Our modified gumbo for finding security-relevant syntax issues
gumbo_snudown: build_dir
	mkdir -p build/gumbo_snudown
	git submodule update --recursive
	@[ -f "${CURDIR}/gumbo_snudown/configure" ] || { \
		cd gumbo_snudown; \
		./autogen.sh; \
		./configure --prefix=$(CURDIR)/build/gumbo_snudown; \
	}
	# Don't build this with AFL instrumentation, I'm assuming Google
	# already ran their own fuzzer over their own parser...
	$(MAKE) -C gumbo_snudown all install

gperf_src:
	cd ../src/ && gperf html_entities.gperf --output-file=html_entities.h

# executable
snudown-validator: build_dir gumbo_snudown gperf_src
	cd build && cmake .. -DCMAKE_C_COMPILER=$(AFL_PATH)/afl-gcc
	$(MAKE) -C build all

# stuff for fuzzing
gen_testcases:
	mkdir -p testing/testcases
	rm -f testing/testcases/test_default_*.md
	python2.7 gen_testcases.py

afl: gen_testcases snudown-validator
	@[ -n "$(AFL_PATH)" ] || { echo '$$AFL_PATH not set'; false; }
	@mkdir -p testing/afl_results
	$(AFL_PATH)/afl-fuzz \
	    -i testing/testcases \
	    -o testing/afl_results \
	    -t 100 \
	    -m none \
	    ./build/snudown-validator

# housekeeping
clean:
	rm -rf *.o
	rm -rf build/
