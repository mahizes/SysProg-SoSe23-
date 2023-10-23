#include "../lib/RR.h"

static queue_object* RR_queue;
//You can add more global variables
static int RR_quantum;
static int running_time;

process* RR_tick (process* running_process){
    if (running_process == NULL || running_process->time_left == 0 || running_time == RR_quantum){
        if (running_process != NULL && running_process->time_left != 0){
            queue_add(running_process, RR_queue);
        }
        running_process = queue_poll(RR_queue);
        running_time = 0;
    }
    if (running_process != NULL){
        running_process->time_left--;
        running_time++;
    }
    return running_process;
}

int RR_startup(int quantum){
    RR_queue = new_queue();
    RR_quantum = quantum;
    running_time = 0;
    if (RR_queue == NULL){
        return 1;
    }
    return 0;
}


process* RR_new_arrival(process* arriving_process, process* running_process){
    if(arriving_process != NULL){
        queue_add(arriving_process, RR_queue);
    }
    return running_process;
}


void RR_finish(){
    free_queue(RR_queue);
}
