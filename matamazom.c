#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "amount_set.h"
#include "matamazom.h"
#include "set.h"
#include "matamazom_print.h"

#define IN_RANGE_OF_MISTAKE 0.001





typedef struct Matamazom_t {
    AmountSet  list_of_products;
    Set set_of_orders;
    unsigned  int current_order_id;
};

/**
 * struct to be used by the warehouse,
 *it functions as the element for amount set
 *  @param name - The name of the product
 *  @param id - the unique id of the product
 *  @param income- the total income the warehouse made by selling this product
 *  @param free_function -pointer for a function to be used to
 *  free additional info
 *  @param copy_function -pointer for a function to be used to
 *  copy additional info
 *  @param get_price_function -pointer for a function to be used to
 *  get the price of the prodcut
 *  @param additional info -pointer to product's additional info
 *  @param amount_type- the amount_type of the product
 */
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


/**
 * struct to describe an order,
 *@param products_of_order- an AmountSet of products to keep the
 * products of the order
 * @param id_of_order- unique id to represent the order
 */

typedef struct order{
    AmountSet products_of_order;
    unsigned int id_of_order;
}*Order;


/**
 * compareProducts: compare function of products
 *
 *  @return:
 *       0 :  if the id of the 2 products it equal
 *      -1 :  if the id of the first product is bigger
 *       1 :  if the id of the second product is bigger
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
 * compareForAmountSet: pointer of function to be sent for amoun set
 *                       does casting of element to product and calls the
 *                       compare products function
 * @return:
 *       0 :  if the 2 elemnts equal
 *      -1 :  if the  first element is bigger
 *       1 :  if the second elemnt is bigger
 */
static int compareForAmountSet(ASElement element1, ASElement element2)
{
    Product product1= (Product)element1;
    Product product2= (Product)element2;
    return compareProducts(product1,product2);
}


/**
 * freeProducts- frees the data the product has (name and info) and than
 * frees the product
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
    Product new_product=malloc(sizeof(*new_product));
    if(!new_product){
        return NULL;
    }
    new_product->name=strdup(product->name);
    new_product->id=product->id;
    new_product->amount_type=product->amount_type;
    new_product->income=product->income;
    new_product->copy_function=product->copy_function;
    new_product->free_function=product->free_function;
    new_product->get_price_function=product->get_price_function;
    new_product->additional_info=
                product->copy_function(product->additional_info);
    return new_product;
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
 * this function does casting of SetElement to Order and calls
 * the compareOrders function
 * this function should return:
 * 1 if the first element is bigger,
 * 0 if the two elements are equal and -1 if the second element is bigger
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
    if(!order){
        return;
    }
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
    Order new_Order=malloc(sizeof(*new_Order));
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
    Matamazom warehouse=malloc(sizeof(*warehouse));
    if(!warehouse){
        return NULL;
    }
    //now need to create list_of_products and the set_of_orders
    warehouse->list_of_products=
            asCreate(copyForAmountSet,freeForAmountSet,compareForAmountSet);
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
    warehouse->current_order_id=0;
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
 * check if name is valid : receives a name and returns if its valid
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
 * receives a number and returns it absolute value
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

    if(amountType==MATAMAZOM_ANY_AMOUNT){
        return true;
    }
    int completeValue;
    if(amount>=0){
        completeValue=(int)amount;
    } else{
        completeValue=(int)amount -1;
    }

    if(absOfNum((amount-completeValue))<=IN_RANGE_OF_MISTAKE||
            absOfNum((completeValue+1)-amount)<=IN_RANGE_OF_MISTAKE){
        return true;
    }
    double complete_value_and_half=(double)completeValue+0.5;
    if(amountType==MATAMAZOM_HALF_INTEGER_AMOUNT){
        if(absOfNum((complete_value_and_half-amount))<=IN_RANGE_OF_MISTAKE){
            return true;
        }
    }
    return false;
}

/**checkIfIdOfProductExists: receives a matamazom and id, and returns if
 * there exicts a product with the same id in the matamazom
 * @return:
 * false- if there isn't a product with the given id in the warehouse
 * true -if there is such a product
 */

