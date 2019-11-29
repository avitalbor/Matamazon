
#include "amount_set.h"
#include <stdlib.h>
#include <assert.h>
#include <stdio.h>

typedef struct set_Container{
    ASElement element;;
    double quantity;
    struct set_Container* nextContainer;
} *Set_Container;

typedef struct AmountSet_t{
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
    AmountSet set=malloc(sizeof(AmountSet));
    if(set==NULL){
        return NULL;
    }
    Set_Container dameContainer= malloc(sizeof(Set_Container));
    if(!dameContainer){
        free(set);
        return NULL;
    }
    dameContainer->quantity=0;
    dameContainer->nextContainer=NULL;
    set ->copyElement= copyElement;
    set ->compareAsElements= compareElements;
    set ->freeAsElement= freeElement;
    set ->amountSetContainer= dameContainer;
    set ->iterator=NULL;
    set->size_of_Set=0;

    return set;
}

//frees all the elements and containers of the set except the dame container;
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
        free(set->amountSetContainer);
        free(set);
    }
}

AmountSet asCopy(AmountSet set){

}
int asGetSize(AmountSet set){
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

}

AmountSetResult asRegister(AmountSet set, ASElement element){
    set->iterator==NULL;//because iterator undefinde after calling this function
    if(!set || !element){
        return AS_NULL_ARGUMENT;
    }
    if(asContains(set,element)){
        return  AS_ITEM_ALREADY_EXISTS;
    }
    Set_Container newContainer= malloc(sizeof(Set_Container));
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
        }
        tmp=tmp->nextContainer;
    }
    //end of the set
    tmp->nextContainer=newContainer;
    newContainer->nextContainer=NULL;
    return  AS_SUCCESS;
}

AmountSetResult asChangeAmount(AmountSet set, ASElement element, const double amount){

}
AmountSetResult asDelete(AmountSet set, ASElement element){
    set->iterator==NULL;
    if(!asContains(set,element)){
        AS_ITEM_DOES_NOT_EXIST;
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


