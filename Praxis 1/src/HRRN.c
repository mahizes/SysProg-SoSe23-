#include "../lib/HRRN.h"

static queue_object* HRRN_queue;
//You can add more global variables and structs here
static unsigned int current_time;
static process* current_process = NULL;

// Function to remove a specific process from the queue
static void queue_remove(process* proc, queue_object* queue) {
    queue_object* current = queue->next;
    queue_object* previous = queue;
    while (current != NULL) {
        if (current->object == proc) {
            previous->next = current->next;
            free(current);
            break;
        }
        previous = current;
        current = current->next;
    }
}

// Function to find the process with the highest response ratio
process* find_highest_ratio_process(queue_object* queue) {
    queue_object* current = queue->next;
    process* highest_ratio_process = NULL;
    double highest_ratio = -1.0;
    while (current != NULL) {
        process* proc = (process*)current->object;
        double wait_time = current_time - proc->start_time;
        double response_ratio = (wait_time + proc->time_left) / (double)proc->time_left;
        if (response_ratio > highest_ratio) {
            highest_ratio = response_ratio;
            highest_ratio_process = proc;
        }
        current = current->next;
    }
    return highest_ratio_process;
}

process* HRRN_tick (process* running_process){
    (void)running_process; // This line suppresses the unused parameter warning

    current_time++;
    if (current_process == NULL || current_process->time_left == 0) {
        current_process = find_highest_ratio_process(HRRN_queue);
        if (current_process != NULL) {
            queue_remove(current_process, HRRN_queue);
        }
    }
    if (current_process != NULL) {
        current_process->time_left--;
    }
    return current_process;
}

int HRRN_startup(){
    HRRN_queue = new_queue();
    current_time = 0;
    return (HRRN_queue == NULL) ? 1 : 0;
}

process* HRRN_new_arrival(process* arriving_process, process* running_process){
    (void)running_process; // This line suppresses the unused parameter warning
    
    if (arriving_process != NULL) {
        queue_add(arriving_process, HRRN_queue);
    }
    return current_process;
}

void HRRN_finish(){
    free_queue(HRRN_queue);
}
