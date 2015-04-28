/*
 * Copyright (c) 2011, Vicent Marti
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

#include "buffer.h"
#include "autolink.h"

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>

#if defined(_WIN32)
#define strncasecmp	_strnicmp
#endif

int
sd_autolink_issafe(const uint8_t *link, size_t link_len)
{
	static const size_t valid_uris_count = 14;
	static const char *valid_uris[] = {
		"http://", "https://", "ftp://", "mailto://",
		"/", "git://", "steam://", "irc://", "news://", "mumble://",
		"ssh://", "ircs://", "ts3server://", "#"
	};

	size_t i;

	for (i = 0; i < valid_uris_count; ++i) {
		size_t len = strlen(valid_uris[i]);

		if (link_len > len &&
			strncasecmp((char *)link, valid_uris[i], len) == 0 &&
			(isalnum(link[len]) || link[len] == '#' || link[len] == '/' || link[len] == '?'))
			return 1;
	}

	return 0;
}

static size_t
autolink_delim(uint8_t *data, size_t link_end, size_t max_rewind, size_t size)
{
	uint8_t cclose, copen = 0;
	size_t i;

	for (i = 0; i < link_end; ++i)
		if (data[i] == '<') {
			link_end = i;
			break;
		}

	while (link_end > 0) {
		uint8_t c = data[link_end - 1];

		if (c == 0)
			break;

		if (strchr("?!.,", c) != NULL)
			link_end--;

		else if (c == ';') {
			size_t new_end = link_end - 2;

			while (new_end > 0 && isalpha(data[new_end]))
				new_end--;

			if (new_end < link_end - 2 && data[new_end] == '&')
				link_end = new_end;
			else
				link_end--;
		}
		else break;
	}

	if (link_end == 0)
		return 0;

	cclose = data[link_end - 1];

	switch (cclose) {
	case '"':	copen = '"'; break;
	case '\'':	copen = '\''; break;
	case ')':	copen = '('; break;
	case ']':	copen = '['; break;
	case '}':	copen = '{'; break;
	}

	if (copen != 0) {
		size_t closing = 0;
		size_t opening = 0;
		size_t i = 0;

		/* Try to close the final punctuation sign in this same line;
		 * if we managed to close it outside of the URL, that means that it's
		 * not part of the URL. If it closes inside the URL, that means it
		 * is part of the URL.
		 *
		 * Examples:
		 *
		 *	foo http://www.pokemon.com/Pikachu_(Electric) bar
		 *		=> http://www.pokemon.com/Pikachu_(Electric)
		 *
		 *	foo (http://www.pokemon.com/Pikachu_(Electric)) bar
		 *		=> http://www.pokemon.com/Pikachu_(Electric)
		 *
		 *	foo http://www.pokemon.com/Pikachu_(Electric)) bar
		 *		=> http://www.pokemon.com/Pikachu_(Electric))
		 *
		 *	(foo http://www.pokemon.com/Pikachu_(Electric)) bar
		 *		=> foo http://www.pokemon.com/Pikachu_(Electric)
		 */

		while (i < link_end) {
			if (data[i] == copen)
				opening++;
			else if (data[i] == cclose)
				closing++;

			i++;
		}

		if (closing != opening)
			link_end--;
	}

	return link_end;
}

/*
 * Checks that `prefix_char` occurs on a word boundary just before `data`,
 * where `data` points to the character to search to the left of, and a word boundary
 * is (currently) a whitespace character, punctuation, or the start of the string.
 * Returns the length of the prefix.
 */
static int
check_reddit_autolink_prefix(
	const uint8_t* data,
	size_t max_rewind,
	size_t max_lookbehind,
	size_t size,
	char prefix_char
	)
{
	/* Make sure this `/` is part of `/?r/` */
	if (size < 2 || max_rewind < 1 || data[-1] != prefix_char)
		return 0;

	/* Not at the start of the buffer, no inlines to the immediate left of the `prefix_char` */
	if (max_rewind > 1) {
		const char boundary = data[-2];
		if (boundary == '/')
			return 2;
		/**
		 * Here's where our lack of unicode-awareness bites us. We don't correctly
		 * match punctuation / whitespace characters for the boundary, because we
		 * reject valid cases like "。r/example" (note the fullwidth period.)
		 *
		 * A better implementation might try to rewind over bytes with the 8th bit set, try
		 * to decode them to a valid codepoint, then do a unicode-aware check on the codepoint.
		 */
		else if (ispunct(boundary) || isspace(boundary))
			return 1;
		else
			return 0;
	} else if (max_lookbehind > 2) {
		/* There's an inline element just left of the `prefix_char`, is it an escaped forward
		 * slash? bail out so we correctly handle stuff like "\/r/foo". This will also correctly
		 * allow "\\/r/foo".
		 */
		if (data[-2] == '/' && data[-3] == '\\')
			return 0;
	}

	/* Must be a new-style shortlink with nothing relevant to the left of it. */
	return 1;
}

