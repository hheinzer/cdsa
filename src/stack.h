#pragma once

#include "list.h"

// general purpose stack, based on the doubly linked list
typedef List Stack;
typedef ListItem StackItem;
typedef ListDataCopy StackDataCopy;
typedef ListDataCompare StackDataCompare;
typedef ListDataFree StackDataFree;

// create an empty stack
static Stack stack_create(size_t data_size, StackDataCopy data_copy, StackDataCompare data_cmp,
                          StackDataFree data_free)
{
    return list_create(data_size, data_copy, data_cmp, data_free);
}

// add an item to the end of the stack
[[maybe_unused]] static void stack_push(Stack *stack, void *data)
{
    list_append(stack, data);
}

// remove the item at the end of the stack, and return its data
[[maybe_unused]] static void *stack_pop(Stack *stack)
{
    return list_pop(stack, -1);
}

// return the item at the end of the stack
[[maybe_unused]] static void *stack_peek(const Stack *stack)
{
    return stack->tail->data;
}

// remove all items from the stack
[[maybe_unused]] static void stack_clear(Stack *stack)
{
    list_clear(stack);
}
