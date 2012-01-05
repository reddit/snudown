#define PY_SSIZE_T_CLEAN
#include <Python.h>

#include "markdown.h"
#include "html.h"
#include "autolink.h"

struct snudown_renderopt {
	struct html_renderopt html;
	int nofollow;
	const char *target;
};

struct module_state {
	struct sd_callbacks callbacks;
	struct snudown_renderopt options;
};

/* The module doc strings */
PyDoc_STRVAR(snudown_module__doc__, "When does the narwhal bacon? At Sundown.");
PyDoc_STRVAR(snudown_md__doc__, "Render a Markdown document");

static const int snudown_default_md_flags =
	MKDEXT_NO_INTRA_EMPHASIS |
	MKDEXT_SUPERSCRIPT |
	MKDEXT_AUTOLINK |
	MKDEXT_STRIKETHROUGH |
	MKDEXT_TABLES;

static const int snudown_default_render_flags = 
	HTML_SKIP_HTML |
	HTML_SKIP_IMAGES |
	HTML_SAFELINK |
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

static struct sd_markdown* default_render(struct module_state* state) {
	sdhtml_renderer(&state->callbacks,
		(struct html_renderopt *)&state->options,
		snudown_default_render_flags);
	
	state->options.html.link_attributes = &snudown_link_attr;
	
	return sd_markdown_new(
		snudown_default_md_flags,
		16,
		&state->callbacks,
		&state->options
	);
}

static PyObject *
snudown_md(PyObject *self, PyObject *args, PyObject *kwargs)
{
	static char *kwlist[] = {"text", "nofollow", "target", NULL};

	struct buf ib, *ob;
	PyObject *py_result;
	const char* result_text;
	struct module_state state;
	struct sd_markdown* sundown = default_render(&state);

	memset(&ib, 0x0, sizeof(struct buf));

	/* Parse arguments */
	if (!PyArg_ParseTupleAndKeywords(args, kwargs, "s#|iz", kwlist,
				&ib.data, &ib.size, &state.options.nofollow, &state.options.target)) {
		return NULL;
	}

	/* Output buffer */
	ob = bufnew(128);

	/* do the magic */
	Py_BEGIN_ALLOW_THREADS
	sd_markdown_render(ob, ib.data, ib.size, sundown);
	Py_END_ALLOW_THREADS
	
	free(sundown);
	
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

	/* Version */
	PyModule_AddStringConstant(module, "__version__", "1.0.2");
}