static size_t
check_domain(uint8_t *data, size_t size, int allow_short)
{
	size_t i, np = 0;

	if (!isalnum(data[0]))
		return 0;

	for (i = 1; i < size - 1; ++i) {
		if (data[i] == '.') np++;
		else if (!isalnum(data[i]) && data[i] != '-') break;
	}

	if (allow_short) {
		/* We don't need a valid domain in the strict sense (with
		 * least one dot; so just make sure it's composed of valid
		 * domain characters and return the length of the the valid
		 * sequence. */
		return i;
	} else {
		/* a valid domain needs to have at least a dot.
		 * that's as far as we get */
		return np ? i : 0;
	}
}

size_t
sd_autolink__www(
	size_t *rewind_p,
	struct buf *link,
	uint8_t *data,
	size_t max_rewind,
	size_t size,
	unsigned int flags)
{
	size_t link_end;

	if (max_rewind > 0 && !ispunct(data[-1]) && !isspace(data[-1]))
		return 0;

	if (size < 4 || memcmp(data, "www.", strlen("www.")) != 0)
		return 0;

	link_end = check_domain(data, size, 0);

	if (link_end == 0)
		return 0;

	while (link_end < size && !isspace(data[link_end]))
		link_end++;

	link_end = autolink_delim(data, link_end, max_rewind, size);

	if (link_end == 0)
		return 0;

	bufput(link, data, link_end);
	*rewind_p = 0;

	return (int)link_end;
}

size_t
sd_autolink__email(
	size_t *rewind_p,
	struct buf *link,
	uint8_t *data,
	size_t max_rewind,
	size_t size,
	unsigned int flags)
{
	size_t link_end, rewind;
	int nb = 0, np = 0;

	for (rewind = 0; rewind < max_rewind; ++rewind) {
		uint8_t c = data[-rewind - 1];

		if (c == 0)
			break;

		if (isalnum(c))
			continue;

		if (strchr(".+-_", c) != NULL)
			continue;

		break;
	}

	if (rewind == 0)
		return 0;

	for (link_end = 0; link_end < size; ++link_end) {
		uint8_t c = data[link_end];

		if (isalnum(c))
			continue;

		if (c == '@')
			nb++;
		else if (c == '.' && link_end < size - 1)
			np++;
		else if (c != '-' && c != '_')
			break;
	}

	if (link_end < 2 || nb != 1 || np == 0)
		return 0;

	link_end = autolink_delim(data, link_end, max_rewind, size);

	if (link_end == 0)
		return 0;

	bufput(link, data - rewind, link_end + rewind);
	*rewind_p = rewind;

	return link_end;
}

size_t
sd_autolink__url(
	size_t *rewind_p,
	struct buf *link,
	uint8_t *data,
	size_t max_rewind,
	size_t size,
	unsigned int flags)
{
	size_t link_end, rewind = 0, domain_len;

	if (size < 4 || data[1] != '/' || data[2] != '/')
		return 0;

	while (rewind < max_rewind && isalpha(data[-rewind - 1]))
		rewind++;

	if (!sd_autolink_issafe(data - rewind, size + rewind))
		return 0;

	link_end = strlen("://");

	domain_len = check_domain(
		data + link_end,
		size - link_end,
		flags & SD_AUTOLINK_SHORT_DOMAINS);

	if (domain_len == 0)
		return 0;

	link_end += domain_len;
	while (link_end < size && !isspace(data[link_end]))
		link_end++;

	link_end = autolink_delim(data, link_end, max_rewind, size);

	if (link_end == 0)
		return 0;

	bufput(link, data - rewind, link_end + rewind);
	*rewind_p = rewind;

	return link_end;
}

