#include "markdown.h"
#include "html.h"
#include "buffer.h"

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

enum snudown_renderer_mode {
	RENDERER_USERTEXT = 0,
	RENDERER_WIKI,
	RENDERER_COUNT
};

struct snudown_renderopt {
	struct html_renderopt html;
	int nofollow;
	const char *target;
};

struct snudown_renderer {
	struct sd_markdown* main_renderer;
	struct sd_markdown* toc_renderer;
	struct module_state* state;
	struct module_state* toc_state;
};

struct module_state {
	struct sd_callbacks callbacks;
	struct snudown_renderopt options;
};

static struct snudown_renderer sundown[RENDERER_COUNT];

static char* html_element_whitelist[] = {"tr", "th", "td", "table", "tbody", "thead", "tfoot", "caption", NULL};
static char* html_attr_whitelist[] = {"colspan", "rowspan", "cellspacing", "cellpadding", "scope", NULL};

static struct module_state usertext_toc_state;
static struct module_state wiki_toc_state;
static struct module_state usertext_state;
static struct module_state wiki_state;

static const unsigned int snudown_default_md_flags =
	MKDEXT_NO_INTRA_EMPHASIS |
	MKDEXT_SUPERSCRIPT |
	MKDEXT_AUTOLINK |
	MKDEXT_STRIKETHROUGH |
	MKDEXT_TABLES;

static const unsigned int snudown_default_render_flags =
	HTML_SKIP_HTML |
	HTML_SKIP_IMAGES |
	HTML_SAFELINK |
	HTML_ESCAPE |
	HTML_USE_XHTML;

static const unsigned int snudown_wiki_render_flags =
	HTML_SKIP_HTML |
	HTML_SAFELINK |
	HTML_ALLOW_ELEMENT_WHITELIST |
	HTML_ESCAPE |
	HTML_USE_XHTML;

static void
snudown_link_attr(struct buf *ob, const struct buf *link, void *opaque)
{
	struct snudown_renderopt *options = opaque;

	if (options->nofollow)
		BUFPUTSL(ob, " rel=\"nofollow\"");

	if (options->target != NULL) {
		BUFPUTSL(ob, " target=\"");
		bufputs(ob, options->target);
		bufputc(ob, '\"');
	}
}

static struct sd_markdown* make_custom_renderer(struct module_state* state,
												const unsigned int renderflags,
												const unsigned int markdownflags,
												int toc_renderer) {
	if(toc_renderer) {
		sdhtml_toc_renderer(&state->callbacks,
			(struct html_renderopt *)&state->options);
	} else {
		sdhtml_renderer(&state->callbacks,
			(struct html_renderopt *)&state->options,
			renderflags);
	}

	state->options.html.link_attributes = &snudown_link_attr;
	state->options.html.html_element_whitelist = html_element_whitelist;
	state->options.html.html_attr_whitelist = html_attr_whitelist;

	return sd_markdown_new(
		markdownflags,
		16,
		64,
		&state->callbacks,
		&state->options
	);
}

void init_default_renderer() {
	sundown[RENDERER_USERTEXT].main_renderer = make_custom_renderer(&usertext_state, snudown_default_render_flags, snudown_default_md_flags, 0);
	sundown[RENDERER_USERTEXT].toc_renderer = make_custom_renderer(&usertext_toc_state, snudown_default_render_flags, snudown_default_md_flags, 1);
	sundown[RENDERER_USERTEXT].state = &usertext_state;
	sundown[RENDERER_USERTEXT].toc_state = &usertext_toc_state;
}

void init_wiki_renderer() {
	sundown[RENDERER_WIKI].main_renderer = make_custom_renderer(&wiki_state, snudown_wiki_render_flags, snudown_default_md_flags, 0);
	sundown[RENDERER_WIKI].toc_renderer = make_custom_renderer(&wiki_toc_state, snudown_wiki_render_flags, snudown_default_md_flags, 1);
	sundown[RENDERER_WIKI].state = &wiki_state;
	sundown[RENDERER_WIKI].toc_state = &wiki_toc_state;
}

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
