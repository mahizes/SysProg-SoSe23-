#include "../lib/MLF.h"
#include <stdio.h>

static queue_object** MLF_queues;
//You can add more global variables here
static int total_queue_levels = 4; // Number of levels
static int active_level; // Current level
static int quantum_remaining; // Time left for the current quantum

// Helper function to calculate factorial
int factorial(int n) {
    int fact = 1;
    for(int i = 2; i <= n; i++)
        fact *= i;
    return fact;
}

// Helper function to handle the running process
process* MLF_handle_running_process(process* running_process) {
    if(running_process != NULL){
        running_process->time_left--;
        quantum_remaining--;
    }
    return running_process;
}

// Helper function to handle the new process
process* MLF_handle_new_process() {
    process* new_process = NULL;
    for(int i = 0; i < total_queue_levels; i++){
        new_process = queue_poll(MLF_queues[i]);
        if(new_process != NULL){
            active_level = i;
            quantum_remaining = (active_level == total_queue_levels - 1) ? new_process->time_left : factorial(active_level + 1);
            break;
        }
    }
    return new_process;
}

process* MLF_tick (process* running_process){
    if (running_process == NULL || running_process->time_left == 0 || quantum_remaining == 0){
        if (running_process != NULL && quantum_remaining == 0 && running_process->time_left != 0 && active_level < total_queue_levels - 1){
            queue_add(running_process, MLF_queues[active_level + 1]);
        }
        running_process = NULL;
    }
    if(running_process == NULL){
        running_process = MLF_handle_new_process();
    }
    return MLF_handle_running_process(running_process);
}

/**
 * Do everything you have to at startup in this function. It is called once.
 * @result 0 if everything was fine. Any other number if there was an error.
 */
int MLF_startup(){
    MLF_queues = malloc(sizeof(queue_object*) * total_queue_levels);
    if (MLF_queues == NULL){
        return 1;
    }
    for (int i = 0; i < total_queue_levels; i++){
        MLF_queues[i] = new_queue();
        if (MLF_queues[i] == NULL){
            return 1;
        }
    }
    return 0;
}

process* MLF_new_arrival(process* arriving_process, process* running_process){
    if(arriving_process != NULL){
        queue_add(arriving_process, MLF_queues[0]);
    }
    return running_process;
}

/**
 * is called once after the all processes were handled. In case you want to cleanup something
 */
void MLF_finish(){
    for(int i = 0; i < total_queue_levels; i++){
        free_queue(MLF_queues[i]);
    }
    free(MLF_queues);
}