size_t
sd_autolink__subreddit(
	size_t *rewind_p,
	struct buf *link,
	uint8_t *data,
	size_t max_rewind,
	size_t max_lookbehind,
	size_t size,
	int *no_slash
	)
{
	/**
	 * This is meant to handle both r/foo and /r/foo style subreddit references.
	 * In a valid /?r/ link, `*data` will always point to the '/' after the first 'r'.
	 * In pseudo-regex, this matches something like:
	 *
	 * `(/|(?<=\b))r/(all-)?%subreddit%([-+]%subreddit%)*(/[\w\-/]*)?`
	 * where %subreddit% == `((t:)?\w{2,24}|reddit\.com)`
	 */
	size_t link_end;
	size_t rewind;
	int is_allminus = 0;

	rewind = check_reddit_autolink_prefix(data, max_rewind, max_lookbehind, size, 'r');
	if (!rewind)
		return 0;

	/* offset to the "meat" of the link */
	link_end = strlen("/");

	if (size >= link_end + 4 && strncasecmp((char*)data + link_end, "all-", 4) == 0)
		is_allminus = 1;

	do {
		size_t start = link_end;
		int max_length = 24;

		/* special case: /r/reddit.com (only subreddit containing '.'). */
		if ( size >= link_end+10 && strncasecmp((char*)data+link_end, "reddit.com", 10) == 0 ) {
			link_end += 10;
			/* Make sure there are no trailing characters (don't do
			 * any autolinking for /r/reddit.commission) */
			max_length = 10;
		}

		/* If not a special case, verify it begins with (t:)?[A-Za-z0-9] */
		else {
			/* support autolinking to timereddits, /r/t:when (1 April 2012) */
			if ( size > link_end+2 && strncasecmp((char*)data+link_end, "t:", 2) == 0 )
				link_end += 2;  /* Jump over the 't:' */

			/* the first character of a subreddit name must be a letter or digit */
			if (!isalnum(data[link_end]))
				return 0;
			link_end += 1;
		}

		/* consume valid characters ([A-Za-z0-9_]) until we run out */
		while (link_end < size && (isalnum(data[link_end]) ||
							data[link_end] == '_'))
			link_end++;

		/* valid subreddit names are between 3 and 21 characters, with
		 * some subreddits having 2-character names. Don't bother with
		 * autolinking for anything outside this length range.
		 * (chksrname function in reddit/.../validator.py) */
		if ( link_end-start < 2 || link_end-start > max_length )
			return 0;

		/* If we are linking to a multireddit, continue */
	} while ( link_end < size && (data[link_end] == '+' || (is_allminus && data[link_end] == '-')) && link_end++ );

	if (link_end < size && data[link_end] == '/') {
		while (link_end < size && (isalnum(data[link_end]) ||
									data[link_end] == '_' ||
									data[link_end] == '/' ||
									data[link_end] == '-'))
			link_end++;
	}

	/* make the link */
	bufput(link, data - rewind, link_end + rewind);

	*no_slash = (rewind == 1);
	*rewind_p = rewind;

	return link_end;
}

size_t
sd_autolink__username(
	size_t *rewind_p,
	struct buf *link,
	uint8_t *data,
	size_t max_rewind,
	size_t max_lookbehind,
	size_t size,
	int *no_slash
	)
{
	size_t link_end;
	size_t rewind;

	if (size < 3)
		return 0;

	rewind = check_reddit_autolink_prefix(data, max_rewind, max_lookbehind, size, 'u');
	if (!rewind)
		return 0;

	link_end = strlen("/");

	/* the first letter of a username must... well, be valid, we don't care otherwise */
	if (!isalnum(data[link_end]) && data[link_end] != '_' && data[link_end] != '-')
		return 0;
	link_end += 1;

	/* consume valid characters ([A-Za-z0-9_-/]) until we run out */
	while (link_end < size && (isalnum(data[link_end]) ||
								data[link_end] == '_' ||
								data[link_end] == '/' ||
								data[link_end] == '-'))
		link_end++;

	/* make the link */
	bufput(link, data - rewind, link_end + rewind);

	*no_slash = (rewind == 1);
	*rewind_p = rewind;

	return link_end;
}
