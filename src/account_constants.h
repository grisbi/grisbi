#ifndef ACCOUNT_CONSTANTS 
#define ACCOUNT_CONSTANTS

/** \struct
 * kind of the account
 * */

enum _kind_account
{
    GSB_TYPE_BANK        = 0,
    GSB_TYPE_CASH        = 1,
    GSB_TYPE_LIABILITIES = 2,
    GSB_TYPE_ASSET       = 3
};
typedef enum _kind_account	kind_account;


#endif