static bool checkIfIdOfProductExists(Matamazom matamazom,const unsigned int id){
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
    if(!checkIfAmountIsValid(amountType,amount)||amount<0){
        return MATAMAZOM_INVALID_AMOUNT;
    }
    Product new_product=malloc(sizeof(*new_product));
    if(!new_product){
        return MATAMAZOM_OUT_OF_MEMORY;
    }
    new_product->id=id;

    new_product->name=malloc(strlen(name)+1);
    if(!new_product->name){
        freeProducts(new_product);
        return MATAMAZOM_OUT_OF_MEMORY;
    }
    new_product->name=strdup(name);
    if(!new_product->name){
        freeProducts(new_product);
        return MATAMAZOM_OUT_OF_MEMORY;
    }
    new_product->copy_function=copyData;
    new_product->free_function=freeData;
    new_product->get_price_function=prodPrice;
    new_product->income=0;
    new_product->additional_info=copyData(customData);
    if(!new_product->additional_info){
        freeProducts(new_product);
        return MATAMAZOM_OUT_OF_MEMORY;
    }
    new_product->amount_type=amountType;
    AmountSetResult registerNewProduct=asRegister
                                      (matamazom->list_of_products,new_product);
    if (registerNewProduct==AS_ITEM_ALREADY_EXISTS){
        freeProducts(new_product);
        return MATAMAZOM_PRODUCT_ALREADY_EXIST;
    }
    if(registerNewProduct==AS_OUT_OF_MEMORY){
        freeProducts(new_product);
        return MATAMAZOM_OUT_OF_MEMORY;
    }
    assert(registerNewProduct==AS_SUCCESS);
    asChangeAmount(matamazom->list_of_products,new_product,amount);
    return MATAMAZOM_SUCCESS;
}


