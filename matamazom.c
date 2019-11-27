#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../amount_Set/amount_set.h"
#include "../matamazom.h"
#include "../set.h"
#include "../list.h"








typedef struct Matamazom_t {
    AmountSet  list_of_products;
    Set set_of_orders;
};

typedef struct product{
    char* name;
    unsigned int id;
    double income;
    MtmFreeData free_function;
    MtmCopyData copy_function;
    MtmProductData additional_info;
    MatamazomAmountType amount_type;
    MtmGetProductPrice  get_price_function;
}*Product;

typedef struct order{
    AmountSet products_of_order;
    unsigned int id_of_order;
}*Order;


/**
 * compare function of products
 * this function should return:
 * 1 if the id of the first product is bigger, 0 if the id of two Products is  equal
 * and -1 if the id of the second product is bigger
 */
static int compareProducts(Product product1, Product product2){
    assert(product1 &&product2);
    if((product1->id)>(product2->id)){
        return 1;
    } else if(product1->id==product2->id){
        return 0;
    }
    assert((product1->id)<(product2->id));
    return -1;
}

/**
 * compare function of ASElement that will be sent to amount_Set
 * this function does casting of ASElement to Product and calls the compareProducts function
 * this function should return:
 * 1 if the first element is bigger, 0 if the two elements are equal and -1 if the second element is bigger
 */
static int compareForAmountSet(ASElement element1, ASElement element2)
{
    Product product1= (Product)element1;
    Product product2= (Product)element2;
    return compareProducts(product1,product2);
}


/**
 * free function of products
 */
static void freeProducts(Product product){
    assert(product!=NULL);
    free(product->name);
    product->free_function (product->additional_info);
    free(product);
}

/**
 * free function that will be sent to amount_Set
 * does casting of ASElement to product and calls the freeProducts function
 */
static void freeForAmountSet(ASElement element)
{
    Product product=(Product) element;
    freeProducts(product);
}


/**
 * copy function  of a Product
 * @return:
 * NULL- if allocation failed
 * The duplicated Product if the function was successful
 */
static Product copyProduct(Product product){
    Product newProdcut=malloc(sizeof(Product));
    if(!newProdcut){
        return NULL;
    }
    strcpy(newProdcut->name,product->name);
    newProdcut->id=product->id;
    newProdcut->amount_type=product->amount_type;
    newProdcut->income=product->income;
    newProdcut->copy_function=product->copy_function;
    newProdcut->free_function=product->free_function;
    newProdcut->additional_info=product->copy_function(product->additional_info);
    return newProdcut;
}


/**
 * copy function that will be sent to amount_Set
 * does casting of ASElement to product and calls the copyProducts function
 * @return:
 * NULL- if allocation failed
 * The duplicated ASElement if the function was successful
 */
static ASElement copyForAmountSet(ASElement element){
    Product product=copyProduct((Product)element);
    if(!product){
        return NULL;
    }
    return (ASElement) product;
}


/**
 * compare function of Order
 * this function should return:
 * 1 if the id of the first order is bigger, 0 if the id of two Orders is  equal
 * and -1 if the id of the second order is bigger
 */
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

/**
 * compare function of SetElement that will be sent to set
 * this function does casting of SetElement to Order and calls the compareOrders function
 * this function should return:
 * 1 if the first element is bigger, 0 if the two elements are equal and -1 if the second element is bigger
 */
static int compareForSet(SetElement element1, SetElement element2){
    Order order1=(Order)element1;
    Order order2=(Order)element2;
    return compareOrders(order1,order2);
}



/**
 * free function of Order
 * first destroys the products_of_order and frees the order.
 */

static void freeOrder(Order order){
    asDestroy(order->products_of_order);
    free(order);
}




/**
 * free function that will be sent to set
 * does casting of SetElement to Order and calls the freeOrder function
 */

static void freeForSet(SetElement element){
    Order order=(Order)element;
    freeOrder(order);
}


/**
 * copy function  of a Order
 * @return:
 * NULL- if allocation failed
 * The duplicated Order if the function was successful
 */


static Order copyOrder(Order order){
    Order new_Order=malloc(sizeof(Order));
    if(!new_Order){
        return NULL;
    }
    new_Order->products_of_order=asCopy(order->products_of_order);
    if(!new_Order->products_of_order){
        freeOrder(new_Order);
        return NULL;
    }
    new_Order->id_of_order=order->id_of_order;
    return new_Order;
}


/**
 * copy function that will be sent to set
 * does casting of SetElement to Order and calls the copyOrder function
 * @return:
 * NULL- if allocation failed
 * The duplicated SetElement if the function was successful
 */

static SetElement copyForSet(SetElement element){
    Order order=copyOrder((Order)element);
    if(!order){
        return NULL;
    }
    return (SetElement)order;
}





Matamazom matamazomCreate(){
    Matamazom matamazom=malloc(sizeof( Matamazom));
    if(!matamazom){
        return NULL;
    }
    matamazom->list_of_products=asCreate(copyForAmountSet,freeForAmountSet,compareForAmountSet);
    if(!matamazom->list_of_products){
        free(matamazom);
        return NULL;
    }
    matamazom->set_of_orders=setCreate(copyForSet,freeForSet,compareForSet);
    if(!matamazom->set_of_orders){
        asDestroy(matamazom->list_of_products);
        free(matamazom);
        return NULL;
    }
    return matamazom;
}






