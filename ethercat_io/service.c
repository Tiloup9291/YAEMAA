#include "service.h"
#include "signalHandler.h"
#include "settings.h"
#include "oobThread.h"
#include "flags.h"
#include "utilities.h"
#include <sys/socket.h>
#include <sys/un.h>
#include <systemd/sd-daemon.h>
#include <systemd/sd-journal.h>
#include <errno.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <stdbool.h>

int service(void){
	int server_fd;
	struct sockaddr_un addr;
	char rep[2];
	char outputs[17];
	char stat[100];
	int size;
	pthread_t rt;
	pthread_attr_t thattr;
	uint16_t outputState = 0;

	struct sigaction sa;
	sa.sa_handler = handle_signal;
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = 0;
	sigaction(SIGTERM, &sa, NULL);
	sigaction(SIGINT, &sa, NULL);
	sigaction(SIGHUP, &sa, NULL);

	mlockall(MCL_CURRENT | MCL_FUTURE);

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
			if (!atomic_load_explicit(&run_enable, memory_order_acquire)){
				close(client);
				break;
			}
			if (errno == EINTR){
				close(client);
				break;
			}
			close(client);
			break;
		}
		msg_t cmd = {0};
		ssize_t n = read(client, &cmd, sizeof(cmd));
		if(n != sizeof(cmd)){
			sd_notify(0,"STATUS=Service invalid command read");
		}
		if (cmd.direction == READ){
			int backoff = 1;
			while(atomic_load_explicit(&producer.di0_written, memory_order_acquire)){
				for (int i = 0; i< backoff; i++){
					sched_yield();
				}
				if (backoff < 1024) backoff *= 2;
			}
			atomic_store_explicit(&producer.di0_reading, true, memory_order_release);
			uint16_t di0 = producer.di0;
			atomic_store_explicit(&producer.di0_reading, false, memory_order_release);
			if (cmd.pdo == 6000 && cmd.index == 0){
				rep[0] = (char)('0' + get_bit(di0, cmd.bit));
				rep[1] = '\0';
				write(client,rep,2);
			}
		}else if (cmd.direction == WRITE){
			if (cmd.pdo == 7000 && cmd.index == 0 && cmd.value == 1){
				int backoff=1;
				while(atomic_load_explicit(&producer.do0_reading, memory_order_acquire)){
					for (int i=0; i<backoff; i++){
						sched_yield();
					}
					if (backoff < 1024) backoff *= 2;
				}
				atomic_store_explicit(&producer.do0_written, true, memory_order_release);
				set_bit(&producer.do0,cmd.bit);
				outputState = producer.do0;
				atomic_store_explicit(&producer.do0_written, false, memory_order_release);
			}else if (cmd.pdo == 7000 && cmd.index == 0 && cmd.value == 0){
				int backoff = 1;
				while(atomic_load_explicit(&producer.do0_reading, memory_order_acquire)){
					for (int i=0; i< backoff; i++){
						sched_yield();
					}
					if (backoff < 1024) backoff *= 2;
				}
				atomic_store_explicit(&producer.do0_written, true, memory_order_release);
				clear_bit(&producer.do0,cmd.bit);
				outputState = producer.do0;
				atomic_store_explicit(&producer.do0_written, false, memory_order_release);
			}
			for(int i=0;i<16;i++){
				outputs[i] = (char)('0' + get_bit(outputState,i));
			}
			outputs[16] = '\0';
			write(client,outputs,17);
		}else if (cmd.direction == STATUS){
			int backoff =1;
			while(atomic_load_explicit(&producer.stat_written, memory_order_acquire)){
				for (int i=0; i< backoff; i++){
					sched_yield();
				}
				if (backoff < 1024) backoff *= 2;
			}
			atomic_store_explicit(&producer.stat_reading, true, memory_order_release);
			long duration = producer.duration;
			long last = producer.last;
			atomic_store_explicit(&producer.stat_reading, false, memory_order_release);
			size = snprintf(stat,sizeof(stat), "%ld %ld",last, duration);
			write(client,stat,size);
		}else {
			write(client,"Unknown command\n",16);
		}
		close(client);
		usleep(20000);
	}
	sd_journal_print(LOG_INFO,"Exiting non-rt thread\n");
	if (pthread_join(rt, NULL) != 0){
		sd_notify(0, "STATUS=Service waiting stopping rt thread");
		return 1;
	}
	close(server_fd);
	unlink(SOCKET_PATH);
	sd_notify(0,"STOPPING=1");

	return 0;
}
