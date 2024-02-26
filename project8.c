

#include <stdlib.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <sys/mman.h>

#include <stdbool.h>

#define ALIGNMENT 16 // Must be power of 2
#define GET_PAD(x) ((ALIGNMENT - 1) - (((x)-1) & (ALIGNMENT - 1)))

#define PADDED_SIZE(x) ((x) + GET_PAD(x))
#define PTR_OFFSET(p, offset) ((void *)((char *)(p) + (offset)))
// pad size x
// mark in use
// shange size "remaining space"
// return node +padded size

struct block *head = NULL;
struct block
{
    int size;   // Size in bytes
    int in_use; // Boolean
    struct block *next;
};
bool current_size_equal_or_greater_than_size(struct block *current, int size)
{
    return current->size >= (int)(PADDED_SIZE(size));
}

bool current_size_big_enough_inluding_next_node(struct block *current, int size)
{
    return current->size > (int)(PADDED_SIZE(size) + PADDED_SIZE(sizeof(struct block))) + PADDED_SIZE(1);
}

void split_node(struct block *current, struct block *temp_block, int size)
{
    temp_block->size = current->size - PADDED_SIZE(size) - PADDED_SIZE(sizeof(struct block));
    temp_block->in_use = 0;
    temp_block->next = current->next;
    current->next = temp_block;
    current->size = PADDED_SIZE(size);
}

void *myalloc(int size)
{
    if (head == NULL)
    {
        void *heap = mmap(NULL, 1024, PROT_READ | PROT_WRITE,
                          MAP_ANON | MAP_PRIVATE, -1, 0);
        int data_size = 1024 - PADDED_SIZE(sizeof(struct block));

        head = heap;
        head->size = data_size;
        head->next = NULL;
        head->in_use = 0;
    }

    struct block *current = head;

    while (current != NULL)
    {
        if (current->in_use == 0)
            if (current_size_equal_or_greater_than_size(current, size))
            {
                current->in_use = 1;
                if (current_size_big_enough_inluding_next_node(current, size))
                {
                    struct block *temp_block = PTR_OFFSET(current, PADDED_SIZE(size));
                    split_node(current, temp_block, size);
                }
                return PTR_OFFSET(current, PADDED_SIZE(sizeof(struct block)));
                // current = current->next;
                //  return PTR_OFFSET(current, PADDED_SIZE(sizeof(struct block)));
            }
        current = current->next;
    }
    return NULL;
}
void print_data(void)
{
    struct block *b = head;

    if (b == NULL)
    {
        printf("[empty]\n");
        return;
    }

    while (b != NULL)
    {
        // Uncomment the following line if you want to see the pointer values
        // printf("[%p:%d,%s]", b, b->size, b->in_use? "used": "free");
        printf("[%d,%s]", b->size, b->in_use ? "used" : "free");
        if (b->next != NULL)
        {
            printf(" -> ");
        }

        b = b->next;
    }

    printf("\n");
}
void myfree(void *pointer)
{
    struct block *current = head;
    struct block *point = PTR_OFFSET(pointer, -PADDED_SIZE(sizeof(struct block)));
    point->in_use = 0;
    while (current->next != NULL)
    {
        struct block *next_node = current->next;
        if ((current->in_use == 0) && (next_node->in_use == 0))
        {
            current->size +=( next_node->size + PADDED_SIZE(sizeof(struct block)));
            current->next = next_node->next;
        }
        else
        {

            current = next_node;
        }
    }
}

int main()
{
    void *p, *q, *r, *s;

    p = myalloc(10);
    print_data();
    q = myalloc(20);
    print_data();
    r = myalloc(30);
    print_data();
    s = myalloc(40);
    print_data();

    myfree(q);
    print_data();
    myfree(p);
    print_data();
    myfree(s);
    print_data();
    myfree(r);
    print_data();
}

// // int *p = myalloc(sizeof(int) * 5);
// }

// struct node *node = PTR_OFFSET(y, -PADDED_SIZE(sizeof(structnode)))

// free (pointer )
// go back the size of the padding
//       // struct node *node = PTR_OFFSET(y, -PADDED_SIZE(sizeof(structnode)))
//   set the nodes in use to 0