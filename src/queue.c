#include "queue.h"

#include <stdio.h>
#include <string.h>

int main(void)
{
    // create an integer queue
    Queue q = queue_create(sizeof(int), memcpy, 0, free);

    // enqueue integers 0 through 9
    for (long i = 0; i < 10; ++i) queue_enqueue(&q, &i);

    // dequeue item
    free(queue_dequeue(&q));

    // print items
    printf("q = [");
    QueueForEach(item, &q) printf("%s%d", (item == q.head ? "" : ", "), *(int *)item->data);
    printf("]\n");

    // peek at front item
    printf("q.peek() = %d\n", *(int *)queue_peek(&q));

    // clear items
    queue_clear(&q);
}
