//
//  radix_list_simple.c
//  alder-sort
//
//  Created by Sang Chul Choi on 8/23/13.
//  Copyright (c) 2013 Sang Chul Choi. All rights reserved.
//

#include <stdio.h>
#include <stdlib.h>
#include "radix_list_simple.h"

#define ended(a,b)  b>0 && a->data[b-1]==0
//#define append(s,a) tmp=a; while(tmp->next) tmp=tmp->next; tmp->next=s
#define append(s,a) tmp=a; while(tmp->next) tmp=tmp->next; tmp->next=s
#define deal(a,p)   tmp=a->next, a->next=p, p=a, a=tmp
//#define deal(a,p)   tmp=a->next; t2=p; while(t2->next) t2=t2->next; a->next=t2->next; t2->next=a; a=tmp

list *rsort(list *a, int b, list *sequel)
{
    list *pile[256];
    list *tmp;
//    list *t2;
    int i;
    if (a==0) return sequel;
    if (ended(a, b)) {
        append(sequel, a);
        return a;
    }
//    for (i = 255; i >= 0; i--) {
    for (i = 0; i < 256; i++) {
        pile[i] = 0;
    }
    while (a) {
        deal(a, pile[a->data[b]]);
//        list *p = pile[a->data[b]];
//        tmp=a->next;
//        if (p == NULL) {
//            p = a;
//            p->next = NULL;
//            pile[a->data[b]] = p;
//        } else {
//            t2=p;
//            while(t2->next) t2=t2->next;
//            a->next=t2->next;
//            t2->next=a;            
//        }
//        a=tmp;
    }
    for (i = 256; --i >= 0;) {
//    for (i = 0; i < 256; i++) {
        sequel = rsort(pile[i], b+1, sequel);
    }
    return sequel;
}

static list *new_list(size_t n, size_t b)
{
    srand(1);
    list *h;
    list *l = malloc(sizeof(list));
    h = l;
    l->data = malloc((b+1)*sizeof(char));
    for (size_t j = 0; j < b; j++) {
        l->data[j] = rand() % 26 + (int)('A');
    }
    l->data[b] = '\0';
    l->next = NULL;
    
    for (size_t i = 1; i < n; i++) {
        list *t = malloc(sizeof(list));
        t->data = malloc((b+1)*sizeof(char));
        for (size_t j = 0; j < b; j++) {
            t->data[j] = rand() % 26 + (int)('A');
        }
        t->data[b] = '\0';
        t->next = NULL;
        l->next = t;
        l = t;
    }
    return h;
}

static void printit(list *l)
{
    printf("---\n");
    while (l->next != NULL) {
        printf("%s\n", l->data);
        l = l->next;
    }
    printf("%s\n", l->data);
}

static void delete_list(list *l)
{
    list *t;
    while (l->next != NULL) {
        t = l;
        l = l->next;
        free(t->data);
        free(t);
    }
    free(l->data);
    free(l);
}

int radix_list_simple_main()
{
    for (size_t i = 0; i < 1000; i++) {
        list *a = new_list(1000, 50);
//        printit(a);
        list *b = rsort(a, 0, NULL);
//        printit(b);
        delete_list(b);
//        delete_list(a);
    }
    
//    radix_exchange(0,n-1,1<<31);
    
//    printf("Sorted? %s\n", sorted()?"Yes":"No");
    
    
//    rsort(a, ;, ;)
    
    return 0;
}