MatamazomResult mtmChangeProductAmount(Matamazom matamazom, const unsigned int id, const double amount){
    if(!matamazom){
        return MATAMAZOM_NULL_ARGUMENT;
    }
    if(!checkIfIdOfProductExists(matamazom,id)){
        return MATAMAZOM_PRODUCT_NOT_EXIST;
    }
    Product wantedProduct=asGetFirst(matamazom->list_of_products);
    while (wantedProduct->id!=id){
        wantedProduct=asGetNext(matamazom->list_of_products);
        assert(wantedProduct);
    }
    if(!checkIfAmountIsValid(wantedProduct->amount_type,amount)){
        return MATAMAZOM_INVALID_AMOUNT;
    }
    double  originalAmount;
    asGetAmount(matamazom->list_of_products,wantedProduct,&originalAmount);
    double newAmount=originalAmount + amount;
    if(!checkIfAmountIsValid(wantedProduct->amount_type,newAmount)){
        return MATAMAZOM_INVALID_AMOUNT;
    }
    if(newAmount<0){
        return MATAMAZOM_INSUFFICIENT_AMOUNT;
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
    fprintf(output,"Best Selling Product:\nnone\n");
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
    AS_FOREACH(Product,currentProduct,matamazom->list_of_products){
        if((currentProduct->income)-(bestSellingProduct->income)>IN_RANGE_OF_MISTAKE){
            bestSellingProduct=currentProduct;
            max_income=bestSellingProduct->income;
        }
    }
    if(max_income==0){
        printNoBestSellingProduct(output);
        return MATAMAZOM_SUCCESS;
    }
    fprintf(output,"Best Selling Product:\n");
    mtmPrintIncomeLine(bestSellingProduct->name,bestSellingProduct->id,max_income,output);
    return MATAMAZOM_SUCCESS;
}

MatamazomResult mtmPrintFiltered(Matamazom matamazom, MtmFilterProduct customFilter, FILE *output){
    if(!matamazom || !customFilter || !output){
        return MATAMAZOM_NULL_ARGUMENT;
    }
    double amount_Of_Product=0;
    AS_FOREACH(Product,currentProduct,matamazom->list_of_products){
        AmountSetResult result = asGetAmount(matamazom->list_of_products,currentProduct,&amount_Of_Product);
        if(customFilter(currentProduct->id,currentProduct->name,
                        amount_Of_Product,currentProduct->additional_info)){
            mtmPrintProductDetails(currentProduct->name,currentProduct->id,
                    amount_Of_Product,currentProduct->get_price_function(currentProduct->additional_info,1),output);
        }
    }
    return MATAMAZOM_SUCCESS;
}

static double getTotalPriceOforder(Order order){
    double total_price_of_order=0;
    double price_of_product=0;
    double amount_of_product_in_order;
    AS_FOREACH(Product,current_product,order->products_of_order){
        AmountSetResult result =asGetAmount(order->products_of_order,current_product,&amount_of_product_in_order);
        assert(result == AS_SUCCESS);
        price_of_product=current_product->get_price_function(current_product->additional_info,amount_of_product_in_order);
        total_price_of_order=total_price_of_order+price_of_product;
    }
    return total_price_of_order;
}




static void printProductsOfAmountSet(AmountSet set, const bool per_unit, FILE *output){
    AS_FOREACH(Product,current_product,set) {
        double amount_of_current_product;
        double price_of_product;
        AmountSetResult result = asGetAmount(set, current_product, &amount_of_current_product);
        assert(result == AS_SUCCESS);
        if(per_unit == true) {
            price_of_product = current_product->get_price_function(
                    current_product->additional_info, 1);
        }
        else{
            price_of_product = current_product->get_price_function(
                    current_product->additional_info, amount_of_current_product);
        }
        mtmPrintProductDetails(current_product->name, current_product->id, amount_of_current_product, price_of_product,
                               output);//check if output her is correct
    }
}

MatamazomResult mtmPrintInventory(Matamazom matamazom, FILE *output){
    if(!matamazom || !output){
        return MATAMAZOM_NULL_ARGUMENT;
    }
    fprintf(output,"Inventory Status:\n");
    printProductsOfAmountSet(matamazom->list_of_products, true, output);
    return MATAMAZOM_SUCCESS;
}

MatamazomResult mtmPrintOrder(Matamazom matamazom, const unsigned int orderId, FILE *output) {
    if (!matamazom || !output) {
        return MATAMAZOM_NULL_ARGUMENT;
    }
    SET_FOREACH(Order, current_order, matamazom->set_of_orders) {
        if (current_order->id_of_order == orderId) {
            mtmPrintOrderHeading(orderId, output);
            printProductsOfAmountSet(current_order->products_of_order,
                                                        false,output);
            double total_price_of_order = getTotalPriceOforder(current_order);
            mtmPrintOrderSummary(total_price_of_order, output);
            return AS_SUCCESS;
        }
    }
    return MATAMAZOM_ORDER_NOT_EXIST;
}




static Order getOrderFromId(Set set, unsigned int orderId){
    assert(set);
    Order wanted_order = NULL;

    SET_FOREACH(Order,currentOrder,set){
        if(currentOrder->id_of_order == orderId){
            wanted_order = currentOrder;
            return wanted_order;
        }
    }
    if(wanted_order == NULL){
        return NULL;
    }
    return wanted_order;
}

/** NEEDS CLARIFYING */
static Product getProductFromId(AmountSet set, unsigned int productId){
    assert(set);
    Product wanted_product = NULL;
    AS_FOREACH(Product,currentProduct,set){
        if(currentProduct->id == productId){
            wanted_product = currentProduct;
            assert(wanted_product);
            break;
        }
    }
    if(wanted_product == NULL){
        return NULL;
    }
    return wanted_product;
}



unsigned int mtmCreateNewOrder(Matamazom matamazom){
    if(!matamazom){
        return 0;
    }
    Order new_order = malloc(sizeof(*new_order));
    if(!new_order){
        return 0;
    }

    new_order->products_of_order = asCreate(copyForAmountSet,freeForAmountSet,compareForAmountSet);
    if(!new_order->products_of_order){
        freeOrder(new_order);
        return 0;
    }

    matamazom->current_order_id=matamazom->current_order_id+1;
    new_order->id_of_order=matamazom->current_order_id;
    // put the order in the specific matamazom
    SetResult register_new_order = setAdd(matamazom->set_of_orders, (SetElement)new_order);
   // SetResult register_new_order = setAdd(matamazom->set_of_orders, new_order;



    if(register_new_order != SET_SUCCESS){
        free(new_order);
        return 0;
    }


    return new_order->id_of_order;
}




MatamazomResult mtmChangeProductAmountInOrder(Matamazom matamazom, const unsigned int orderId,
                                              const unsigned int productId, const double amount){
    if(!matamazom){
        return MATAMAZOM_NULL_ARGUMENT;
    }
    //get the wanted order
    Order wanted_order = getOrderFromId(matamazom->set_of_orders, orderId);
    if(wanted_order == NULL){
        return MATAMAZOM_ORDER_NOT_EXIST;
    }
    //check if product in warehouse
    Product product_in_warehouse = getProductFromId(matamazom->list_of_products, productId);
    if(product_in_warehouse == NULL){
        return MATAMAZOM_PRODUCT_NOT_EXIST;
    }
    if(!checkIfAmountIsValid(product_in_warehouse->amount_type, amount)){
        return MATAMAZOM_INVALID_AMOUNT;
    }
    //check if product is in order
    Product product_in_order = getProductFromId(wanted_order->products_of_order, productId);
    if(product_in_order == NULL){
        if(amount > 0){
            AmountSetResult register_result = asRegister(wanted_order->products_of_order, (ASElement)product_in_warehouse);
            assert(register_result ==AS_SUCCESS);
            AmountSetResult change_result = asChangeAmount(wanted_order->products_of_order,
                    (ASElement)product_in_warehouse, amount);
            assert(change_result != AS_NULL_ARGUMENT);
        }
    }
    else{
        AmountSetResult change_result = asChangeAmount(wanted_order->products_of_order, (ASElement)product_in_order, amount);
        assert(change_result != AS_NULL_ARGUMENT);
        if(change_result == AS_INSUFFICIENT_AMOUNT){
            AmountSetResult delete_result = asDelete(wanted_order->products_of_order, (ASElement)product_in_order);
            assert(delete_result != AS_NULL_ARGUMENT);
        }
        double amount_of_product_in_order;
        AmountSetResult getAmount_result = asGetAmount(wanted_order->products_of_order, (ASElement)product_in_order, &amount_of_product_in_order);
        assert(getAmount_result != AS_NULL_ARGUMENT);
        if(amount_of_product_in_order == 0){
            AmountSetResult delete_result = asDelete(wanted_order->products_of_order, (ASElement)product_in_order);
            assert(delete_result != AS_NULL_ARGUMENT);
        }
    }
    return MATAMAZOM_SUCCESS;
}



/**
 * checkIfOrderIsValid: checks if the amount of all the products in the order are valid
 * @return:
 * false-  if the amount of one product in the order is bigger than the amount in matamazom
 * true -if the amount of all the products is valid
 */


static bool checkIfOrderIsValid(Matamazom matamazom, Order order) {
    assert(matamazom && order);
    double amount_in_matamazom;
    double amount_in_order;
    AmountSetResult get_amount;
    AS_FOREACH(Product, orderProduct, order->products_of_order) {
        get_amount=asGetAmount(matamazom->list_of_products,orderProduct,&amount_in_matamazom);

        assert(get_amount==AS_SUCCESS);
        get_amount=asGetAmount(order->products_of_order,orderProduct,&amount_in_order);
        assert(get_amount==AS_SUCCESS);
        if(amount_in_order>amount_in_matamazom){
            return false;
        }
    }
    return true;
}

MatamazomResult mtmShipOrder(Matamazom matamazom, const unsigned int orderId){
    if(!matamazom){
        return MATAMAZOM_NULL_ARGUMENT;
    }
    //get the order
    Order wanted_order =getOrderFromId(matamazom->set_of_orders,orderId);
    if(wanted_order == NULL){
        return MATAMAZOM_ORDER_NOT_EXIST;
    }
    // check if the amounts are ok and if not - dont do anything and return insufficient
    if(!checkIfOrderIsValid(matamazom,wanted_order)){
        return MATAMAZOM_INSUFFICIENT_AMOUNT;
    }
    // now the order is ok - substract all amounts from the warehouse
    double amount_of_product_in_order;
    Product warehouse_product;

    AmountSetResult result;

    AS_FOREACH(Product,orderProduct,wanted_order->products_of_order){
        result=asGetAmount(wanted_order->products_of_order,orderProduct,&amount_of_product_in_order);
        assert(result==AS_SUCCESS);
        result=asChangeAmount(matamazom->list_of_products,orderProduct,-amount_of_product_in_order);
        assert(result==AS_SUCCESS);

        warehouse_product=getProductFromId(matamazom->list_of_products,orderProduct->id);
        assert(warehouse_product);
        warehouse_product->income=(warehouse_product->income)
                +warehouse_product->get_price_function(warehouse_product->additional_info,amount_of_product_in_order);
    }
    // delete order after changing amounts
    mtmCancelOrder(matamazom, orderId);
    return MATAMAZOM_SUCCESS;
}


MatamazomResult mtmCancelOrder(Matamazom matamazom, const unsigned int orderId){
    if(!matamazom){
        return MATAMAZOM_NULL_ARGUMENT;
    }
    //get the order
    Order wanted_order=getOrderFromId(matamazom->set_of_orders,orderId);

    if(wanted_order == NULL){
        return MATAMAZOM_ORDER_NOT_EXIST;
    }
    //asDestroy(wanted_order->products_of_order); the removes makes care of the destroy
    assert(wanted_order);
    assert(matamazom->set_of_orders);
    SetResult remove_result = setRemove(matamazom->set_of_orders, (SetElement)wanted_order);
    assert(remove_result != SET_NULL_ARGUMENT);
    return MATAMAZOM_SUCCESS;
}









