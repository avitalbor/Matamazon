
#include "amount_set.h"
#include <stdlib.h>
#include <assert.h>
#define MIN_AMOUNT 0



//#include <stdio.h>

typedef struct set_Container{
    ASElement element;
    double quantity;
    struct set_Container* nextContainer;
} *Set_Container;

struct AmountSet_t{
    CopyASElement copyElement;
    FreeASElement freeAsElement;
    CompareASElements compareAsElements;
    Set_Container amountSetContainer;
    Set_Container iterator;
    int size_of_Set;
};

AmountSet asCreate(CopyASElement copyElement,
                   FreeASElement freeElement,
                   CompareASElements compareElements){
    if(!copyElement || !freeElement || !compareElements){
        return NULL;
    }
    AmountSet set=malloc(sizeof(*set));
    if(set==NULL){
        return NULL;
    }
    assert(set);
    Set_Container dummy_container= malloc(sizeof(*dummy_container));
    if(!dummy_container){
        free(set);
        return NULL;
    }
    assert(dummy_container);
    dummy_container->quantity=0;
    dummy_container->nextContainer=NULL;
    dummy_container->element=NULL;
    set->copyElement= copyElement;
    set->compareAsElements= compareElements;
    set->freeAsElement= freeElement;
    set->amountSetContainer= dummy_container;
    set->iterator=NULL;
    set->size_of_Set=0;

    return set;
}

/**
 * freeElements: frees all the elements and containers of the set except for the dummy container.
 */

// frees all the elements and containers of the set except for the dummy container
static void freeElements(AmountSet set){
    assert(set);
    assert(set->amountSetContainer!=NULL);
    while((set->amountSetContainer->nextContainer)!=NULL){
        Set_Container tmp=set->amountSetContainer->nextContainer;
        set->amountSetContainer->nextContainer=(tmp->nextContainer);
        set->freeAsElement(tmp->element);
        free(tmp);
    }
}

void asDestroy(AmountSet set) {
    if(set!=NULL){
        freeElements(set);
        assert(set->amountSetContainer);
        Set_Container tmp=set->amountSetContainer;
        free(tmp);
        free(set);
    }
}

/**
 * scCopy: receives a set and a container in the set and returns a copy container of the received container
 * * @return
 *     NULL if  a memory allocation failed.
 *     A copy container of the received container
 */
static Set_Container scCopy(AmountSet set, Set_Container Container){
    Set_Container container_copy = malloc(sizeof(*container_copy));
    if(!container_copy){
        return NULL;
    }
    container_copy->element = set->copyElement(Container->element);

    if(!container_copy->element){
        free(container_copy);
        return NULL;
    }

    container_copy->quantity = Container->quantity;
    container_copy->nextContainer = NULL;
    return container_copy;
}

AmountSet asCopy(AmountSet set){
    if(!set){
        return NULL;
    }
    AmountSet set_copy = malloc(sizeof(*set_copy));
    if(set_copy == NULL){
        return NULL;
    }
    Set_Container dummy_container_copy = malloc(sizeof(*dummy_container_copy));
    if(!dummy_container_copy){
        free(set_copy);
        return NULL;
    }
    dummy_container_copy->quantity=0;
    dummy_container_copy->nextContainer=NULL;
    set_copy->copyElement = set->copyElement;
    set_copy->compareAsElements = set->compareAsElements;
    set_copy->freeAsElement = set->freeAsElement;
    set_copy->amountSetContainer = dummy_container_copy;
    set_copy->iterator = NULL;
    set_copy->size_of_Set = 0;



    Set_Container tmp = set->amountSetContainer->nextContainer;
    Set_Container current_container_of_copy=set_copy->amountSetContainer;

    while (tmp){
        current_container_of_copy->nextContainer=scCopy(set,tmp);
        if(!current_container_of_copy->nextContainer){
            asDestroy(set_copy);
            return NULL;
        }
        tmp=tmp->nextContainer;
        current_container_of_copy=current_container_of_copy->nextContainer;
    }
    set_copy->iterator = NULL;
    set->iterator = NULL;
    set_copy->size_of_Set=set->size_of_Set;
    return set_copy;
}

int asGetSize(AmountSet set){
    if(set == NULL){
        return -1;
    }
    return set->size_of_Set;
}

bool asContains(AmountSet set, ASElement element)
{
    if(!set || !element){
        return false;
    }
    Set_Container current_container=set->amountSetContainer->nextContainer;
    while (current_container){
        if(set->compareAsElements(current_container->element,element)==0){
            return true;
        }
        current_container=current_container->nextContainer;
    }
    return false;
}

