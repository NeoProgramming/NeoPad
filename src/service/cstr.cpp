#include "cstr.h"

unsigned char chrToHex(char c)
{
	if (c >= '0' && c <= '9')
		return c - '0';
	if (c >= 'A' && c <= 'F')
		return c - 'A' + 10;
	if (c >= 'a' && c <= 'f')
		return c - 'a' + 10;
	return 0;
}

unsigned long strToHex(const char *buf)
{
	unsigned long val = 0, d;
	while (*buf) {

		d = chrToHex(*buf);
		if (d == 0 && *buf != '0')
			break;
		val <<= 4;
		val |= d;

		buf++;
	}
	return val;
}

int IsBlank(char c)
{
	// whitespace character
	if(c == ' ' || c == '\t' || c == '\r' || c == '\n')
		return 1;
	return 0;
}

int IsBlank(const char *c)
{
	// determines if a line is empty, taking into account spaces, tabs, line breaks
	if(!c) return 1;
	while(*c)
	{
		// at least one non-empty string is non-empty
		if(!IsBlank(*c))
			return 0;
		c++;
	}
	// the string is empty
	return 1;
}
