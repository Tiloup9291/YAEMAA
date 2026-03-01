#ifndef _UTILITIES_H_
#define _UTILITIES_H_
#include <stdint.h>

int char2DToInt(char *s, int *out);
int get_bit(uint16_t byte, int pos);
void set_bit(uint16_t *byte, int pos);
void clear_bit(uint16_t *byte, int pos);

#endif /* _UTILITIES_H_ */