AmountSetResult asGetAmount(AmountSet set, ASElement element, double *outAmount){
    if(!set || !element || !outAmount) {
        return AS_NULL_ARGUMENT;
    }
    if(!asContains(set,element)){
        return AS_ITEM_DOES_NOT_EXIST;
    }
    Set_Container tmp = set->amountSetContainer->nextContainer;
    while (tmp){
        if(set->compareAsElements(tmp->element,element)==0){
            *outAmount=tmp->quantity;
            return AS_SUCCESS;
        }
        tmp=tmp->nextContainer;
    }

}

AmountSetResult asRegister(AmountSet set, ASElement element){

    if(!set || !element){
        return AS_NULL_ARGUMENT;
    }
    set->iterator = NULL;//because iterator undefinde after calling this function
    if(asContains(set,element)){
        return  AS_ITEM_ALREADY_EXISTS;
    }
    Set_Container newContainer= malloc(sizeof(*newContainer));
    if(!newContainer){
        return AS_OUT_OF_MEMORY;
    }
    newContainer->quantity=0;
    newContainer->element=set->copyElement(element);
    if(!newContainer->element){
        free(newContainer);
        return AS_OUT_OF_MEMORY;
    }
    newContainer->nextContainer=NULL;
    set->size_of_Set=set->size_of_Set+1;
    if(!(set->amountSetContainer->nextContainer)){
        set->amountSetContainer->nextContainer=newContainer;
        return  AS_SUCCESS;
    }
    Set_Container tmp= set->amountSetContainer;
    while(tmp->nextContainer){
        if(set->compareAsElements((tmp->nextContainer)->element,element)>0){
            newContainer->nextContainer=tmp->nextContainer;
            tmp->nextContainer=newContainer;
            return  AS_SUCCESS;
        }
        tmp=tmp->nextContainer;
    }
    tmp->nextContainer=newContainer;
    newContainer->nextContainer=NULL;
    return  AS_SUCCESS;
}

AmountSetResult asChangeAmount(AmountSet set, ASElement element, const double amount){
    if(!set || !element) {
        return AS_NULL_ARGUMENT;
    }
    if(!asContains(set,element)){
        return AS_ITEM_DOES_NOT_EXIST;
    }
    Set_Container tmp = set->amountSetContainer->nextContainer;
    while (tmp){
        if(set->compareAsElements(tmp->element,element)==0)
        {
            if((tmp->quantity)+amount<MIN_AMOUNT){
                return AS_INSUFFICIENT_AMOUNT;
            } else{
                tmp->quantity=tmp->quantity+amount;
                return AS_SUCCESS;
            }
        }
        tmp=tmp->nextContainer;
    }
}

AmountSetResult asDelete(AmountSet set, ASElement element){
    if(!set|| !element){
        return AS_NULL_ARGUMENT;
    }
    set->iterator = NULL;
    if(!asContains(set,element)){
        return AS_ITEM_DOES_NOT_EXIST;
    }

    Set_Container tmp1= set->amountSetContainer;
    Set_Container tmp2= set->amountSetContainer;

    while (set->compareAsElements(tmp1->nextContainer->element,element)!=0){
        tmp1=tmp1->nextContainer;
    }
    assert(set->compareAsElements(tmp1->nextContainer->element,element)==0);
    tmp2=(tmp1->nextContainer)->nextContainer;
    set->freeAsElement(tmp1->nextContainer->element);
    free(tmp1->nextContainer);
    tmp1->nextContainer=tmp2;
    set->size_of_Set=(set->size_of_Set)-1;
    return AS_SUCCESS;
}

AmountSetResult asClear(AmountSet set){
    if(!set){
        return AS_NULL_ARGUMENT;
    }
    freeElements(set);
    set->size_of_Set=0;
    return AS_SUCCESS;
}

ASElement asGetFirst(AmountSet set){
    if( !set || !(set ->amountSetContainer) || !(set->amountSetContainer->nextContainer)){
        return NULL;
    }
    set->iterator=set->amountSetContainer->nextContainer;
    return set->iterator->element;
}

ASElement asGetNext(AmountSet set){
    if(!set ||!(set->iterator)||!(set->iterator->nextContainer)){
        return NULL;
    }
    set->iterator=set->iterator->nextContainer;
    assert(set->iterator->element);
    return set->iterator->element;
}

