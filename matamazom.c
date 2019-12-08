
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "amount_set.h"
#include "matamazom.h"
#include "set.h"
#include "matamazom_print.h"

#define IN_RANGE_OF_MISTAKE 0.001
#define COMPARE_EQUAL 0
#define COMPARE_LARGER (-1)
#define COMPARE_SMALLER 1
#define INTEGER 1
#define HALF_INTEGER (0.5)
#define UNIT 1

struct Matamazom_t {
    AmountSet  list_of_products;
    Set set_of_orders;
    unsigned  int current_order_id;
};

/**
 * Product
 *
 * This is an internal struct implemented to be used by the Matamazom warehouse.
 * It functions as the ASElement for an AmountSet.
 *
 *  @param name - The name of the product.
 *  @param id - A unique identifier to represent the product.
 *  @param income- The total income that the warehouse made by selling this
 *  product.
 *  @param free_function - A pointer to a function to be used to
 *  free additional info.
 *  @param copy_function - A pointer to a function to be used to
 *  copy additional info.
 *  @param get_price_function - A pointer to a function to be used to
 *  get the price of the prodcut.
 *  @param additional info - A pointer to product's additional info.
 *  @param amount_type - The type of amount that the product may recieve from
 *  the user - INTEGER, HALF_INTEGER or ALL.
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
 * Order
 *
 * This is an internal struct implemented to be used by the Matamazom warehouse.
 * It functions as the SetElement for a Set, and as an order in a warehouse.
 *
 * @param list_of_order_products - An AmountSet of products to keep the
 * products of the order.
 * @param id - A unique identifier to represent the order.
 */
typedef struct order{
    AmountSet list_of_order_products;
    unsigned int id;
}*Order;

/**
 * compareProducts: a compare function for 2 products.
 *
 * @param product1 - the first product to be compared.
 * @param product2 - the second product to be compared.
 *
 * @return:
 *      COMPARE_EQUAL - if the id of the 2 products is equal.
 *      COMPARE_LARGER - if the id of the first product is larger.
 *      COMPARE_SMALLER - if the id of the second product is larger.
 */
static int compareProducts(Product product1, Product product2){
    if((product1->id)>(product2->id)){
        return COMPARE_SMALLER;
    } else if(product1->id==product2->id){
        return COMPARE_EQUAL;
    }
    return COMPARE_LARGER;
}

/**
 * compareForAmountSet: a function to be sent to an AmountSet.
 *                       it is responsible for the casting from element to
 *                       product then calling the compare products function.
 *
 * @param element1 - the first element to be compared.
 * @param element2 - the second element to be compared.
 *
 * @return:
 *      COMPARE_EQUAL - if the id of the 2 products is equal.
 *      COMPARE_LARGER - if the id of the first product is larger.
 *      COMPARE_SMALLER - if the id of the second product is larger.
 */
static int compareProductsForAmountSet(ASElement element1, ASElement element2)
{
    Product product1= (Product)element1;
    Product product2= (Product)element2;
    return compareProducts(product1,product2);
}

/**
 * freeProducts: frees the data the product has (name and addintional_info)
 * then frees the memory that was allocated for the product.
 *
 * @param product - The product its data needs to be freed.
 */
static void freeProduct(Product product){
    free(product->name);
    product->free_function(product->additional_info);
    free(product);
}

/**
 * freeProductForAmountSet: a free function that will be sent to an AmountSet.
 *                          it is responsible for the casting from element to
 *                          product then calling the free product function.
 *
 * @param element - the element its data needs to be freed.
 */
static void freeProductForAmountSet(ASElement element)
{
    Product product=(Product) element;
    freeProduct(product);
}

/**
 * copyProduct: copies product and its content.
 *
 * @param element - the product to be copied.
 *
 * @return
 *     NULL - if a memory allocation failed.
 *     A copy of the received product if the process was successfull.
 */
