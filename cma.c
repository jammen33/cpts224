#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

#include "cma.h"
#include "debug.h"

#define ITEMNOTFOUND ((void *)-1)

static void *class_membase=NULL;
static void *class_limit=NULL;
static MNode class_inuse=NULL;
static MNode class_nouse=NULL;

static struct {
  unsigned long malloc,calloc,realloc,free,gc,nomem;
} class_counters={0,0,0,0,0,0};

static MNode class_AddToList(MNode list, MNode item) {
  ENTER("class_AddToList");
  
  item->next = list;
  EXIT("class_AddToList");
  RETURN(item);
}

static MNode class_RemoveFromList(MNode list,MNode item) {
  ENTER("class_RemoveFromList");
  MNode p,prev;

  prev = NULL;
  for (p = list; p!=NULL; p = p->next) {
    if (p == item) {
      if (prev == NULL)
	list=p->next;
      else
	prev->next = p->next;
      EXIT("class_RemoveFromList");
      RETURN(list);
    }
    prev=p;
  }
  //not in list..
  EXIT("class_RemoveFromList");
  RETURN(ITEMNOTFOUND);
}


static void class_printList(MNode list) {
  ENTER("class_printList");
  if (!list) {
    EXIT("class_printList");
    return;
  }
  printf("Node %p, %d\n",list,list->size);
  class_printList(list->next);
  EXIT("class_printList");
}

int class_memory(void *mem, size_t size) {
  ENTER("class_memory");
  
  MNode item;
  if (class_membase ) {
    RETURN(FALSE);
    EXIT("class_memory");
  }

  class_membase = mem;
  class_limit = mem+size;
  
  //setup initial list item
  item = (MNode)mem;
  item->size=size-sizeof(struct MemNode);
  item->next = NULL;
  DEBUG("Creating Initial NoUseList with %x: %ud",item,size);
  class_nouse = class_AddToList(class_nouse,item);
  EXIT("class_memory");
}

void *class_calloc(size_t nmemb, size_t size) {
  ENTER("class_calloc");
  
  void *mem;

  mem = class_malloc(nmemb*size);
  memset(mem,0,nmemb*size);
  class_counters.calloc += 1;
  
  EXIT("class_memory");
  RETURN(mem);
}

static MNode class_findNoUse(size_t target) {
  ENTER("class_findNoUse");
  
  size_t closeness=LONG_MAX;
  size_t c;
  MNode best=NULL;
  MNode p;

  DEBUG("Searching for a block of size: %ud",target);
  for (p=class_nouse;p!=NULL;p=p->next) {
    c = p->size - target;
    if (c >= 0 && c<closeness) {
      best = p;
      closeness=c;
      DEBUG("Best is now: %x size=%ud",best,p->size);
    }
  }
  EXIT("class_memory");
  RETURN(best);
}

MNode class_splitNode(MNode org,size_t size) {
  ENTER("class_splitNode");
  
	MNode extra=NULL;
	size_t orgsz = org->size;
	
	//we need room for a new header
	if ( (orgsz-size-sizeof(struct MemNode)) > 0 ) {
		DEBUG("Node split: %ud => %ud,%ud",org->size,size,orgsz-sizeof(struct MemNode)-size);
		org->size = size;
		extra = (MNode)((void*)org+size+sizeof(struct MemNode));
		extra->next = 0;
		extra->size = orgsz-sizeof(struct MemNode)-size;
	}
	else
		DEBUG("Node does not have enough size to split:%ud %ud",org->size,size);
	
	EXIT("class_memory");
	RETURN(extra);
}

void *class_malloc(size_t size) {
  ENTER("class_malloc");
  
  MNode newnode = NULL,extra = NULL;

  newnode = class_findNoUse(size);

  if (newnode) {
    class_nouse=class_RemoveFromList(class_nouse,newnode);//assume it is there since we just found it there

    //split the node..
    extra = class_splitNode(newnode,size);
    if (extra)
      class_nouse=class_AddToList(class_nouse,extra);
    
    newnode->next = NULL;
    class_inuse = class_AddToList(class_inuse,newnode);
    class_counters.malloc += 1;
    RETURN( (void *)newnode+sizeof(struct MemNode));
  }
  else {
    EXIT("class_memory");
    RETURN( NULL );
  }
}

//attempt to find adjacent unused nodes and collapse them.
static void class_garbage() {
  ENTER("class_garbage");
  
  MNode here = NULL, there = NULL;
  here = class_nouse;

  
  #define NXT(e) ((void*)e+e->size+sizeof(struct MemNode))
  #define TXN(e) ((void*)e->size+sizeof(struct MemNode))
  while(here != NULL)
  {
    there = here->next;    
    if((NXT(here)) == there)
    {
      here->size = here->size + TXN(there);
      class_RemoveFromList(class_nouse, there);
      class_counters.gc += 1;
      break;
    }    
    here = here->next;
  }
  EXIT("class_memory");
}

void class_free(void *ptr) {
  ENTER("class_free");
  
  MNode cur=NULL;
  if (!ptr) {
  EXIT("class_memory");
    return;
  }
 
  cur=class_RemoveFromList(class_inuse,PTRTOMNODE(ptr));
  if (cur==ITEMNOTFOUND) {//not our pointer
    EXIT("class_memory");
    return;    
  }
  class_inuse = cur;
  class_nouse = class_AddToList(class_nouse,PTRTOMNODE(ptr));
  class_garbage();
  class_counters.free += 1;
}

void *class_realloc(void *ptr, size_t size) {
  ENTER("class_realloc");
  
  void *mem;
  size_t oldsize;

  mem=class_malloc(size);
  if (!mem) {
    EXIT("class_memory");
    RETURN( NULL );
  }

  oldsize=PTRTOMNODE(ptr)->size;
  memcpy(mem,ptr,oldsize);

  class_free(ptr);
  class_counters.realloc += 1;
  
  EXIT("class_memory");
  RETURN( mem );
}

void class_stats() {
  ENTER("class_stats");
  
  printf("InUse\n");
  class_printList(class_inuse);

  printf("NoUse\n");
  class_printList(class_nouse);

  printf("Counters:\n");
#define DUMPC(x) printf(" %10s : %ld\n",#x,class_counters.x)
  DUMPC(malloc);
  DUMPC(calloc);
  DUMPC(realloc);
  DUMPC(free);
  DUMPC(gc);
  DUMPC(nomem);
#undef DUMPC
}

