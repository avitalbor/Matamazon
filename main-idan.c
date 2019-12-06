#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "Set.h"

typedef struct order{
    int products_of_order;
    unsigned int id_of_order;
}*Order;

static void freeOrder(Order order){
    if(!order){
        return;
    }
    //asDestroy(order->products_of_order);
    free(order);
}

static Order copyOrder(Order order){
    Order new_Order=malloc(sizeof(*new_Order));
    if(!new_Order){
        return NULL;
    }
    new_Order->products_of_order=order->products_of_order;
    if(!new_Order->products_of_order){
        freeOrder(new_Order);
        return NULL;
    }
    new_Order->id_of_order=order->id_of_order;
    return new_Order;
}

static int compareOrders(Order order1, Order order2){
    assert(order1 &&order2);
    if((order1->id_of_order)>(order2->id_of_order)){
        return 1;
    } else if(order1->id_of_order==order2->id_of_order){
        return 0;
    }
    assert((order1->id_of_order)<(order2->id_of_order));
    return -1;
}

static int compareForSet(SetElement element1, SetElement element2){
    Order order1=(Order)element1;
    Order order2=(Order)element2;
    return compareOrders(order1,order2);
}

static void freeForSet(SetElement element){
    Order order=(Order)element;
    freeOrder(order);
}

static SetElement copyForSet(SetElement element){
    Order order=copyOrder((Order)element);
    if(!order){
        return NULL;
    }
    return (SetElement)order;
}

int main() {
    Set tmp = setCreate(copyForSet,freeForSet, compareForSet);
    Order ord = malloc(sizeof(*ord));
    ord->products_of_order=NULL;
    SetResult res = setAdd(tmp,ord);
    if (res==SET_SUCCESS) printf("ok");
    Order answer = (Order)setGetFirst(tmp);
    printf(answer->products_of_order);
    return 0;
}