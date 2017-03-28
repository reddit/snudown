#include "markdown.h"
#include "../html/html.h"
#include "renderers.h"

static struct module_state usertext_toc_state;
static struct module_state wiki_toc_state;
static struct module_state usertext_state;
static struct module_state wiki_state;

static struct sd_markdown *make_custom_renderer(struct module_state *state,
	const unsigned int renderflags,
	const unsigned int markdownflags,
	int toc_renderer) {
	if (toc_renderer) {
		sdhtml_toc_renderer(&state->callbacks,
			(struct html_renderopt *)&state->options);
	}
	else {
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

struct snudown_renderer *get_default_renderer() {
	struct snudown_renderer *sr = NULL;

	sr = calloc(1, sizeof(struct snudown_renderer));

	if (!sr)
		return NULL;

	sr->main_renderer = make_custom_renderer(&usertext_state, snudown_default_render_flags, snudown_default_md_flags, 0);
	sr->toc_renderer = make_custom_renderer(&usertext_toc_state, snudown_default_render_flags, snudown_default_md_flags, 1);
	sr->state = &usertext_state;
	sr->toc_state = &usertext_toc_state;

	return sr;
}

struct snudown_renderer *get_wiki_renderer() {
	struct snudown_renderer *sr = NULL;

	sr = calloc(1, sizeof(struct snudown_renderer));

	if (!sr)
		return NULL;

	sr->main_renderer = make_custom_renderer(&wiki_state, snudown_wiki_render_flags, snudown_default_md_flags, 0);
	sr->toc_renderer = make_custom_renderer(&wiki_toc_state, snudown_wiki_render_flags, snudown_default_md_flags, 1);
	sr->state = &wiki_state;
	sr->toc_state = &wiki_toc_state;

	return sr;
}

struct snudown_renderer *get_default_renderer_without_links() {
	struct snudown_renderer *sr = NULL;
	
	sr = calloc(1, sizeof(struct snudown_renderer));

	if (!sr)
		return NULL;

	sr->main_renderer = make_custom_renderer(&usertext_state, snudown_default_render_flags, snudown_default_md_flags_without_links, 0);
	sr->toc_renderer = make_custom_renderer(&usertext_toc_state, snudown_default_render_flags, snudown_default_md_flags_without_links, 1);
	sr->state = &usertext_state;
	sr->toc_state = &usertext_toc_state;

	return sr;
}

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