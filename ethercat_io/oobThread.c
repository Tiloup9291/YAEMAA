#define _GNU_SOURCE
#include "oobThread.h"
#include "flags.h"
#include "settings.h"
#include <evl/thread.h>
#include <evl/clock.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <pthread.h>
#include <sched.h>
#include "ethercatPDOs.h"
#include <systemd/sd-daemon.h>
#include <systemd/sd-journal.h>

#define FREQUENCY 1000
#define NSEC_PER_SEC (1000000000L)
#define PERIOD_NS (NSEC_PER_SEC / FREQUENCY)
#define DIFF_NS(A, B) (((B).tv_sec - (A).tv_sec) * NSEC_PER_SEC + (B).tv_nsec - (A).tv_nsec)
#define TIMESPEC2NS(T) ((uint64_t) (T).tv_sec * NSEC_PER_SEC + (T).tv_nsec)

static ec_master_t *master = NULL;
static ec_domain_t *domain = NULL;
static ec_slave_config_t *sc = NULL;
static uint8_t *domain_pd = NULL;
static unsigned int sync_ref_counter = 0;
const struct timespec cycletime = {0, PERIOD_NS};

int initThread(void){
	off_do = 0;
	off_do2 = 0;
	off_di = 0;
	off_di2 = 0;
	master = ecrt_request_master(0);
	if (!master){
		return -1;
	}
	domain = ecrt_master_create_domain(master);
	if (!domain){
		sd_journal_print(LOG_ERR,"Unable create domain\n");
		return -1;
	}
	sc = ecrt_master_slave_config(master, 0, 0, 0x00000b95, 0x00001500);
	if (!sc){
		sd_journal_print(LOG_ERR,"Unable to config slave\n");
		return -1;
	}
	if (ecrt_slave_config_pdos(sc, EC_END, slave_0_sync)){
		sd_journal_print(LOG_ERR,"Unable to config sync manage\n");
		return -1;
	}
	if (ecrt_domain_reg_pdo_entry_list(domain, domain_regs)){
		sd_journal_print(LOG_ERR, "Unable to register pdo to domain\n");
		return -1;
	}
	ecrt_slave_config_dc(sc, 0x0100, PERIOD_NS, 0, 0, 0);
	if (ecrt_master_activate(master)){
		return -1;
	}
	domain_pd = ecrt_domain_data(domain);
	if (!domain){
		return -1;
	}
	sd_journal_print(LOG_INFO, "DO: %u %u | DI: %u %u\n",off_do,off_do2,off_di,off_di2);
	ec_slave_config_state_t s;
	ecrt_slave_config_state(sc, &s);
	sd_journal_print(LOG_INFO, "Slave : State 0x%02X.\n",s.al_state);
	sd_journal_print(LOG_INFO, "Slave : %s.\n", s.online ? "online" : "offline");
	sd_journal_print(LOG_INFO, "Slave : %soperational.\n", s.operational ? "": "Not ");
	return 0;
}

struct timespec timespec_add(struct timespec time1, struct timespec time2){
	struct timespec result;
	if ((time1.tv_nsec + time2.tv_nsec) >= NSEC_PER_SEC) {
		result.tv_sec = time1.tv_sec + time2.tv_sec +1;
		result.tv_nsec = time1.tv_nsec + time2.tv_nsec - NSEC_PER_SEC;
	} else {
		result.tv_sec = time1.tv_sec + time2.tv_sec;
		result.tv_nsec = time1.tv_nsec + time2.tv_nsec;
	}
	return result;
}

