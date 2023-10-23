#include "../lib/LCFSPR.h"

static queue_object* LCFSPR_queue;
//You can add more global variables here

// Helper function to handle running process
process* handle_running_process(process* running_process) {
    if (running_process) {
        --running_process->time_left;
    }
    return running_process;
}

// Helper function to handle new arrival
process* handle_new_arrival(process* arriving_process, process* running_process) {
    if (arriving_process) {
        running_process = (running_process && running_process->time_left > 0) ? queue_add(running_process, LCFSPR_queue), arriving_process : arriving_process;
    }
    return running_process;
}

int LCFSPR_startup(){
    LCFSPR_queue = new_queue();
    return (LCFSPR_queue == NULL) ? 1 : 0;

}

process* LCFSPR_tick (process* running_process){
    running_process = (running_process && running_process->time_left > 0) ? running_process : queue_poll(LCFSPR_queue);
    return handle_running_process(running_process);
}


process* LCFSPR_new_arrival(process* arriving_process, process* running_process){
    return handle_new_arrival(arriving_process, running_process);
}

void LCFSPR_finish(){
    free_queue(LCFSPR_queue);
}