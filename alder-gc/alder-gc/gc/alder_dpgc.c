#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include "collector.h"

typedef struct Node Node;
struct Node{
    Collectee collectee; /* All managed objects must begin with this. */
    Node *next;
    uint64_t status;
    uint64_t s;
    uint64_t *addr;
    uint64_t *old;
    uint64_t *n;
    int i;
};

typedef struct alder_thread_mcas_struct {
    Collectee collectee; /* All managed objects must begin with this. */
    uint64_t status;
    uint64_t s;
    uint64_t *addr;
    uint64_t *old;
    uint64_t *n;
} alder_thread_mcas_t;

int
touch(Collector *collector, Collectee *collectee)
{
    Node *node;

    node = (Node *)collectee;

    printf("tracing references held by %p\n", collectee);

    if(node->next != NULL)
        touchobj(collector, (Collectee *)(node->next));

    return 1;
}

void
dispose(Collector *collector, Collectee *collectee)
{
    printf("disposing of %p\n", collectee);
    free(collectee);
}

int
main_dpgb(void)
{
    Collector *col;
    Node      *head;
    Node      *node;
    Node      *prev;
    int       i;

    col = makecollector(touch, dispose);
    if(col != NULL){
        head = calloc(1, sizeof(Node));
        if(head != NULL){
            addroot(col, (Collectee *)head);
            unpausegc(col);

            prev = head;
            for(i = 0; i < 9; i++){
                node = gcalloc(col, sizeof(Node));
                if(node != NULL){
                    node->i = i;
   
                    /* Create 4999 reachable objects, and make the other
                     * 5000 objects unreachable so that they are collected. */
                    pausegc(col);
                    if(i < 5){
                        prev->next = node;
                        addobj(col, (Collectee *)node, (Collectee *)prev);
                    } else
                        addobj(col, (Collectee *)node, NULL);
                    unpausegc(col);

                    prev = node;
                } else{
                    fprintf(stderr, "unable to allocate list node\n");
                    exit(EXIT_FAILURE);
                }
            }
        } else{
            fprintf(stderr, "unable to allocate list root\n");
            exit(EXIT_FAILURE);
        }
    } else{
        fprintf(stderr, "unable to allocate collector\n");
        exit(EXIT_FAILURE);
    }

    /* Normally, one would start the collector long before now, but
     * this provides an example of a large number of unreachable objects
     * simply. */
//    unpausegc(col);

    return EXIT_SUCCESS;
}
