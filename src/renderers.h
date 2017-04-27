/* renderers.h - header for renderers */

/*
* Copyright (c) 2017, Reddit
*
* Permission to use, copy, modify, and distribute this software for any
* purpose with or without fee is hereby granted, provided that the above
* copyright notice and this permission notice appear in all copies.
*
* THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
* WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
* MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
* ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
* WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
* ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
* OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
*/
#include "../html/html.h"

enum snudown_renderer_mode {
	RENDERER_USERTEXT = 0,
	RENDERER_WIKI,
	RENDERER_USERTEXT_WITHOUTLINKS,
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

static const unsigned int snudown_default_md_flags =
	MKDEXT_NO_INTRA_EMPHASIS |
	MKDEXT_SUPERSCRIPT |
	MKDEXT_AUTOLINK |
	MKDEXT_STRIKETHROUGH |
	MKDEXT_TABLES;

static const unsigned int snudown_default_md_flags_without_links =
	MKDEXT_NO_INTRA_EMPHASIS |
	MKDEXT_SUPERSCRIPT |
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
snudown_link_attr(struct buf *ob, const struct buf *link, void *opaque);

extern struct snudown_renderer *
get_default_renderer(void);

extern struct snudown_renderer *
get_wiki_renderer(void);

extern struct snudown_renderer *
get_default_renderer_without_links(void);