void* oobThread(void* arg){
	cpu_set_t cpuset;
	pthread_t thread = pthread_self();
	CPU_ZERO(&cpuset);
	CPU_SET(2,&cpuset);
	pthread_setaffinity_np(thread,sizeof(cpu_set_t), &cpuset);
	struct timespec wakeupTime, time;
	struct timespec startTime, endTime, lastStartTime = {};
	uint64_t period_ns = 0, exec_ns = 0, latency_ns = 0, latency_min_ns = 0, latency_max_ns = 0, period_min_ns = 0, period_max_ns = 0, exec_min_ns = 0, exec_max_ns = 0;
	arg = NULL;
	int efd;
	int xbuf;
	payload_t send = {0};
	payload_t receive = {0};
	dig_array_t inputs, outputs;
	inputs.full = 0;
	outputs.full = 0;
	
	if(initThread()<0){
		return NULL;
	}

	xbuf = open("/dev/evl/xbuf/ethercat_ioxbuf", O_RDWR);
	if(xbuf < 0){
		sd_notify(0,"STATUS=Service OOB open xbuf");
		return NULL;
	}
	
	struct sched_param param;
	int policy, ret;
	int mask = 0xffffffff;
	int oldmask;
	pthread_getschedparam(pthread_self(), &policy, &param);
	param.sched_priority = 90;
	ret = pthread_setschedparam(pthread_self(), SCHED_FIFO, &param);
	sd_journal_print(LOG_INFO,"Attaching thread");
	
//	if(initThread() < 0){
//		return NULL;
//	}
	
	efd = evl_attach_thread(EVL_CLONE_PUBLIC, "oobThread");
	evl_clear_thread_mode(efd, mask, &oldmask);
	
	//xbuf = open("/dev/evl/xbuf/ethercat_ioxbuf", O_RDWR);
	//if (xbuf < 0){
	//	return NULL;
	//}

//	if (initThread()<0){
//		return NULL;
//	}

	evl_read_clock(EVL_CLOCK_MONOTONIC, &wakeupTime);	
	ecrt_master_application_time(master, TIMESPEC2NS(wakeupTime));
	while(1){
		wakeupTime = timespec_add(wakeupTime, cycletime);
		evl_sleep_until(EVL_CLOCK_MONOTONIC, &wakeupTime);

		evl_read_clock(EVL_CLOCK_MONOTONIC, &startTime);
		latency_ns = DIFF_NS(wakeupTime, startTime);
		period_ns = DIFF_NS(lastStartTime, startTime);
		exec_ns = DIFF_NS(lastStartTime, endTime);
		lastStartTime = startTime;

		if (latency_ns > latency_max_ns) {
			latency_max_ns = latency_ns;
		}
		if (latency_ns < latency_min_ns){
			latency_min_ns = latency_ns;
		}
		if (period_ns > period_max_ns) {
			period_max_ns = period_ns;
		}
		if (period_ns < period_min_ns) {
			period_min_ns = period_ns;
		}
		if (exec_ns > exec_max_ns) {
			exec_max_ns = exec_ns;
		}
		if (exec_ns < exec_min_ns) {
			exec_min_ns = exec_ns;
		}

		if (atomic_load_explicit(&run_enable, memory_order_acquire) == 0){
			break;
		}
		
		ecrt_master_receive(master);
		ecrt_domain_process(domain);
		
		send.duration = exec_ns;
		send.last = period_ns;
		period_max_ns = 0;
		period_min_ns = 0xffffffff;
		exec_max_ns = 0;
		exec_min_ns = 0xffffffff;
		latency_max_ns = 0;
		latency_min_ns = 0xffffffff;

		oob_read(xbuf,&receive, sizeof(payload_t));
		outputs.full = receive.do0;

		inputs.full = EC_READ_U16(domain_pd + off_di);
		send.di0 = inputs.full;

		EC_WRITE_U16(domain_pd + off_do, outputs.full);

		oob_write(xbuf,&send,sizeof(payload_t));
		
		evl_read_clock(EVL_CLOCK_MONOTONIC, &time);
		ecrt_master_application_time(master, TIMESPEC2NS(time));

		if (sync_ref_counter) {
			sync_ref_counter--;
		} else {
			sync_ref_counter = 1;
			ecrt_master_sync_reference_clock(master);
		}
		ecrt_master_sync_slave_clocks(master);

		ecrt_domain_queue(domain);
		ecrt_master_send(master);
		
		evl_read_clock(EVL_CLOCK_MONOTONIC, &endTime);
	}
	
	evl_detach_self();
	ecrt_release_master(master);
	close(xbuf);
	sd_notify(0,"STATUS=Service OOB stopped");
	return NULL;
}
