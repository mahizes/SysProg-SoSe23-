#include "../lib/queue.h"
#include <stdlib.h>
#include <stdio.h>

int queue_add(void* new_object, queue_object* queue){
    // Allocate memory for the new queue object
    queue_object* new_queue_object = (queue_object*)malloc(sizeof(queue_object));
    if (new_queue_object == NULL) {
        return 1; // Return 1 if memory allocation failed
    }

    // Initialize the new queue object
    new_queue_object->object = new_object;
    new_queue_object->next = NULL;

    // Find the last queue object
    queue_object* last = queue;
    while (last->next != NULL) {
        last = last->next;
    }

    // Add the new queue object to the end of the queue
    last->next = new_queue_object;

    return 0; // Return 0 if everything was fine
}

void* queue_poll(queue_object* queue){
    // Check if the queue is empty
    if (queue->next == NULL) {
        return NULL;
    }

    // Get the next queue object
    queue_object* next_queue_object = queue->next;

    // Get the object to return
    void* object = next_queue_object->object;

    // Remove the next queue object from the queue
    queue->next = next_queue_object->next;

    // Free the next queue object
    free(next_queue_object);

    // Return the object
    return object;
}

queue_object* new_queue(){
    // Allocate memory for the new queue
    queue_object* queue = (queue_object*)malloc(sizeof(queue_object));
    if (queue == NULL) {
        return NULL; // Return NULL if memory allocation failed
    }

    // Initialize the new queue
    queue->object = NULL;
    queue->next = NULL;

    // Return the new queue
    return queue;
}


void free_queue(queue_object* queue){
    // Loop through the queue
    while (queue != NULL) {
        // Get the next queue object
        queue_object* next_queue_object = queue->next;

        // Free the current queue object
        free(queue);

        // Move to the next queue object
        queue = next_queue_object;
    }
}

void* queue_peek(queue_object* queue){
    // Check if the queue is empty
    if (queue->next == NULL) {
        return NULL;
    }

    // Return the object of the next queue object
    return queue->next->object;
}


