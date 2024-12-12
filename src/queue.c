#include "queue.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(void)
{
    // create an integer queue
    Queue s = queue_create(sizeof(int), memcpy, 0, free);

    // enqueue integers 0 through 9
    for (long i = 0; i < 10; ++i) queue_enqueue(&s, &i);

    // dequeue item
    free(queue_dequeue(&s));

    // peek at front item
    printf("s.peek() = %d\n", *(int *)queue_peek(&s));

    // print items
    printf("a: ");
    for (const QueueItem *item = s.head; item; item = item->next)
        printf("%d%s", *(int *)item->data, (item->next ? ", " : "\n"));

    // clear items
    queue_clear(&s);
}
