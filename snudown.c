#define PY_SSIZE_T_CLEAN
#include <Python.h>

#include "markdown.h"
#include "autolink.h"
#include "snudown.h"

#define SNUDOWN_VERSION "1.4.0"

/* The module doc strings */
PyDoc_STRVAR(snudown_module__doc__, "When does the narwhal bacon? At Sundown.");
PyDoc_STRVAR(snudown_md__doc__, "Render a Markdown document");

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

void init_default_renderer_without_links() {
	sundown[RENDERER_USERTEXT_WITHOUTLINKS].main_renderer = make_custom_renderer(&usertext_state, snudown_default_render_flags, snudown_default_md_flags_without_links, 0);
	sundown[RENDERER_USERTEXT_WITHOUTLINKS].toc_renderer = make_custom_renderer(&usertext_toc_state, snudown_default_render_flags, snudown_default_md_flags_without_links, 1);
	sundown[RENDERER_USERTEXT_WITHOUTLINKS].state = &usertext_state;
	sundown[RENDERER_USERTEXT_WITHOUTLINKS].toc_state = &usertext_toc_state;
}

void register_default_renderer(PyObject *module) {
	PyModule_AddIntConstant(module, "RENDERER_USERTEXT", RENDERER_USERTEXT);
	init_default_renderer();
}

void register_wiki_renderer(PyObject *module) {
	PyModule_AddIntConstant(module, "RENDERER_WIKI", RENDERER_WIKI);
	init_wiki_renderer();
}

void register_default_renderer_without_links(PyObject *module) {
	PyModule_AddIntConstant(module, "RENDERER_USERTEXT_WITHOUTLINKS", RENDERER_USERTEXT_WITHOUTLINKS);
	init_default_renderer_without_links();
}

static PyObject *
snudown_md(PyObject *self, PyObject *args, PyObject *kwargs)
{
	static char *kwlist[] = {"text", "nofollow", "target", "toc_id_prefix", "renderer", "enable_toc", NULL};

	struct buf ib, *ob;
	PyObject *py_result;
	const char* result_text;
	int renderer = RENDERER_USERTEXT;
	int enable_toc = 0;
	struct snudown_renderer _snudown;
	int nofollow = 0;
	char* target = NULL;
	char* toc_id_prefix = NULL;
	unsigned int flags;

	memset(&ib, 0x0, sizeof(struct buf));

	/* Parse arguments */
	if (!PyArg_ParseTupleAndKeywords(args, kwargs, "s#|izzii", kwlist,
				&ib.data, &ib.size, &nofollow,
				&target, &toc_id_prefix, &renderer, &enable_toc)) {
		return NULL;
	}

	if (renderer < 0 || renderer >= RENDERER_COUNT) {
		PyErr_SetString(PyExc_ValueError, "Invalid renderer");
		return NULL;
	}

	_snudown = sundown[renderer];

	struct snudown_renderopt *options = &(_snudown.state->options);
	options->nofollow = nofollow;
	options->target = target;

	/* Output buffer */
	ob = bufnew(128);

	flags = options->html.flags;

	if (enable_toc) {
		_snudown.toc_state->options.html.toc_id_prefix = toc_id_prefix;
		sd_markdown_render(ob, ib.data, ib.size, _snudown.toc_renderer);
		_snudown.toc_state->options.html.toc_id_prefix = NULL;

		options->html.flags |= HTML_TOC;
	}

	options->html.toc_id_prefix = toc_id_prefix;

	/* do the magic */
	sd_markdown_render(ob, ib.data, ib.size, _snudown.main_renderer);

	options->html.toc_id_prefix = NULL;
	options->html.flags = flags;

	/* make a Python string */
	result_text = "";
	if (ob->data)
		result_text = (const char*)ob->data;
	py_result = Py_BuildValue("s#", result_text, (int)ob->size);

	/* Cleanup */
	bufrelease(ob);
	return py_result;
}

static PyMethodDef snudown_methods[] = {
	{"markdown", (PyCFunction) snudown_md, METH_VARARGS | METH_KEYWORDS, snudown_md__doc__},
	{NULL, NULL, 0, NULL} /* Sentinel */
};

PyMODINIT_FUNC initsnudown(void)
{
	PyObject *module;

	module = Py_InitModule3("snudown", snudown_methods, snudown_module__doc__);
	if (module == NULL)
		return;

	register_default_renderer(module);
	register_wiki_renderer(module);
	register_default_renderer_without_links(module);

	/* Version */
	PyModule_AddStringConstant(module, "__version__", SNUDOWN_VERSION);
}