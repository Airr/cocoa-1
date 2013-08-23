//
//  raghavan291.h
//  brass-ds
//
//  Created by Sang Chul Choi on 8/18/13.
//  Copyright (c) 2013 Sang Chul Choi. All rights reserved.
//

#ifndef brass_ds_raghavan291_h
#define brass_ds_raghavan291_h

#include<stdio.h>
#include<stdlib.h>
#include<limits.h>
#include<string.h>
#include<time.h>

#define DEBUG 1
#define BLOCKSIZE 256
#define MAX(X,Y) X>Y?X:Y
#define MIN(X,Y) X<Y?X:Y

typedef int key_t;
typedef int height_t;

typedef struct list_node
{
	int interval_a, interval_b;
	struct list_node *next;
} list_node_t;

typedef struct node_text
{
    key_t key;
	int interval_a;
	int interval_b;
	int measure;
	int leftmin;
	int rightmax;
    height_t height;
	list_node_t *interval_list;
    struct node_text *left;
    struct node_text *right;
    struct node_text *parent;
} m_tree_t;

m_tree_t *currentblock = NULL, *free_list = NULL;
int size_left;
int nodes_taken = 0;
int nodes_returned = 0;

m_tree_t *get_node();
void return_node(m_tree_t *);
m_tree_t *create_node();
list_node_t *create_list_node(int , int );
m_tree_t *create_m_tree();
void left_rotation(m_tree_t *);
void right_rotation(m_tree_t *);
void balance_left_interval(m_tree_t *);
void balance_right_interval(m_tree_t *);
void insert_BST(m_tree_t *, int );
void insert_assoc_interval(m_tree_t *, int , int );
void update_measure(m_tree_t *);
int query_length(m_tree_t *);
void insert_interval(m_tree_t *, int , int );
list_node_t *delete_list_node(list_node_t *, int , int );
void delete_assoc_interval(m_tree_t *, int , int );
void delete_interval(m_tree_t *, int , int );
void inorder(m_tree_t *);


#endif
