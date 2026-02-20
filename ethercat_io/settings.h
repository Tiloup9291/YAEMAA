#ifndef _SETTINGS_H_
#define _SETTINGS_H_

#define RUNDIR "/run/ethercat_io"
#define SOCKET_PATH RUNDIR "/control.sock"

#include <stdint.h>

typedef enum{
	READ = 1,
	WRITE = 2,
	STATUS = 3
} cmd_t;
typedef struct {
	int direction;
	int pdo;
	int index;
	int bit;
	int value;
} msg_t;
typedef struct {
	long duration;
	long last;
	uint16_t di0;
	uint16_t di1;
	uint16_t do0;
	uint16_t do1;
} payload_t;
typedef union{
	struct {
		uint8_t low;
		uint8_t high;
	};
	uint16_t full;
} dig_array_t;

#endif /* _SETTINGS_H_ */
