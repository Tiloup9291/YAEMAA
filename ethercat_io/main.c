#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "toService.h"
#include "service.h"
#include "signalHandler.h"
#include "settings.h"
#include "utilities.h"

int main(int argc, char **argv){
	if (argc == 2 && !strcmp(argv[1], "--service") ){
		return service();
	}

	if (argc < 2){
		printf("Usage: ethercat_ioctl <start|stop|status|live|read [pdo] [subindex] [position]|write [pdo] [subindex] [position] [value]>\n");
		return 1;
	}
	
	if (argc == 2 && !strcmp(argv[1], "start")){
		return system("systemctl start ethercat_io.service");
	}else if (argc == 2 && !strcmp(argv[1], "stop")){
		return system("systemctl stop ethercat_io.service");
	}else if (argc == 2 && !strcmp(argv[1], "status")){
		return system("systemctl status ethercat_io.service");
	}else if (argc == 5 && !strcmp(argv[1], "read")){
		int pdo, subindex, position, value, err;
		err = char2DToInt(argv[2], &pdo);
		if (err != 0) return err;
		err = char2DToInt(argv[3], &subindex);
		if (err != 0) return err;
		err = char2DToInt(argv[4], &position);
		if (err != 0) return err;
		return toService(READ, pdo, subindex, position, 0);		
	}else if (argc == 6 && !strcmp(argv[1], "write")){
		int pdo, subindex, position, value, err;
		err = char2DToInt(argv[2], &pdo);
		if (err != 0) return err;
		err = char2DToInt(argv[3], &subindex);
		if (err != 0) return err;
		err = char2DToInt(argv[4], &position);
		if (err != 0) return err;
		err = char2DToInt(argv[5],&value);
		if (err != 0) return err;
		return toService(WRITE, pdo, subindex, position, value);
	}else if (argc == 2 && !strcmp(argv[1], "live")){
		return toService(STATUS, 0, 0, 0, 0);
	}else{
		printf("Unknown command\n");
	}
	return 0;
}