static Product copyProduct(Product product){
    Product new_product=malloc(sizeof(*new_product));
    if(!new_product){
        return NULL;
    }
    new_product->name=malloc(strlen(product->name)+1);
    if(!new_product->name){
        freeProduct(new_product);
        return NULL;
    }
    strcpy(new_product->name,product->name);
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
 * copyProductForAmountSet: a copy function that will be sent to an AmountSet.
 *                          it is responsible for the casting from element to
 *                          product then calling the copy product function.
 *
 * @param element - the element to be copied.
 *
 * @return
 *     NULL - if a memory allocation failed.
 *     A copy of the received element if the process was successfull.
 */
static ASElement copyProductForAmountSet(ASElement element){
    Product product=copyProduct((Product)element);
    if(!product){
        return NULL;
    }
    return (ASElement) product;
}

/**
 * compareOrders: a compare function for 2 orders.
 *
 * @param order1 - the first order to be compared.
 * @param order2 - the second order to be compared.
 *
 * @return:
 *      COMPARE_EQUAL - if the id of the 2 orders is equal.
 *      COMPARE_LARGER - if the id of the first order is larger.
 *      COMPARE_SMALLER - if the id of the second order is larger.
 */
static int compareOrders(Order order1, Order order2){
    if((order1->id)>(order2->id)){
        return COMPARE_SMALLER;
    } else if(order1->id==order2->id){
        return COMPARE_EQUAL;
    }
    return COMPARE_LARGER;
}

/**
 * compareOrdersForSet: a function to be sent to a Set.
 *                       it is responsible for the casting from element to
 *                       order then calling the compare orders function.
 *
 * @param element1 - the first element to be compared.
 * @param element2 - the second element to be compared.
 *
 * @return:
 *      COMPARE_EQUAL - if the id of the 2 products is equal.
 *      COMPARE_LARGER - if the id of the first product is larger.
 *      COMPARE_SMALLER - if the id of the second product is larger.
 */
static int compareOrdersForSet(SetElement element1, SetElement element2){
    Order order1=(Order)element1;
    Order order2=(Order)element2;
    return compareOrders(order1,order2);
}

/**
 * freeOrder: frees the data the order has (its products and id)
 * then frees the memory that was allocated for the order.
 *
 * @param order - The order its data needs to be freed.
 */
static void freeOrder(Order order){
    if(!order){
        return;
    }
    asDestroy(order->list_of_order_products);
    free(order);
}



/**
 * freeOrderForSet: a free function that will be sent to a Set.
 *                  it is responsible for the casting from element to
 *                  order then calling the free order function.
 *
 * @param element - the element its data needs to be freed.
 */
static void freeOrderForSet(SetElement element){
    Order order=(Order)element;
    freeOrder(order);
}










/**
 * copyOrder: copies an order and its content.
 *
 * @param order - the order to be copied.
 *
 * @return
 *     NULL - if a memory allocation failed.
 *     A copy of the received order if the process was successfull.
 */
static Order copyOrder(Order order){
    Order new_Order=malloc(sizeof(*new_Order));
    if(!new_Order){
        return NULL;
    }
    new_Order->list_of_order_products=asCopy(order->list_of_order_products);
    if(!new_Order->list_of_order_products){
        freeOrder(new_Order);
        return NULL;
    }
    new_Order->id=order->id;
    return new_Order;
}

/**
 * copyOrderForSet: a copy function that will be sent to a Set.
 *                  it is responsible for the casting from element to
 *                  order then calling the copy order function.
 *
 * @param element - the element to be copied.
 *
 * @return
 *     NULL - if a memory allocation failed.
 *     A copy of the received element if the process was successfull.
 */
static SetElement copyOrderForSet(SetElement element){
    Order order=copyOrder((Order)element);
    if(!order){
        return NULL;
    }
    return (SetElement)order;
}

/**
 * getProductFromId: receives an AmountSet and an id, and returns a pointer
 *                   to the product with the same id in the received AmountSet.
 *
 * @param set - The AmountSet in which the product will be looked for.
 * @param productId - The id of the desired product.
 *
 * @return:
 *      NULL - if there is no product with the given id in the AmountSet.
 *      A pointer to the desired product (the product with the same id
 *      as received).
 */
 static Product getProductFromId(AmountSet set, unsigned int productId){
    Product wanted_product = NULL;
    AS_FOREACH(Product,currentProduct,set){
        if(currentProduct->id == productId){
            wanted_product = currentProduct;
            break;
        }
    }
    if(wanted_product == NULL){
        return NULL;
    }
    return wanted_product;
}

/**
 * checkIfNameIsValid: receives a name and determines whether it is valid.
 *
 * @param name - The name to be checked..
 *
 * @return:
 *      false - if the name is not valid, which means:
 *          the name is emtpy, or
 *          the name doesn't start with a letter (a-z, A-Z) or a digit (0-9).
 *      otherwise - true.
 */
static bool checkIfNameIsValid(const char* name){
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
 * absOfNumber
 * * @return:
 * the absolute value of the number the function receives
 */

/**
 * absOfNumber: calculates the absolute value of a number.
 *
 * @param number - the number that we wish to get its absolute value.
 *
 * @return
 *      The absolute value of the number.
 */
static double absOfNumber(double number){
    if(number<0){
        return -number;
    }
    return number;
}

/**
 * checkIfAmountIsValid: receives an amount and determines whether it is valid.
 *
 * @param amountType - The type of amount that can be inserted.
 * @param amount - The amount to be checked.
 *
 * @return:
 *      false - if the amount is not valid, which means it is inconsistent with
 *          the amount type.
 *      otherwise - true.
 */
static bool checkIfAmountIsValid(MatamazomAmountType amountType,
                                                        const double amount){
    if(amountType==MATAMAZOM_ANY_AMOUNT){
        return true;
    }
    int completeValue;
    if(amount>=0){
        completeValue=(int)amount;
    } else{
        completeValue=(int)amount - INTEGER;
    }

    if(absOfNumber((amount-completeValue))<=IN_RANGE_OF_MISTAKE||
            absOfNumber((completeValue + INTEGER)-amount)<=IN_RANGE_OF_MISTAKE){
        return true;
    }
    double complete_value_and_half=(double)completeValue + HALF_INTEGER;
    if(amountType==MATAMAZOM_HALF_INTEGER_AMOUNT){
        if(absOfNumber((complete_value_and_half-amount))<=IN_RANGE_OF_MISTAKE){
            return true;
        }
    }
    return false;
}

/**
 * printNoBestSellingProduct: prints to an output file the line that needs to be
 *                            printed in case there's no best selling product.
 *
 * @param output - A pointer to the output file the the printing will happen in.
 */
static void printNoBestSellingProduct(FILE *output){
    fprintf(output,"Best Selling Product:\nnone\n");
}

/**
 * getTotalPriceOfOrder: receives an order and returns how much is needed to be
 *                       paid for it.
 *
 * @param order - The order that its price is requested.
 *
 * @return:
 *      A number that represents the price needed to be paid for the order.
 */
static double getTotalPriceOfOrder(Order order){
    double total_price_of_order=0;
    double price_of_product=0;
    double amount_of_product_in_order;
    AS_FOREACH(Product,current_product,order->list_of_order_products){
        asGetAmount(order->list_of_order_products,current_product,
                &amount_of_product_in_order);
        price_of_product=current_product->get_price_function
                (current_product->additional_info,amount_of_product_in_order);
        total_price_of_order=total_price_of_order+price_of_product;
    }
    return total_price_of_order;
}

/**
 * printProductsOfAmountSet: receives an AmountSet, then prints the details of
 *                           the products in it, such as their names, amounts,
 *                           prices etc.

 * @param set - The AmountSet which we want to print its products' price.
 * @param per_unit - a boolean variable to determine wheter we would like to
 *          print the total price of all of the products, or just the price per
 *          a single unit of the product.
 * @param output - A pointer to the output file the the printing will happen in.
 */
static void printProductsOfAmountSet(AmountSet set,
                                     const bool per_unit, FILE *output){
    AS_FOREACH(Product,current_product,set) {
        double amount_of_current_product;
        double price_of_product;
        asGetAmount(set, current_product, &amount_of_current_product);
        if(per_unit == true) {
            price_of_product = current_product->get_price_function(
                    current_product->additional_info, UNIT);
        } else{
            price_of_product = current_product->
                    get_price_function(current_product->additional_info,
                            amount_of_current_product);
        }
        mtmPrintProductDetails(current_product->name, current_product->id,
                amount_of_current_product, price_of_product, output);
    }
}

/**
 * getOrderFromId: receives a Set and an id, and returns a pointer
 *                   to the order with the same id in the received Ser.
 *
 * @param set - The AmountSet in which the product will be looked for.
 * @param productId - The id of the desired product.
 *
 * @return:
 *      NULL - if there is no order with the given id in the set.
 *      A pointer to the desired order (the product with the same id
 *      as received).
 */
static Order getOrderFromId(Set set, unsigned int orderId){
    Order wanted_order = NULL;

    SET_FOREACH(Order,currentOrder,set){
        if(currentOrder->id == orderId){
            wanted_order = currentOrder;
        }
    }
    if(wanted_order == NULL){
        return NULL;
    }
    return wanted_order;
}

/**
 * checkIfOrderIsValid: receives an order and determines whether it is valid.
 *
 * @param matamazom - The matamazom warehouse that the order is in.
 * @param order - The order to be checked.
 *
 * @return:
 *      false - if there is a product in the order with a larger amount than its
 *          amount in the matamazom warehouse.
 *      true - if the amount of all of the products in the order is valid.
 */
static bool checkIfOrderIsValid(Matamazom matamazom, Order order) {
    double amount_in_matamazom;
    double amount_in_order;
    AS_FOREACH(Product, orderProduct, order->list_of_order_products) {
        asGetAmount(matamazom->list_of_products,orderProduct,
                &amount_in_matamazom);
        asGetAmount(order->list_of_order_products,orderProduct,
                &amount_in_order);
        if(amount_in_order>amount_in_matamazom){
            return false;
        }
    }
    return true;
}

Matamazom matamazomCreate(){
    Matamazom warehouse=malloc(sizeof(*warehouse));
    if(!warehouse){
        return NULL;
    }
    warehouse->list_of_products=
            asCreate(copyProductForAmountSet,freeProductForAmountSet,
                    compareProductsForAmountSet);
    if(!warehouse->list_of_products){
        free(warehouse);
        return NULL;
    }
    warehouse->set_of_orders=setCreate(copyOrderForSet,freeOrderForSet,
            compareOrdersForSet);
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
    setDestroy(matamazom->set_of_orders);
    asDestroy(matamazom->list_of_products);
    free(matamazom);
}

MatamazomResult mtmNewProduct(Matamazom matamazom, const unsigned int id,
                                const char *name, const double amount,
                                const MatamazomAmountType amountType,
                                const MtmProductData customData,
                                MtmCopyData copyData, MtmFreeData freeData,
                                MtmGetProductPrice prodPrice){
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
        freeProduct(new_product);
        return MATAMAZOM_OUT_OF_MEMORY;
    }
    strcpy(new_product->name,name);
    new_product->copy_function=copyData;
    new_product->free_function=freeData;
    new_product->get_price_function=prodPrice;
    new_product->income=0;
    new_product->additional_info=copyData(customData);
    if(!new_product->additional_info){
        freeProduct(new_product);
        return MATAMAZOM_OUT_OF_MEMORY;
    }
    new_product->amount_type=amountType;
    AmountSetResult registerNewProduct=asRegister
                                      (matamazom->list_of_products,new_product);
    if (registerNewProduct==AS_ITEM_ALREADY_EXISTS){
        freeProduct(new_product);
        return MATAMAZOM_PRODUCT_ALREADY_EXIST;
    } else if(registerNewProduct==AS_OUT_OF_MEMORY){
        freeProduct(new_product);
        return MATAMAZOM_OUT_OF_MEMORY;
    }
    asChangeAmount(matamazom->list_of_products,new_product,amount);
    freeProduct(new_product); //because asRegister makes a newCopy
    return MATAMAZOM_SUCCESS;
}

MatamazomResult mtmChangeProductAmount(Matamazom matamazom,
                                        const unsigned int id,
                                        const double amount){
    if(!matamazom){
        return MATAMAZOM_NULL_ARGUMENT;
    }

    Product wantedProduct=getProductFromId(matamazom->list_of_products,id);
    if(wantedProduct==NULL){
        return MATAMAZOM_PRODUCT_NOT_EXIST;
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
    Product wantedProduct =getProductFromId(matamazom->list_of_products,id);
    if(wantedProduct==NULL){
        return MATAMAZOM_PRODUCT_NOT_EXIST;
    }
    SET_FOREACH(Order,current_order,matamazom->set_of_orders){
        asDelete(current_order->list_of_order_products,
                (ASElement)wantedProduct);
    }
    asDelete(matamazom->list_of_products,(ASElement)wantedProduct);
    return MATAMAZOM_SUCCESS;
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
        if((currentProduct->income)-(bestSellingProduct->income) >
        IN_RANGE_OF_MISTAKE){
            bestSellingProduct=currentProduct;
            max_income=bestSellingProduct->income;
        }
    }
    if(max_income==0){
        printNoBestSellingProduct(output);
        return MATAMAZOM_SUCCESS;
    }
    fprintf(output,"Best Selling Product:\n");
    mtmPrintIncomeLine(bestSellingProduct->name,bestSellingProduct->id,
            max_income,output);
    return MATAMAZOM_SUCCESS;
}

MatamazomResult mtmPrintFiltered(Matamazom matamazom,
                                    MtmFilterProduct customFilter,FILE *output){
    if(!matamazom || !customFilter || !output){
        return MATAMAZOM_NULL_ARGUMENT;
    }
    double amount_Of_Product=0;
    AS_FOREACH(Product,currentProduct,matamazom->list_of_products){
        /*AmountSetResult result = */asGetAmount(matamazom->list_of_products,
                currentProduct,&amount_Of_Product);
        if(customFilter(currentProduct->id,currentProduct->name,
                        amount_Of_Product,currentProduct->additional_info)){
            mtmPrintProductDetails(currentProduct->name,currentProduct->id,
                    amount_Of_Product,currentProduct->
                    get_price_function(currentProduct->additional_info,UNIT),
                    output);
        }
    }
    return MATAMAZOM_SUCCESS;
}

MatamazomResult mtmPrintInventory(Matamazom matamazom, FILE *output){
    if(!matamazom || !output){
        return MATAMAZOM_NULL_ARGUMENT;
    }
    fprintf(output,"Inventory Status:\n");
    printProductsOfAmountSet(matamazom->list_of_products, true, output);
    return MATAMAZOM_SUCCESS;
}

MatamazomResult mtmPrintOrder(Matamazom matamazom, const unsigned int orderId,
                                FILE *output) {
    if (!matamazom || !output) {
        return MATAMAZOM_NULL_ARGUMENT;
    }
    SET_FOREACH(Order, current_order, matamazom->set_of_orders) {
        if (current_order->id == orderId) {
            mtmPrintOrderHeading(orderId, output);
            printProductsOfAmountSet(current_order->list_of_order_products,
                                                        false,output);
            double total_price_of_order = getTotalPriceOfOrder(current_order);
            mtmPrintOrderSummary(total_price_of_order, output);
            return MATAMAZOM_SUCCESS;
        }
    }
    return MATAMAZOM_ORDER_NOT_EXIST;
}

unsigned int mtmCreateNewOrder(Matamazom matamazom){
    if(!matamazom){
        return 0;
    }
    Order new_order = malloc(sizeof(*new_order));
    if(!new_order){
        return 0;
    }

    new_order->list_of_order_products = asCreate(copyProductForAmountSet,
            freeProductForAmountSet,compareProductsForAmountSet);
    if(!new_order->list_of_order_products){
        freeOrder(new_order);
        return 0;
    }

    matamazom->current_order_id=matamazom->current_order_id+1;
    new_order->id=matamazom->current_order_id;
    // put the order in the specific matamazom
    SetResult register_new_order = setAdd(matamazom->set_of_orders,
            (SetElement)new_order);
    if(register_new_order != SET_SUCCESS){
        freeOrder(new_order);
        return 0;
    }
    unsigned int id_of_order = new_order->id;
    freeOrder(new_order);

    return id_of_order;
}

MatamazomResult mtmChangeProductAmountInOrder(Matamazom matamazom,
                                                const unsigned int orderId,
                                                const unsigned int productId,
                                                const double amount){
    if(!matamazom){
        return MATAMAZOM_NULL_ARGUMENT;
    }
    //get the wanted order
    Order wanted_order = getOrderFromId(matamazom->set_of_orders, orderId);
    if(wanted_order == NULL){
        return MATAMAZOM_ORDER_NOT_EXIST;
    }
    //check if product in warehouse
    Product product_in_warehouse = getProductFromId(matamazom->list_of_products,
            productId);
    if(product_in_warehouse == NULL){
        return MATAMAZOM_PRODUCT_NOT_EXIST;
    }
    if(!checkIfAmountIsValid(product_in_warehouse->amount_type, amount)){
        return MATAMAZOM_INVALID_AMOUNT;
    }
    //check if product is in order
    Product product_in_order = getProductFromId(wanted_order->
            list_of_order_products, productId);
    if(product_in_order == NULL){
        if(amount > 0){
            asRegister(wanted_order->list_of_order_products,
                    (ASElement)product_in_warehouse);
            asChangeAmount(wanted_order->list_of_order_products,
                    (ASElement)product_in_warehouse, amount);
        }
    } else{
        AmountSetResult change_result = asChangeAmount(wanted_order->
                list_of_order_products, (ASElement)product_in_order, amount);
        if(change_result == AS_INSUFFICIENT_AMOUNT){
            asDelete(wanted_order->list_of_order_products,
                    (ASElement)product_in_order);
        }
        double amount_of_product_in_order;
        asGetAmount(wanted_order->list_of_order_products,
                (ASElement)product_in_order, &amount_of_product_in_order);
        if(amount_of_product_in_order == 0){
            asDelete(wanted_order->list_of_order_products,
                    (ASElement)product_in_order);
        }
    }
    return MATAMAZOM_SUCCESS;
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
    // check if the amounts are ok and if not - return insufficient
    if(!checkIfOrderIsValid(matamazom,wanted_order)){
        return MATAMAZOM_INSUFFICIENT_AMOUNT;
    }
    // now the order is ok - substract all amounts from the warehouse
    double amount_of_product_in_order;
    Product warehouse_product;

    AS_FOREACH(Product,orderProduct,wanted_order->list_of_order_products){
        asGetAmount(wanted_order->list_of_order_products,orderProduct,
                &amount_of_product_in_order);
        asChangeAmount(matamazom->list_of_products,orderProduct,
                -amount_of_product_in_order);

        warehouse_product=getProductFromId(matamazom->list_of_products,
                orderProduct->id);
        warehouse_product->income=(warehouse_product->income)
                +warehouse_product->get_price_function(warehouse_product->
                additional_info,amount_of_product_in_order);
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
    setRemove(matamazom->set_of_orders, (SetElement)wanted_order);
    return MATAMAZOM_SUCCESS;
}
