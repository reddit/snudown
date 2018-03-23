#include <stdarg.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>
#include "buffer.h"

/* MSVC compat */
#if defined(_MSC_VER)
#	define _buf_vsnprintf _vsnprintf
#else
#	define _buf_vsnprintf vsnprintf
#endif


/* bufprintf: formatted printing to a buffer */
void
bufprintf(struct buf *buf, const char *fmt, ...)
{
	va_list ap;
	int n;

	assert(buf && buf->unit);

	if (buf->size >= buf->asize && bufgrow(buf, buf->size + 1) < 0)
		return;
	va_start(ap, fmt);
	n = _buf_vsnprintf((char *)buf->data + buf->size, buf->asize - buf->size, fmt, ap);
	va_end(ap);

	if (n < 0) {
#ifdef _MSC_VER
		va_start(ap, fmt);
		n = _vscprintf(fmt, ap);
		va_end(ap);
#else
		return;
#endif
	}
	if ((size_t)n >= buf->asize - buf->size) {
		if (bufgrow(buf, buf->size + n + 1) < 0)
			return;

		va_start(ap, fmt);
		n = _buf_vsnprintf((char *)buf->data + buf->size, buf->asize - buf->size, fmt, ap);
		va_end(ap);
	}

	if (n < 0)
		return;

	buf->size += n;
}
