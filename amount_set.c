
#include <stdlib.h>
#include "amount_set.h"

#define MIN_AMOUNT 0
#define ELEMENTS_ARE_EQUAL 0
#define NULL_WAS_SENT_GETSIZE -1


/**
 * SetContainer
 *
 * This is an internal struct implemented to be used by the AmountSet struct.
 * Every element inside an AmountSet is "held" by a Set_container,
 * one container per element.
 *  @param element - The element's info
 *  @param quantity - The amount of the specific element in the AmountSet
 *  @param next_container - A pointer to next container,
 *         which is the next element in the AmountSet (by the requested order).
 */
typedef struct set_Container{
    ASElement element;
    double quantity;
    struct set_Container* next_container;
} *SetContainer;

struct AmountSet_t{
    CopyASElement copyElement;
    FreeASElement freeElement;
    CompareASElements compareElements;
    SetContainer first_AS_container;
    SetContainer iterator;
    int size_of_Set;
};

/**
 * freeElements: frees all of the space the elements and containers of a
 * specific AmountSet occupie, except for the dummy container.
 *
 * @param set - An AmountSet which we want to free the space all of its elements
 * occupie.
 */
static void freeElements(AmountSet set){
    if(!set ||!set->first_AS_container){
        return;
    }

    while((set->first_AS_container->next_container)!=NULL){
        SetContainer tmp=set->first_AS_container->next_container;
        set->first_AS_container->next_container=(tmp->next_container);
        set->freeElement(tmp->element);
        free(tmp);
    }
}

/**
 * copySetContainer: receives an AmountSet and a container in the set and
 * returns a copy container of the received container.
 *
 * @param set - An AmountSet to copy the container from.
 * @param container - a SetContainer to copy.
 * @return
 *     NULL - if a memory allocation failed.
 *     A copy of the received container if the process was successfull.
 */
static SetContainer copySetContainer(AmountSet set, SetContainer container){
    SetContainer container_copy = malloc(sizeof(*container_copy));
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
    SetContainer dummy_container= malloc(sizeof(*dummy_container));
    if(!dummy_container){
        free(set);
        return NULL;
    }

    dummy_container->quantity=0;
    dummy_container->next_container=NULL;
    dummy_container->element=NULL;
    set->copyElement= copyElement;
    set->compareElements= compareElements;
    set->freeElement= freeElement;
    set->first_AS_container= dummy_container;
    set->iterator=NULL;
    set->size_of_Set=0;

    return set;
}

void asDestroy(AmountSet set) {
    if(set!=NULL){
        freeElements(set);
        SetContainer tmp=set->first_AS_container;
        free(tmp);
        free(set);
    }
}

AmountSet asCopy(AmountSet set){
    if(!set){
        return NULL;
    }
    AmountSet set_copy = malloc(sizeof(*set_copy));
    if(set_copy == NULL){
        return NULL;
    }
    SetContainer dummy_container_copy = malloc(sizeof(*dummy_container_copy));
    if(!dummy_container_copy){
        free(set_copy);
        return NULL;
    }
    dummy_container_copy->quantity=0;
    dummy_container_copy->next_container=NULL;
    set_copy->copyElement = set->copyElement;
    set_copy->compareElements = set->compareElements;
    set_copy->freeElement = set->freeElement;
    set_copy->first_AS_container = dummy_container_copy;
    set_copy->iterator = NULL;
    set_copy->size_of_Set = 0;

    SetContainer tmp = set->first_AS_container->next_container;
    SetContainer current_container_of_copy=set_copy->first_AS_container;

    while (tmp){
        current_container_of_copy->next_container=copySetContainer(set,tmp);
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
        return NULL_WAS_SENT_GETSIZE;
    }
    return set->size_of_Set;
}

bool asContains(AmountSet set, ASElement element)
{
    if(!set || !element){
        return false;
    }
    SetContainer current_container=set->first_AS_container->next_container;
    while (current_container){
        if(set->compareElements(current_container->element,element)
        ==ELEMENTS_ARE_EQUAL){
            //found the element in the set
            return true;
        }
        current_container=current_container->next_container;
    }
    //went over all of the containers and didn't find the element
    return false;
}

AmountSetResult asGetAmount(AmountSet set, ASElement element,double *outAmount){
    if(!set || !element || !outAmount) {
        return AS_NULL_ARGUMENT;
    }
    if(!asContains(set,element)){
        return AS_ITEM_DOES_NOT_EXIST;
    }
    SetContainer tmp = set->first_AS_container->next_container;
    while (tmp){
        if(set->compareElements(tmp->element,element)==ELEMENTS_ARE_EQUAL){
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
    set->iterator = NULL; //iterator is undefined after this function
    if(asContains(set,element)){
        return  AS_ITEM_ALREADY_EXISTS;
    }
    SetContainer new_container= malloc(sizeof(*new_container));
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
    set->size_of_Set=set->size_of_Set+1; //added an element to the AmountSet

    if(!(set->first_AS_container->next_container)){
        set->first_AS_container->next_container=new_container;
        return  AS_SUCCESS;
    }
    SetContainer tmp= set->first_AS_container;
    while(tmp->next_container){
        if(set->compareElements((tmp->next_container)->element,element)>0){
            new_container->next_container=tmp->next_container;
            tmp->next_container=new_container;
            return  AS_SUCCESS;
        }
        tmp=tmp->next_container;
    }
    //tmp is the last container and the element in new_container is the largest
    tmp->next_container=new_container;
    return  AS_SUCCESS;
}

AmountSetResult asChangeAmount(AmountSet set, ASElement element,
                                const double amount){
    if(!set || !element) {
        return AS_NULL_ARGUMENT;
    }
    if(!asContains(set,element)){
        return AS_ITEM_DOES_NOT_EXIST;
    }
    SetContainer tmp = set->first_AS_container->next_container;
    while (tmp){
        if(set->compareElements(tmp->element,element)==ELEMENTS_ARE_EQUAL){
            if((tmp->quantity)+amount<MIN_AMOUNT){
                return AS_INSUFFICIENT_AMOUNT;
            } else{
                //the amount is valid
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
    SetContainer tmp1= set->first_AS_container;
    SetContainer tmp2=NULL;

    while (set->compareElements(tmp1->next_container->element,element)
           !=ELEMENTS_ARE_EQUAL){
        tmp1=tmp1->next_container;
    }
    tmp2=(tmp1->next_container)->next_container;
    //now the container of the given element is between tmp1 and tmp2

    set->freeElement(tmp1->next_container->element);
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
    if( !set || !(set ->first_AS_container)
                       || !(set->first_AS_container->next_container)){
        return NULL;
    }
    set->iterator=set->first_AS_container->next_container;
    return set->iterator->element;
}

ASElement asGetNext(AmountSet set){
    if(!set ||!(set->iterator)||!(set->iterator->next_container)){
        return NULL;
    }
    set->iterator=set->iterator->next_container;
    return set->iterator->element;
}
