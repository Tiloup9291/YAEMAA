#include "utilities.h"
#include <limits.h>
#include <errno.h>
#include <stdlib.h>

int char2DToInt(char *s, int *out){
	char *end;
	long val;

	errno = 0;
	val = strtol(s, &end, 10);

	if (errno != 0) return 1;
	if (end == s) return 1;
	if (*end != '\0') return 1;
	if (val < INT_MIN || val > INT_MAX) return 1;

	*out = (int)val;
	return 0;
}

int get_bit(uint16_t byte, int pos){
	return (byte >> pos) & 1;
}

void set_bit(uint16_t *byte, int pos){
	*byte |= (1 << pos);
}

void clear_bit(uint16_t *byte, int pos){
	*byte &= ~(1 << pos);
}
