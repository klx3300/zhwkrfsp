#ifndef Q_RE_LIST_H
#define Q_RE_LIST_H

#include "listd.h"

qListDescriptor qList_constructor();

#define qList_push_back(desc,target) q__List_push_back(&(desc),&(target),sizeof(target))
int q__List_push_back(void* descriptor,void* target,unsigned int target_size);
#define qList_push_front(desc,target) q__List_push_front(&(desc),&(target),sizeof(target))
int q__List_push_front(void* descriptor,void* target,unsigned int target_size);
#define qList_pop_back(desc) q__List_pop_back(&(desc))
int q__List_pop_back(void* descriptor);
#define qList_pop_front(desc) q__List_pop_front(&(desc))
int q__List_pop_front(void* descriptor);
#define qList_erase_elem(desc,iter) q__List_erase_elem(&(desc),iter)
int q__List_erase_elem(void* descriptor,qListIterator iter);
#define qList_swap_elem(desc,itera,iterb) q__List_swap_elem(&(desc),itera,iterb)
int q__List_swap_elem(void* descriptor,qListIterator itera,qListIterator iterb);
#define qList_islist(desc) q__List_islist(&(desc),sizeof(desc))
int q__List_islist(void* descriptor,unsigned int size);
#define qList_copy(desc) q__List_copy(&(desc))
qListDescriptor q__List_copy(struct q__ListDescriptor *desc);
// notice: unlike cpp behaivor, the destructor here plays the role of
// clear the whole list and set the necessary parameters to its starting status.
#define qList_destructor(desc) q__List_destructor(&desc)
int q__List_destructor(void* descriptor);

qListIterator qList_begin(qListDescriptor desc);
qListIterator qList_end(qListDescriptor desc);

qListIterator qListIterator_prev(qListIterator iter);
qListIterator qListIterator_next(qListIterator iter);

int qListIterator_isvalid(qListIterator iter);
void* qListIterator_deref(qListIterator iter);

#define qList_foreach(desc,iter) for(qListIterator (iter)=qList_begin(desc);qListIterator_isvalid(iter);(iter)=qListIterator_next(iter))

#endif