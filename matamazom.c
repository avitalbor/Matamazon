#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
//#include "amount_set.h"
#include "matamazom.h"
#include "set.h"
#include "matamazom_print.h"

#define IN_RANGE_OF_MISTAKE 0.001





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
    //strcpy(newProdcut->name,product->name);
    newProdcut->name=strdup(product->name);
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
    Matamazom warehouse=malloc(sizeof( Matamazom));
    if(!warehouse){
        return NULL;
    }
    warehouse->list_of_products=asCreate(copyForAmountSet,freeForAmountSet,compareForAmountSet);
    if(!warehouse->list_of_products){
        free(warehouse);
        return NULL;
    }
    warehouse->set_of_orders=setCreate(copyForSet,freeForSet,compareForSet);
    if(!warehouse->set_of_orders){
        asDestroy(warehouse->list_of_products);
        free(warehouse);
        return NULL;
    }
    return warehouse;
}



void matamazomDestroy(Matamazom matamazom){
    if(!matamazom){
        return;
    }
    asDestroy(matamazom->list_of_products);
    setDestroy(matamazom->set_of_orders);
    free(matamazom);
}

/**
 * check if name is valid function
 * @return:
 * false- if name is empty, or doesn't start with a
 *         letter (a -z, A -Z) or a digit (0 -9).
 * true -otherwise
 */

static bool checkIfNameIsValid(const char* name){
    assert(name);
    if(strlen(name)==0){
        return false;
    }
    if(name[0]>='a'&&name[0]<='z'){
        return true;
    }
    if(name[0]>='A'&&name[0]<='Z'){
        return true;
    }
    if((name[0]-'0')>=0 &&(name[0]-'0')<=9){
        return true;
    }
    return false;
}


/**
 * receives a number ant returns it absolute value
 */
static double absOfNum(double num){
    if(num<0){
        return -num;
    }
    return num;
}


/**
 * check if the amount is valid and consistent with the amountType
 * @return:
 * false- if amount is negative or amount isn't consistent with the amountType
 * true -otherwise
 */

static bool checkIfAmountIsValid(MatamazomAmountType amountType,const double amount){
    if(amount<0){
        return false;
    }
    if(amountType==MATAMAZOM_ANY_AMOUNT){
        return true;
    }
    int completeValue=(int)amount;

    if((amount-completeValue)<=IN_RANGE_OF_MISTAKE|| ((completeValue+1))-amount<=IN_RANGE_OF_MISTAKE){
        return true;
    }
    double completeValueAndHalf=(double)completeValue+0.5;
    if(amountType=MATAMAZOM_HALF_INTEGER_AMOUNT){
        if(absOfNum((completeValueAndHalf-amount))<=IN_RANGE_OF_MISTAKE){
            return true;
        }
    }
    return false;
}

/**
 * check if the product exists in the warehouse
 * @return:
 * false- if the product doesn't belong to the warehouse
 * true -if it does
 */

static bool checkIfIdOfProductExixts(Matamazom matamazom,const unsigned int id){
    AS_FOREACH(Product,i,matamazom->list_of_products){
        if(i->id==id){
            return true;
        }
    }
    return false;
}







MatamazomResult mtmNewProduct(Matamazom matamazom, const unsigned int id, const char *name,
                              const double amount, const MatamazomAmountType amountType,
                              const MtmProductData customData, MtmCopyData copyData,
                              MtmFreeData freeData, MtmGetProductPrice prodPrice){


    if(!matamazom || !name ||!customData ||!copyData ||!freeData ||!prodPrice){
        return MATAMAZOM_NULL_ARGUMENT;
    }
    if(!checkIfNameIsValid(name)){
        return MATAMAZOM_INVALID_NAME;
    }
    if(!checkIfAmountIsValid(amountType,amount)){
        return MATAMAZOM_INVALID_AMOUNT;
    }
    Product newProduct=malloc(sizeof(Product));
    if(!newProduct){
        return MATAMAZOM_OUT_OF_MEMORY;
    }
    newProduct->id=id;

    newProduct->name==malloc(strlen(name)+1);
    if(!newProduct->name){
        freeProducts(newProduct);
        return MATAMAZOM_OUT_OF_MEMORY;
    }
    newProduct->name=strdup(name);

    newProduct->copy_function=copyData;
    newProduct->free_function=freeData;
    newProduct->get_price_function=prodPrice;
    newProduct->income=0;
    newProduct->additional_info=customData;
    newProduct->amount_type=amountType;
    AmountSetResult registerNewProduct=asRegister(matamazom->list_of_products,newProduct);
    assert(registerNewProduct);
    if (registerNewProduct==AS_ITEM_ALREADY_EXISTS){
        freeProducts(newProduct);
        return MATAMAZOM_PRODUCT_ALREADY_EXIST;
    }
    assert(registerNewProduct==AS_SUCCESS);
    asChangeAmount(matamazom->list_of_products,newProduct,amount);
    return MATAMAZOM_SUCCESS;
}


