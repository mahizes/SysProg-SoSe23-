#include "../lib/SJN.h"

static queue_object* SJN_queue;
//You can add more global variables here
static queue_object* running_queue;
static process* current_process = NULL;

process* SJN_tick (process* running_process){
    (void)running_process; // This line suppresses the unused parameter warning

    // If there's no running process or the running process has finished, select the shortest job next.
    if (current_process == NULL || current_process->time_left == 0){
        queue_object* curr = SJN_queue->next;
        queue_object* prev = SJN_queue;
        queue_object* shortest = NULL;
        queue_object* prev_shortest = NULL;
        while (curr != NULL){
            process* curr_process = (process*)curr->object;
            if (shortest == NULL || curr_process->time_left < ((process*)shortest->object)->time_left){
                prev_shortest = prev;
                shortest = curr;
            }
            prev = curr;
            curr = curr->next;
        }
        if (shortest != NULL){
            prev_shortest->next = shortest->next;
            queue_add(shortest->object, running_queue);
            current_process = (process*)queue_poll(running_queue);
        } else {
            // If there's no process in the incoming queue, idle.
            return NULL;
        }
    }

    // Decrease the time left for the running process.
    if (current_process != NULL){
        current_process->time_left--;
    }

    return current_process;
}

int SJN_startup(){
    SJN_queue = new_queue(); // This will be used as the incoming_queue
    running_queue = new_queue();
    if (SJN_queue == NULL || running_queue == NULL){
        return 1;
    }
    return 0;
}

process* SJN_new_arrival(process* arriving_process, process* running_process){
    (void)running_process; // This line suppresses the unused parameter warning

    // If a new process arrives, add it to the incoming queue.
    if(arriving_process != NULL){
        queue_add(arriving_process, SJN_queue);
    }

    // Return the current process, do not preempt.
    return current_process;
}

void SJN_finish(){
    free_queue(SJN_queue);
    free_queue(running_queue);
}
