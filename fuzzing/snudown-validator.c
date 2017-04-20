#include "markdown.h"
#include "html.h"
#include "buffer.h"
#include "../src/renderers.h"

#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <unistd.h>

#include <gumbo.h>

#define READ_UNIT 1024
#define OUTPUT_UNIT 64

#include "autolink.h"

#define SNUDOWN_VERSION "1.3.2"

void
snudown_md(struct buf *ob, const uint8_t *document, size_t doc_size, int wiki_mode)
{
	int renderer = RENDERER_USERTEXT;
	int enable_toc = 0;
	struct snudown_renderer _snudown;
	int nofollow = 0;
	char* target = NULL;
	char* toc_id_prefix = NULL;
	unsigned int flags;

	if (wiki_mode)
		renderer = RENDERER_WIKI;

	_snudown = sundown[renderer];

	struct snudown_renderopt *options = &(_snudown.state->options);
	options->nofollow = nofollow;
	options->target = target;

	flags = options->html.flags;

	if (enable_toc) {
		_snudown.toc_state->options.html.toc_id_prefix = toc_id_prefix;
		sd_markdown_render(ob, document, doc_size, _snudown.toc_renderer);
		_snudown.toc_state->options.html.toc_id_prefix = NULL;

		options->html.flags |= HTML_TOC;
	}

	options->html.toc_id_prefix = toc_id_prefix;

	/* do the magic */
	sd_markdown_render(ob, document, doc_size, _snudown.main_renderer);

	options->html.toc_id_prefix = NULL;
	options->html.flags = flags;
}
int
main(int argc, char **argv)
{
	init_default_renderer();
	init_wiki_renderer();
	init_default_renderer_without_links();

	struct buf *ib, *ob;
	int size_read = 0, wiki_mode = 0, i = 0, have_errors = 0;

	/* reading everything */
	ib = bufnew(READ_UNIT);
	bufgrow(ib, READ_UNIT);
	while ((size_read = fread(ib->data + ib->size, 1, ib->asize - ib->size, stdin)) > 0) {
		ib->size += size_read;
		bufgrow(ib, ib->size + READ_UNIT);
	}
	/* Render to a buffer, then print that out */
	ob = bufnew(OUTPUT_UNIT);
	bufputs(ob, "<!DOCTYPE html><html><body>\n");
	snudown_md(ob, ib->data, ib->size, wiki_mode);
	bufputs(ob, "</body></html>\n");

	// Wiki mode explicitly allows unbalanced tags, need some way to exclude those
	if (!wiki_mode) {
		GumboOutput* output = gumbo_parse_with_options(&kGumboDefaultOptions, bufcstr(ob), ob->size);

		for (i=0; i < output->errors.length; ++i) {
			// stupid "public" API I hacked in.
			void* thing = output->errors.data[i];
			GumboErrorType type = gumbo_get_error_type(thing);
			switch(type) {
				case GUMBO_ERR_UTF8_INVALID:
				case GUMBO_ERR_UTF8_NULL:
					// Making sure the user gave us valid
					// utf-8 or transforming it to valid
					// utf-8 is outside the scope of snudown
					continue;
				default:
					have_errors = 1;
					printf("%s\n", GUMBO_ERROR_NAMES[type]);
					printf("%s\n",gumbo_get_error_text(thing));
					printf("===============\n");
					break;
			}
		}

		if (have_errors) {
			// gotta trigger a crash for AFL to catch it
			assert(0);
		}

		gumbo_destroy_output(&kGumboDefaultOptions, output);
	}
	bufrelease(ob);
	bufrelease(ib);
	return 0;
}