MatamazomResult mtmChangeProductAmount(Matamazom matamazom, const unsigned int id, const double amount){
    if(!matamazom){
        return MATAMAZOM_NULL_ARGUMENT;
    }
    if(!checkIfIdOfProductExixts(matamazom,id)){
        return MATAMAZOM_PRODUCT_NOT_EXIST;
    }
    Product wantedProduct=asGetFirst(matamazom->list_of_products);
    while (wantedProduct->id!=id){
        wantedProduct=asGetNext(matamazom->list_of_products);
        assert(wantedProduct);
    }
    double  *originalAmount;
    asGetAmount(matamazom->list_of_products,wantedProduct,originalAmount);
    double newAmount=*originalAmount + amount;
    if(!checkIfAmountIsValid(wantedProduct->amount_type,newAmount)){
        return MATAMAZOM_INVALID_AMOUNT;
    }
    asChangeAmount(matamazom->list_of_products,wantedProduct,amount);
    return MATAMAZOM_SUCCESS;
}



MatamazomResult mtmClearProduct(Matamazom matamazom, const unsigned int id){
    if(!matamazom){
        return MATAMAZOM_NULL_ARGUMENT;
    }
    Product tmpProduct=asGetFirst(matamazom->list_of_products);
    while (tmpProduct){
        if(tmpProduct->id==id){
            AmountSetResult delete= asDelete(matamazom->list_of_products,tmpProduct);
            assert(delete==AS_SUCCESS);
            return MATAMAZOM_SUCCESS;
        }
        tmpProduct=asGetNext(matamazom->list_of_products);
    }
    return MATAMAZOM_PRODUCT_NOT_EXIST;
}


static void printNoBestSellingProduct(FILE *output){
    fprintf(output,"Best Selling Product: /n none ");
}

MatamazomResult mtmPrintBestSelling(Matamazom matamazom, FILE *output){
    if(!matamazom|| !output){
        return MATAMAZOM_NULL_ARGUMENT;
    }
    Product bestSellingProduct=asGetFirst(matamazom->list_of_products);
    if(!bestSellingProduct){
        printNoBestSellingProduct(output);
        return MATAMAZOM_SUCCESS;
    }
    double max_income=bestSellingProduct->income;
    Product tmpProduct=asGetFirst(matamazom->list_of_products);
    AS_FOREACH(Product,currentProduct,matamazom->list_of_products)
    {
        if((currentProduct->income)>(bestSellingProduct->income)){
            bestSellingProduct=currentProduct;
            max_income=bestSellingProduct->income;
        }
    }
    if(max_income==0){
        printNoBestSellingProduct(output);
        return MATAMAZOM_SUCCESS;
    }
    mtmPrintIncomeLine(bestSellingProduct->name,bestSellingProduct->id,max_income,output);
    return MATAMAZOM_SUCCESS;
}


MatamazomResult mtmPrintFiltered(Matamazom matamazom, MtmFilterProduct customFilter, FILE *output){
    if(!matamazom || !customFilter || !output){
        return MATAMAZOM_NULL_ARGUMENT;
    }
    double amount_Of_Product=0;
    AS_FOREACH(Product,currentProduct,matamazom->list_of_products){
        asGetAmount(matamazom->list_of_products,currentProduct,&amount_Of_Product);
        if(customFilter(currentProduct->id,currentProduct->name,
                        amount_Of_Product,currentProduct->additional_info)){
            mtmPrintProductDetails(currentProduct->name,
                    currentProduct->id,amount_Of_Product,currentProduct->get_price_function(currentProduct,1),output);
        }
    }
    return MATAMAZOM_SUCCESS;
}