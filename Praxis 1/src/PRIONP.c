#include "../lib/PRIONP.h"
#include <stdio.h>

static queue_object* PRIONP_queue;
//You can add more global variables here
static process* current_process = NULL;

// Function to sort the queue by priority
void sort_queue_by_priority(queue_object* queue) {
    queue_object* i;
    queue_object* j;
    for(i = queue->next; i != NULL; i = i->next) {
        for(j = i->next; j != NULL; j = j->next) {
            if(((process*)i->object)->priority < ((process*)j->object)->priority) {
                void* temp = i->object;
                i->object = j->object;
                j->object = temp;
            }
        }
    }
}


process* PRIONP_tick (process* running_process){
    (void)running_process; 

    if (current_process == NULL || current_process->time_left == 0){
        current_process = queue_poll(PRIONP_queue); 
    }

    if (current_process != NULL){
        current_process->time_left--; 
    }

    return current_process;
}

int PRIONP_startup(){
    PRIONP_queue = new_queue();

    if (PRIONP_queue == NULL){
        return 1;
    }

    return 0;
}

process* PRIONP_new_arrival(process* arriving_process, process* running_process){
    (void)running_process; 

    if(arriving_process != NULL){
        queue_add(arriving_process, PRIONP_queue); 
        if(PRIONP_queue->next != NULL) {
            sort_queue_by_priority(PRIONP_queue);
        }
    }

    return current_process;
}

void PRIONP_finish(){
    free_queue(PRIONP_queue);
}
