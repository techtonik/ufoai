/**
 * @file
 * @brief Shared parsing functions.
 */

/*
All original material Copyright (C) 2002-2011 UFO: Alien Invasion.

Copyright (C) 1997-2001 Id Software, Inc.

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

*/

#include "parse.h"
#include "defines.h"
#include "ufotypes.h"

static char com_token[4096];
static bool isUnparsedToken;
static Com_TokenType_t type;

/**
 * @brief Put back the last token into the parser
 * The next call of Com_Parse will return the same token again
 * @note Only allow to use it one time
 * @note With it, we can't read to file at the same time
 */
void Com_UnParseLastToken (void)
{
	isUnparsedToken = true;
}

/**
 * @brief Get the current token value
 * @return The current token value
 */
const char *Com_GetToken(const char **data_p)
{
	return com_token;
}

/**
 * @brief Get the current token type
 * @return The current token type
 */
Com_TokenType_t Com_GetType(const char **data_p)
{
	return type;
}

/**
 * @brief Compute the next token
 * @return Type of the next token
 */
Com_TokenType_t Com_NextToken(const char **data_p)
{
	Com_Parse(data_p);
	return type;
}

/**
 * @brief Counts the tokens in the given buffer that the Com_Parse function would extract
 * @param[in] buffer The buffer to count the tokens in
 * @return The amount of tokens in the given buffer
 */
int Com_CountTokensInBuffer (const char *buffer)
{
	const char *text = buffer;
	int n = 0;
	for (;;) {
		Com_Parse(&text);
		if (!text)
			break;
		n++;
	}
	return n;
}

/**
 * @brief Parse a token out of a string
 * @param data_p Pointer to a string which is to be parsed
 * @param target A place to put the parsed data. If 0, an internal buffer of 4096 is used.
 * @param size The length of target buffer, if any
 * @pre @c data_p is expected to be null-terminated
 * @return The string result of parsing in a string.
 * @note if used to parse a quoted string that needs further parsing
 * the returned string must be copied first, as Com_Parse returns a
 * pointer to a static buffer that it holds. this will be re-used on
 * the next call.
 * @sa Com_EParse
 */
const char *Com_Parse (const char *data_p[], char *target, size_t size)
{
	char c;
	size_t len;
	const char *data;

	if (!target) {
		target = com_token;
		size = sizeof(com_token);
	}

	if (isUnparsedToken) {
		isUnparsedToken = false;
		return target;
	}

	data = *data_p;
	len = 0;
	target[0] = 0;

	if (!data) {
		*data_p = NULL;
		type = TT_EOF;
		return "";
	}

	/* skip whitespace */
skipwhite:
	while ((c = *data) <= ' ') {
		if (c == 0) {
			*data_p = NULL;
			type = TT_EOF;
			return "";
		}
		data++;
	}

	if (c == '/' && data[1] == '*') {
		int clen = 0;
		data += 2;
		while (!((data[clen] && data[clen] == '*') && (data[clen + 1] && data[clen + 1] == '/'))) {
			clen++;
		}
		data += clen + 2; /* skip end of multiline comment */
		goto skipwhite;
	}

	/* skip // comments */
	if (c == '/' && data[1] == '/') {
		while (*data && *data != '\n')
			data++;
		goto skipwhite;
	}

	/* handle quoted strings specially */
	if (c == '\"') {
		data++;
		for (;;) {
			c = *data++;
			if (c == '\\' && data[0] == 'n') {
				c = '\n';
				data++;
			} else if (c == '\\' && data[0] == 't') {
				c = '\t';
				data++;
			/* nested quotation */
			} else if (c == '\\' && data[0] == '\"') {
				c = '\"';
				data++;
			} else if (c == '\"' || !c) {
				target[len] = '\0';
				*data_p = data;
				type = TT_QUOTED_WORD;
				return target;
			} else if (c == '\0') {
				// TODO here the token is wrongly formed
				break;
			}

			if (len < size) {
				if (c != '\r') {
					target[len] = c;
					len++;
				}
			} else {
				/* exceeded com_token size */
				break;
			}
		}

		if (len == size) {
			len = 0;
		}

		// TODO here the token is wrongly formed
		target[len] = '\0';
		*data_p = data;
		type = TT_QUOTED_WORD;
		return target;
	}

	if (c == '{' || c == '}' || c == '(' || c == ')' || c == ',') {
		data++;
		target[len] = c;
		target[len + 1] = '\0';
		// Com_TokenType_t contains expected values for this set of characters
		type = (Com_TokenType_t) c;
		len++;
		*data_p = data;
		return target;
	}

	/* parse a regular word */
	do {
		if (len < size) {
			target[len] = c;
			len++;
		} else {
			/* exceeded com_token size */
			break;
		}
		data++;
		c = *data;
		if (c == '{' || c == '}' || c == '(' || c == ')' || c == ',')
			break;
	} while (c > 32);

	if (len == size) {
		len = 0;
	}
	target[len] = '\0';

	*data_p = data;
	type = TT_WORD;
	return target;
}
