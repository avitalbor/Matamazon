
#include "amount_set.h"
#include <stdlib.h>
#include <assert.h>


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
    Set_Container dummyContainer= malloc(sizeof(*dummyContainer));
    if(!dummyContainer){
        free(set);
        return NULL;
    }
    assert(dummyContainer);
    dummyContainer->quantity=0;
    dummyContainer->nextContainer=NULL;
    dummyContainer->element=NULL;
    set->copyElement= copyElement;
    set->compareAsElements= compareElements;
    set->freeAsElement= freeElement;
    set->amountSetContainer= dummyContainer;
    set->iterator=NULL;
    set->size_of_Set=0;

    return set;
}

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

static Set_Container scCopy(AmountSet set, Set_Container Container, Set_Container Last_Container){
    Set_Container Container_copy = malloc(sizeof(*Container_copy));
    if(!Container_copy){
        return NULL;
    }
    Container_copy->element = set->copyElement(Container->element);
    Container_copy->quantity = Container->quantity;
    Container_copy->nextContainer = NULL;
    Last_Container->nextContainer = Container_copy;
    return Container_copy;
}

AmountSet asCopy(AmountSet set){
    AmountSet set_copy = malloc(sizeof(*set_copy));
    if(set_copy == NULL){
        return NULL;
    }
    Set_Container dummyContainer_copy = malloc(sizeof(*dummyContainer_copy));
    if(!dummyContainer_copy){
        free(set_copy);
        return NULL;
    }
    dummyContainer_copy->quantity=0;
    dummyContainer_copy->nextContainer=NULL;
    set_copy->copyElement = set->copyElement;
    set_copy->compareAsElements = set->compareAsElements;
    set_copy->freeAsElement = set->freeAsElement;
    set_copy->amountSetContainer = dummyContainer_copy;
    set_copy->iterator = NULL;
    set_copy->size_of_Set = 0;
    Set_Container tmp = dummyContainer_copy;
    AS_FOREACH(ASElement ,currentElement,set){
        if(scCopy(set_copy, currentElement, tmp) == NULL){
            freeElements(set_copy);
            free(dummyContainer_copy);
            free(set_copy);
            return NULL;
        }
        tmp = tmp->nextContainer;
    }
    set_copy->iterator = NULL;
    set->iterator = NULL;
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
    if(!set){
        return false;
    }
    AS_FOREACH(ASElement ,currentElement,set){
        if(set->compareAsElements(currentElement,element)==0){
            return true;
        }
    }
    return false;
}

AmountSetResult asGetAmount(AmountSet set, ASElement element, double *outAmount){
    if(!set || !element || !*outAmount) {
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
    /*Set_Container tmp = set->iterator;
    AS_FOREACH(ASElement ,currentElement,set){
        if(set->compareAsElements(currentElement,element)==0){
            *outAmount = set->iterator->quantity;
        }
        break;
    }
    set->iterator = tmp;
    return AS_SUCCESS;*/
}

AmountSetResult asRegister(AmountSet set, ASElement element){
    set->iterator = NULL;//because iterator undefinde after calling this function
    if(!set || !element){
        return AS_NULL_ARGUMENT;
    }
    if(asContains(set,element)){
        return  AS_ITEM_ALREADY_EXISTS;
    }
    Set_Container newContainer= malloc(sizeof(*newContainer));
    if(!newContainer){
        return AS_OUT_OF_MEMORY;
    }
    newContainer->quantity=0;
    newContainer->element=element;
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
            if((tmp->quantity)+amount<0){
                return AS_INSUFFICIENT_AMOUNT;
            } else{
                tmp->quantity=tmp->quantity+amount;
                return AS_SUCCESS;
            }
        }
        tmp=tmp->nextContainer;
    }
    //should not get here
}

AmountSetResult asDelete(AmountSet set, ASElement element){
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
    tmp2=tmp1->nextContainer->nextContainer;
    set->freeAsElement(tmp1->nextContainer->element);
    free(tmp1->nextContainer);
    tmp1->nextContainer=tmp2;
    set->size_of_Set=set->size_of_Set-1;
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

