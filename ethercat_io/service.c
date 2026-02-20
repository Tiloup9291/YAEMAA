#include "service.h"
#include "signalHandler.h"
#include "settings.h"
#include "oobThread.h"
#include "flags.h"
#include "utilities.h"
#include <sys/socket.h>
#include <sys/un.h>
#include <systemd/sd-daemon.h>
#include <errno.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/stat.h>
#include <evl/xbuf.h>
#include <sys/mman.h>

int service(void){
	int server_fd, xbuf, readed;
	struct sockaddr_un addr;
	payload_t send = {0};
	payload_t receive = {0};
	char rep[2];
	char outputs[17];
	char stat[100];
	int size;
	pthread_t rt;
	pthread_attr_t thattr;

	signal(SIGTERM, handle_signal);
	signal(SIGINT, handle_signal);
	signal(SIGHUP, handle_signal);

	mlockall(MCL_CURRENT | MCL_FUTURE);
	
//	if (initThread() == -1){
//		return 1;
//	}

	unlink(SOCKET_PATH);

	server_fd = socket(AF_UNIX, SOCK_STREAM, 0);
	if ( server_fd < 0 ){
		perror("Service socket");
		return 1;
	}

	memset(&addr, 0, sizeof(addr));
	addr.sun_family = AF_UNIX;
	strncpy(addr.sun_path, SOCKET_PATH, sizeof(addr.sun_path) -1);
	
	if (bind(server_fd, (struct sockaddr*)&addr, sizeof(addr)) < 0){
		perror("Service bind");
		return 1;
	}

	listen(server_fd, 5);

	chmod(SOCKET_PATH, 0660);

	sd_notify(0, "READY=1");
	sd_notify(0, "STATUS=Service running");
	
	xbuf = evl_create_xbuf(sizeof(receive),sizeof(send),EVL_CLONE_PUBLIC | EVL_CLONE_NONBLOCK,"ethercat_ioxbuf");
	if ( xbuf < 0 ) {
		sd_notify(0,"STATUS=Service create xbuf");
		return 1;
	}

	atomic_store_explicit(&run_enable,1,memory_order_release);
	
	struct sched_param param = {};
	param.sched_priority = sched_get_priority_max(SCHED_FIFO);
	if (sched_setscheduler(0, SCHED_FIFO, &param) == -1){
		return 1;
	}
	pthread_attr_init(&thattr);

	if (pthread_create(&rt, &thattr, &oobThread, NULL) != 0) {
		sd_notify(0,"STATUS=Service create rt thread");
		return 1;
	}

	while(atomic_load_explicit(&run_enable,memory_order_acquire)){
		int client = accept(server_fd,NULL,NULL);
		if (client < 0){
			if (errno == EINTR) continue;
			break;
		}
		msg_t cmd = {0};
		ssize_t n = read(client, &cmd, sizeof(cmd));
		if(n != sizeof(cmd)){
			sd_notify(0,"STATUS=Service invalid command read");
		}
		if (cmd.direction == READ){
			readed = read(xbuf, &receive,sizeof(payload_t));
			if (cmd.pdo == 6000 && cmd.index == 0){
				rep[0] = (char)('0' + get_bit(receive.di0, cmd.bit));
				rep[1] = '\0';
				write(client,rep,2);
			}else if (cmd.pdo == 6000 && cmd.index == 1){
				rep[0] = (char)('0' + get_bit(receive.di1, cmd.bit));
				rep[1] = '\0';
				write(client,rep,2);
			}
		}else if (cmd.direction == WRITE){
			if (cmd.pdo == 7000 && cmd.index == 0 && cmd.value == 1){
				set_bit(&send.do0,cmd.bit);
			}else if (cmd.pdo == 7000 && cmd.index ==1 && cmd.value == 1){
				set_bit(&send.do1,cmd.bit);
			}else if (cmd.pdo == 7000 && cmd.index == 0 && cmd.value == 0){
				clear_bit(&send.do0,cmd.bit);
			}else if (cmd.pdo == 7000 && cmd.index == 1 && cmd.value == 0){
				clear_bit(&send.do1,cmd.bit);
			}
			readed = write(xbuf, &send,sizeof(payload_t));
			for(int i=0;i<16;i++){
				outputs[i] = (char)('0' + get_bit(send.do0,i));
			}
			outputs[16] = '\0';
			write(client,outputs,17);
		}else if (cmd.direction == STATUS){
			readed = read(xbuf, &receive,sizeof(payload_t));
			size = snprintf(stat,sizeof(stat), "%ld %ld",receive.last, receive.duration);
			write(client,stat,size);
		}else {
			write(client,"Unknown command\n",16);
		}
		close(client);
		usleep(20000);
	}
	if (pthread_join(rt, NULL) != 0){
		sd_notify(0, "STATUS=Service waiting stopping rt thread");
		return 1;
	}
	close(xbuf);
	close(server_fd);
	unlink(SOCKET_PATH);
	sd_notify(0,"STOPPING=1");

	return 0;
}
