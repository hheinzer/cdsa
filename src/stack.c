#include "stack.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(void)
{
    // create an integer stack
    Stack s = stack_create(sizeof(int), memcpy, 0, free);

    // push integers 0 through 9
    for (long i = 0; i < 10; ++i) stack_push(&s, &i);

    // pop item
    free(stack_pop(&s));

    // peek at front item
    printf("s.peek() = %d\n", *(int *)stack_peek(&s));

    // print items
    printf("a: ");
    for (const StackItem *item = s.head; item; item = item->next)
        printf("%d%s", *(int *)item->data, (item->next ? ", " : "\n"));

    // clear items
    stack_clear(&s);
}
