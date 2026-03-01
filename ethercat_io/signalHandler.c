#include "signalHandler.h"
#include "settings.h"
#include "flags.h"
#include <systemd/sd-daemon.h>

void handle_signal(int sig){
	if (sig == SIGTERM || sig == SIGINT){
		atomic_store_explicit(&run_enable, 0, memory_order_release);
		sd_notify(0, "STATUS=Stopping...");
	}else if (sig == SIGHUP){
		sd_notify(0, "STATUS=Reload requested");
	}
}
