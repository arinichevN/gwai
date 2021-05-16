#ifndef LIBPAS_DSTRUCTURE_H
#define LIBPAS_DSTRUCTURE_H


#include <stdlib.h>

#define LIST_ITEM_SIZE(list) sizeof (*(list)->items)

#define DEC_LIST(T) typedef struct {T *items; size_t length;size_t max_length;} T##List;
#define LIST_INITIALIZER {.items = NULL, .length = 0, .max_length = 0}
#define LIST_RESET(list) (list)->items=NULL; (list)->length=0; (list)->max_length=0;
#define LIST_FREE(list) free((list)->items); LIST_RESET(list)
#define LIST_CLEAR(list) memset((list)->items,0,LIST_ITEM_SIZE(list) * (list)->max_length );(list)->length=0;
#define LIST_RESIZE(list, new_length)  if((list)->max_length < new_length){(list)->items = realloc((list)->items, LIST_ITEM_SIZE(list) * new_length );if ((list)->items == NULL) {(list)->max_length=0;(list)->length=0;}else{(list)->max_length=new_length;}}
#define LIST_PUSH(list, min_alloc_length, new_item) if((list)->max_length<(list)->length+1){if(min_alloc_length<1)return 0;(list)->items=realloc((list)->items, LIST_ITEM_SIZE(list) * ((list)->max_length+min_alloc_length));if((list)->items==NULL){(list)->max_length=0;(list)->length=0;return 0;}(list)->max_length+=min_alloc_length;}(list)->items[(list)->length]=*(new_item);(list)->length++;
//call it for empty lists
#define LIST_ALLOC(list, length) if(length>0){(list)->items = calloc(length, LIST_ITEM_SIZE(list));if ((list)->items != NULL)(list)->max_length=length;}

#define LIST_GETBYFIELD(FIELD, DEST,LIST,NEEDLE) (DEST) = NULL;for (size_t I = 0; I < (LIST)->length; I++) {if ((LIST)->items[I].FIELD == (NEEDLE)) {(DEST) = (LIST)->items + I;}}
#define LIST_GETBYID(DEST,LIST,ID) LIST_GETBYFIELD(id, DEST,LIST,ID)
#define LIST_GETBYIDSTR(DEST,LIST,ID) (DEST) = NULL;for (size_t I = 0; I < (LIST)->length; I++) {if (strcmp((LIST)->items[I].id, ID)==0) {(DEST) = (LIST)->items + I;}}


#define DEC_PLIST(T) typedef struct {T **item; size_t length;size_t max_length;} T##PList;
#define DEC_FUN_LIST_INIT(T)  extern int init ## T ## List(T ## List *list, unsigned int n);
//#define DEC_FUN_LIST_GET_BY_ID(T) extern T *get ## T ## ById(int id, const T ## List *list);
//#define DEC_FUN_LIST_GET_BY_IDSTR(T) extern T *get ## T ## ById(char *id, const T ## List *list);
//#define DEC_FUN_LLIST_GET_BY_ID(T) extern T *get ## T ## ById(int id, const T ## List *list);



#define DEC_LLIST(T) typedef struct {T *top; T *last; size_t length;} T##LList;
#define DEC_LLISTM(T) typedef struct {T *top; T *last; size_t length; Mutex mutex;} T##LListm;

#define LLIST_INITIALIZER {.top = NULL, .last = NULL, .length = 0}
#define LLISTM_INITIALIZER {.top = NULL, .last = NULL, .length = 0, .mutex = MUTEX_INITIALIZER}
#define LLIST_RESET(list) (list)->top = NULL; (list)->last = NULL; (list)->length = 0;
//#define LLIST_GETBYID(DEST,LIST,ID)(DEST) = (LIST)->top;while((DEST)!=NULL){if((DEST)->id==ID){break;}(DEST)=(DEST)->next;}
#define LLIST_GETBYID(DEST,LIST,ID)for((DEST) = (LIST)->top;(DEST)!=NULL;(DEST)=(DEST)->next)if((DEST)->id==ID)break;

#define FOREACH_LLIST(ITEM,LIST,T) for(T * ITEM=(LIST)->top;ITEM!=NULL;ITEM=ITEM->next)
#define FORLLi(T) FOREACH_LLIST(item,list,T)
#define LLIST_DEL_ITEM(ITEM, LIST, T) {T *prev = NULL;FOREACH_LLIST ( curr, LIST, T ) {if ( curr == (ITEM) ) {if ( prev != NULL ) {prev->next = curr->next;} else {(LIST)->top = curr->next;}if ( curr == (LIST)->last ) {(LIST)->last = prev;}(LIST)->length--; break;}prev = curr;}}
#define LLIST_ADD_ITEM(ITEM, LIST) if ((LIST)->length < INT_MAX ) {if ( (LIST)->top == NULL ) {(LIST)->top = ITEM;} else {(LIST)->last->next = ITEM;}(LIST)->last = ITEM;(LIST)->length++;}


