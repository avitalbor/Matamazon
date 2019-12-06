
#include "amount_set.h"
#include <stdlib.h>
#include <assert.h>
#define MIN_AMOUNT 0
#define ELEMENTS_ARE_EQUAL 0


//#include <stdio.h>
/**
 * struct to be used by the AmountSet struct,
 * one container per element. keeps ASElement element,
 * the quantity of the element in the AmountSet,
 * and a pointer for the next container
 */
typedef struct set_Container{
    ASElement element;
    double quantity;
    struct set_Container* next_container;
} *Set_Container;

struct AmountSet_t{
    CopyASElement copyElement;
    FreeASElement freeAsElement;
    CompareASElements compareAsElements;
    Set_Container amount_set_container;
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
    dummy_container->next_container=NULL;
    dummy_container->element=NULL;
    set->copyElement= copyElement;
    set->compareAsElements= compareElements;
    set->freeAsElement= freeElement;
    set->amount_set_container= dummy_container;
    set->iterator=NULL;
    set->size_of_Set=0;

    return set;
}

/**
 * freeElements: frees all the elements and containers of the set
 * except for the dummy container.
 */

static void freeElements(AmountSet set){
    if(!set ||!set->amount_set_container){
        return;
    }

    while((set->amount_set_container->next_container)!=NULL){
        Set_Container tmp=set->amount_set_container->next_container;
        set->amount_set_container->next_container=(tmp->next_container);
        set->freeAsElement(tmp->element);
        free(tmp);
    }
}

void asDestroy(AmountSet set) {
    if(set!=NULL){
        freeElements(set);
        assert(set->amount_set_container);
        Set_Container tmp=set->amount_set_container;
        free(tmp);
        free(set);
    }
}

/**
 * scCopy: receives a set and a container in the set
 * and returns a copy container of the received container
 * * @return
 *     NULL if  a memory allocation failed.
 *     A copy container of the received container if the process was successful
 */
static Set_Container scCopy(AmountSet set, Set_Container container){
    assert(set && container);
    Set_Container container_copy = malloc(sizeof(*container_copy));
    if(!container_copy){
        return NULL;
    }
    container_copy->element = set->copyElement(container->element);
    // in case the copyElement function returns a NULL argument
    if(!container_copy->element){
        free(container_copy);
        return NULL;
    }
    container_copy->quantity = container->quantity;
    container_copy->next_container = NULL;
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
    dummy_container_copy->next_container=NULL;
    set_copy->copyElement = set->copyElement;
    set_copy->compareAsElements = set->compareAsElements;
    set_copy->freeAsElement = set->freeAsElement;
    set_copy->amount_set_container = dummy_container_copy;
    set_copy->iterator = NULL;
    set_copy->size_of_Set = 0;

    Set_Container tmp = set->amount_set_container->next_container;
    Set_Container current_container_of_copy=set_copy->amount_set_container;

    while (tmp){
        current_container_of_copy->next_container=scCopy(set,tmp);
        //check if allocation failed
        if(!current_container_of_copy->next_container){
            asDestroy(set_copy);
            return NULL;
        }
        tmp=tmp->next_container;
        current_container_of_copy=current_container_of_copy->next_container;
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
    Set_Container current_container=set->amount_set_container->next_container;
    while (current_container){
        if(set->compareAsElements(current_container->element,element)
        ==ELEMENTS_ARE_EQUAL){
            return true;//found the element in the set
        }
        current_container=current_container->next_container;
    }
    //searched over all the containers and didn't found the element
    return false;
}

AmountSetResult asGetAmount(AmountSet set, ASElement element, double *outAmount){
    if(!set || !element || !outAmount) {
        return AS_NULL_ARGUMENT;
    }
    if(!asContains(set,element)){
        return AS_ITEM_DOES_NOT_EXIST;
    }
    Set_Container tmp = set->amount_set_container->next_container;
    while (tmp){
        if(set->compareAsElements(tmp->element,element)==ELEMENTS_ARE_EQUAL){
            *outAmount=tmp->quantity;
            break;
        }
        tmp=tmp->next_container;
    }
    return AS_SUCCESS;
}

AmountSetResult asRegister(AmountSet set, ASElement element){
    if(!set || !element){
        return AS_NULL_ARGUMENT;
    }
    set->iterator = NULL;//iterator is undefined after this function
    if(asContains(set,element)){
        return  AS_ITEM_ALREADY_EXISTS;
    }
    Set_Container new_container= malloc(sizeof(*new_container));
    if(!new_container){
        return AS_OUT_OF_MEMORY;
    }
    new_container->quantity=0;
    new_container->element=set->copyElement(element);
    if(!new_container->element){
        free(new_container);
        return AS_OUT_OF_MEMORY;
    }
    new_container->next_container=NULL;
    set->size_of_Set=set->size_of_Set+1;//added an element to the amountSet

    if(!(set->amount_set_container->next_container)){
        set->amount_set_container->next_container=new_container;
        return  AS_SUCCESS;
    }
    Set_Container tmp= set->amount_set_container;
    while(tmp->next_container){
        if(set->compareAsElements((tmp->next_container)->element,element)>0){
            new_container->next_container=tmp->next_container;
            tmp->next_container=new_container;
            return  AS_SUCCESS;
        }
        tmp=tmp->next_container;
    }
    /*if we are here:
    tmp is the last container and the element in new container is the biggest*/
    tmp->next_container=new_container;
    return  AS_SUCCESS;
}

AmountSetResult asChangeAmount(AmountSet set, ASElement element, const double amount){
    if(!set || !element) {
        return AS_NULL_ARGUMENT;
    }
    if(!asContains(set,element)){
        return AS_ITEM_DOES_NOT_EXIST;
    }
    Set_Container tmp = set->amount_set_container->next_container;
    while (tmp){
        if(set->compareAsElements(tmp->element,element)==ELEMENTS_ARE_EQUAL){
            if((tmp->quantity)+amount<MIN_AMOUNT){
                return AS_INSUFFICIENT_AMOUNT;
            } else{
                //amount is good
                tmp->quantity=tmp->quantity+amount;
                break;
            }
        }
        tmp=tmp->next_container;
    }
    return AS_SUCCESS;
}

AmountSetResult asDelete(AmountSet set, ASElement element){
    if(!set|| !element){
        return AS_NULL_ARGUMENT;
    }
    set->iterator = NULL;
    if(!asContains(set,element)){
        return AS_ITEM_DOES_NOT_EXIST;
    }
    Set_Container tmp1= set->amount_set_container;
    Set_Container tmp2=NULL;

    while (set->compareAsElements(tmp1->next_container->element,element)
           !=ELEMENTS_ARE_EQUAL){
        tmp1=tmp1->next_container;
    }
    assert(set->compareAsElements(tmp1->next_container->element,element)==0);
    tmp2=(tmp1->next_container)->next_container;
    //now the container of the given element is between tmp1 and tmp2

    set->freeAsElement(tmp1->next_container->element);
    free(tmp1->next_container);
    tmp1->next_container=tmp2;
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
    if( !set || !(set ->amount_set_container)
                       || !(set->amount_set_container->next_container)){
        return NULL;
    }
    set->iterator=set->amount_set_container->next_container;
    return set->iterator->element;
}

ASElement asGetNext(AmountSet set){
    if(!set ||!(set->iterator)||!(set->iterator->next_container)){
        return NULL;
    }
    set->iterator=set->iterator->next_container;
    assert(set->iterator->element);
    return set->iterator->element;
}

