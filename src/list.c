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
 // list.c
//
#include "jobs.h"
#include "list.h"

// the next two routines in lieu of new and free memory allocation routines
volatile static int list_lock=0;
static int list_newNode(List_t *List, Fxn_t funct, Ptr state, Ptr data, int nice)
{   int ii;
	while(list_lock); list_lock=1;
	//
    for(ii=0;ii<List->node_num;ii++)
        if(List->nodeList[ii].job.funct==0)
        {   List->nodeList[ii].job.funct=funct;
            List->nodeList[ii].job.state=state;
            List->nodeList[ii].job.data=data;
            List->nodeList[ii].job.nice=nice;
            List->nodeList[ii].meta=0;
			//
			list_lock=0;
			return ii;
        }
	//
	list_lock=0;
  return -1;
}

static void list_freeNode(Node_t *temp)
{	while(list_lock); list_lock=1;
//
	temp->job.nice=MNICE;
	temp->next=NULL;
	temp->job.funct=0;
//
	list_lock=0;
}
// end of memory allocation routines

void list_init(List_t *List, Node_t *nodeList, int node_num)
{   List->head=(Node_t *) 0;
    List->nodeList=nodeList;
	  List->node_num=node_num;
}

static int list_count(List_t *List)
{   Node_t *n=List->head;
    int c=0; while(n!=NULL) { n=n->next; c++; }
    return c;
}

int node_count(List_t *List)
{	int ii;
	int c=0;
	for(ii=0;ii<List->node_num;ii++)  if(List->nodeList[ii].job.funct) c++;
	return c;
}

// list insertion routines
static int list_append(List_t *List, Fxn_t funct, Ptr state, Ptr data, int nice)
{	int ii;
    Node_t *temp,*right;

	// find last
    right=(Node_t *)List->head;
    while(right->next != NULL) right=right->next;

  ii= list_newNode(List, funct,state,data,nice); 
  if(ii>0)
	{
		temp=&List->nodeList[ii];
		right->next =temp;
		right=temp;
		right->next=NULL;
	}	
	
	return ii;
}

static int list_add(List_t *List,  Fxn_t funct, Ptr state, Ptr data ,int nice)
{	int ii;
    Node_t *temp;
    ii=list_newNode(List, funct,state,data,nice); 
	if(ii>=0)
	{
		temp=&List->nodeList[ii];
		if (List->head== NULL)
		{   List->head=temp;
			List->head->next=NULL;
		}
		else
		{   temp->next=List->head;
			List->head=temp;
		}
	}
	return ii;
}

static int list_addafter(List_t *List, Fxn_t funct, Ptr state, Ptr data, int nice, int loc)
{
    int ii;
    Node_t *temp,*left,*right;
	
	// find insertion point
    right=List->head;   for(ii=1;ii<loc;ii++) {  left=right; right=right->next; }
    //
    ii=list_newNode(List,funct,state,data,nice); 
	if(ii>=0)
	{
		temp=&List->nodeList[ii];
		//put between left and right
		left->next=temp;
		left=temp;
		left->next=right;
	}
    return ii;
}

int list_insert(List_t *List, Fxn_t funct, Ptr state, Ptr data, int nice)
{
    int c=0;
    Node_t *temp;
	
    temp=List->head;
    if(temp==NULL)
    {   return list_add(List, funct,state,data,nice);
    }
    else
    {   // find nice insertion point
		while(temp!=NULL)
        {   if(temp->job.nice<=nice) c++;
            temp=temp->next;
        }
		//
        if(c==0)
            return list_add(List, funct,state,data,nice);
        else if(c<list_count(List))
            return list_addafter(List, funct,state,data,nice, ++c);
        else
            return list_append(List, funct,state,data,nice);
    }
}

// lremove node from list
int list_remove(List_t *List, Fxn_t funct)
{   Node_t *temp, *prev;
	prev=List->head;
    temp=prev;
    while(temp!=NULL)
    {   if(temp->job.funct==funct)
        {   // remove from linked list
			if(temp==List->head)
            {   List->head=temp->next;
            }
            else
            {   prev->next=temp->next;
            }
            // mark slot as free
			list_freeNode(temp);
			return 1; // remove was successful
        }
		prev= temp;
		temp= temp->next;
    }
    return 0; // not found
}