#define FIFO_LIST_INITIALIZER {.item = NULL, .length = 0, .pop_item = NULL, .push_item = NULL}
#define DEC_FIFO_LIST(T) struct fifo_item_ ## T {T data;int free;struct fifo_item_ ## T *prev;struct fifo_item_ ## T *next;};typedef struct fifo_item_ ## T FIFOItem_ ## T;typedef struct {FIFOItem_ ## T *item;size_t length;FIFOItem_ ## T *push_item;FIFOItem_ ## T *pop_item;Mutex mutex;} FIFOItemList_ ## T;
#define FUN_FIFO_PUSH(T) int T ## _fifo_push(T item, FIFOItemList_ ## T *list) {if (!lockMutex(&list->mutex)) {return 0;}if (list->push_item == NULL) {unlockMutex(&list->mutex);return 0;}list->push_item->data = item;list->push_item->free = 0;if(list->pop_item==NULL){list->pop_item=list->push_item;}if (list->push_item->next->free) {list->push_item = list->push_item->next;} else {list->push_item = NULL;}unlockMutex(&list->mutex);return 1;}
#define FUN_FIFO_POP(T) int T ## _fifo_pop(T * item, FIFOItemList_ ## T *list) {if (!lockMutex(&list->mutex)) {return 0;}if (list->pop_item == NULL) {unlockMutex(&list->mutex);return 0;}*item = list->pop_item->data;list->pop_item->free = 1;if (list->push_item == NULL) {list->push_item = list->pop_item;}if (!list->pop_item->next->free) {list->pop_item = list->pop_item->next;} else {list->pop_item = NULL;}unlockMutex(&list->mutex);return 1;}
#define FREE_FIFO(fifo) free((fifo)->item); (fifo)->item=NULL; (fifo)->length=0; (fifo)->pop_item = NULL;(fifo)->push_item = NULL;
#define DEC_FUN_FIFO_PUSH(T) extern int T ## _fifo_push(T item, FIFOItemList_ ## T *list);
#define DEC_FUN_FIFO_POP(T) extern int T ## _fifo_pop(T * item, FIFOItemList_ ## T *list);
#define FIFO_INIT(list, count) (list)->item = malloc(LIST_ITEM_SIZE * count);if ((list)->item == NULL) {(list)->length = 0;}(list)->length = count;for (size_t i = 0; i < (list)->length; i++){if (i == ((list)->length-1)) {(list)->item[i].next = &(list)->item[0];} else {(list)->item[i].next = &(list)->item[i + 1];} if (i == 0) {(list)->item[i].prev = &(list)->item[(list)->length-1];} else {(list)->item[i].prev = &(list)->item[i - 1];} (list)->item[i].free = 1; } (list)->pop_item = NULL; (list)->push_item = &(list)->item[0]; if(!initMutex(&(list)->mutex)){FREE_FIFO(list)}

#define DEC_PIPE(TIN, TPIPE)		typedef struct {TIN *item; size_t length;} TPIPE;
#define PIPE_BEGIN(PIPE, LENGTH)	(PIPE)->item = NULL; (PIPE)->length = 0; if(LENGTH>0){(PIPE)->item = calloc(LENGTH, sizeof (*(PIPE)->item));if ((PIPE)->item != NULL)(PIPE)->length=LENGTH;}
#define PIPE_IN(PIPE, VALUE)		(PIPE)->item[0] = VALUE;
#define PIPE_OUT(PIPE)				(PIPE)->item[(PIPE)->length - 1]
#define PIPE_MOVE(PIPE, SUB)		for (size_t i=(PIPE)->length - 1; i > 0; i--) {(PIPE)->item[i] = (PIPE)->item[i - 1];}	(PIPE)->item[0] = SUB;
#define PIPE_PRINT(PIPE, FORMAT)	for(size_t i=0; i<(PIPE)->length; i++) {printf(FORMAT, (PIPE)->item[i]);}	puts("");
#define PIPE_FILL(PIPE, VALUE)		for(size_t i=0; i<(PIPE)->length; i++) {(PIPE)->item[i]=VALUE;}
#define PIPE_FREE(PIPE)				free((PIPE)->item); (PIPE)->item = NULL; (PIPE)->length = 0;

//round list. we will push first to free place, if no free place, we will update oldest items
#define DEC_RLIST(T) typedef struct {T *item;size_t next_ind;size_t length;} T ## RList;
#define FUN_RLIST_PUSH(T) void push ## T ## RList(T ## RList *list, T value) {if (list->length<=0){return;}list->item[list->next_ind] = value;if (list->next_ind < list->length - 1) {list->next_ind++;} else {list->next_ind = 0;}}
#define FUN_RLIST_INIT(T) int init ## T ## RList(T ## RList *list, size_t n) {list->length=0;list->item=NULL;if(n<=0){return 1;}list->item = calloc(n, LIST_ITEM_SIZE);if (list->item == NULL) {return 0;}list->length=n;return 1;}
#define RESET_RLIST(list) (list)->item=NULL; (list)->length=0;
#define FREE_RLIST(list) free((list)->item); RESET_RLIST(list)


#endif 

