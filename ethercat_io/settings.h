#ifndef _SETTINGS_H_
#define _SETTINGS_H_

#define RUNDIR "/run/ethercat_io"
#define SOCKET_PATH RUNDIR "/control.sock"

#include <stdint.h>
#include <stdatomic.h>

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
	atomic_bool di0_written;
	atomic_bool di0_reading;
	atomic_bool do0_written;
	atomic_bool do0_reading;
	atomic_bool stat_written;
	atomic_bool stat_reading;
	long duration;
	long last;
	uint16_t di0;
	uint16_t do0;
} payload_t;
typedef union{
	struct {
		uint8_t low;
		uint8_t high;
	};
	uint16_t full;
} dig_array_t;

extern payload_t producer;

#endif /* _SETTINGS_H_ */
