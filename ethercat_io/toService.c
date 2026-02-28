#include "toService.h"
#include "settings.h"
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

int toService(int command, int pdo, int index, int bit, int value){
	int fd;
	struct sockaddr_un addr;
	fd = socket(AF_UNIX, SOCK_STREAM, 0);
	if ( fd < 0 ) {
		perror("utility socket\n");
		return 1;
	}
	memset(&addr, 0, sizeof(addr));
	addr.sun_family = AF_UNIX;
	strncpy(addr.sun_path, SOCKET_PATH, sizeof(addr.sun_path) -1);
	if (connect(fd, (struct sockaddr*)&addr, sizeof(addr)) < 0){
		perror("utility connect");
		close(fd);
		return 1;
	}


	msg_t cmd = {
		.direction = command,
		.pdo = pdo,
		.index = index,
		.bit = bit,
		.value = value
	};
	write(fd,&cmd,sizeof(cmd));
	
	char buf[100];
	int n = read(fd, buf, sizeof(buf)-1);
	if ( n > 0 ){
		buf[n] = 0;
		printf("%s", buf);
	}
	close(fd);
	return 0;
}
