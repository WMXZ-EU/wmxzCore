/*
 * WMXZ Teensy core library
 * Copyright (c) 2016 Walter Zimmer.
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
 * BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */
 //list.h
 #ifndef LIST_H
#define LIST_H

#include "basics.h"

typedef struct nodeStruct
{	Task_Obj job;
	Ptr meta;
    struct nodeStruct *next;
} Node_t;

typedef struct listStruct
{   int node_num;
	Node_t *head;
    Node_t *nodeList;
} List_t;

#ifdef __cplusplus
extern "C"{
#endif

void list_init(List_t *List, Node_t *nodeList, int node_num);
int list_insert(List_t *List, Fxn_t funct, Ptr state, Ptr data, int nice);
int list_remove(List_t *List, Fxn_t funct);
int node_count(List_t *List);

#ifdef __cplusplus
}
#endif

#endif
