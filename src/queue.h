#pragma once

#include "list.h"

// general purpose queue, based on the doubly linked list
typedef List Queue;
typedef ListItem QueueItem;
typedef ListDataCopy QueueDataCopy;
typedef ListDataCompare QueueDataCompare;
typedef ListDataFree QueueDataFree;

// create an empty queue
static Queue queue_create(size_t data_size, QueueDataCopy data_copy, QueueDataCompare data_cmp,
                          QueueDataFree data_free)
{
    return list_create(data_size, data_copy, data_cmp, data_free);
}

// add an item to the end of the queue
[[maybe_unused]] static void queue_enqueue(Queue *queue, void *data)
{
    list_append(queue, data);
}

// remove the item at the front of the queue, and return its data
[[maybe_unused]] static void *queue_dequeue(Queue *queue)
{
    return list_pop(queue, 0);
}

// return the item at the front of the queue
[[maybe_unused]] static void *queue_peek(const Queue *queue)
{
    return queue->head->data;
}

// remove all items from the queue
[[maybe_unused]] static void queue_clear(Queue *queue)
{
    list_clear(queue);
}
