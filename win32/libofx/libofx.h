/***************************************************************************
              libofx.h  -  Main header file for the libofx API
                             -------------------
    copyright            : (C) 2002 by Benoit Grégoire
    email                : bock@step.polymtl.ca
***************************************************************************/
/**@file
 * \brief Main header file containing the LibOfx API
 *
 This file should be included for all applications who use this API.  This
 header file will work with both C and C++ programs.  The entire API is 
 made of the following structures and functions. 
 *
 All of the following ofx_proc_* functions are callbacks (Except
 ofx_proc_file which is the entry point).  They must be implemented by 
 your program, but can be left empty if not needed. They are called each
 time the associated structure is filled by the library.
 *
 Important note:  The variables associated with every data element have a 
 *_valid companion.  Always check that data_valid == true before using. 
 Not only will you ensure that the data is meaningfull, but also that
 pointers are valid and strings point to a null terminated string.  
 Elements listed as mandatory are for information purpose only, do not 
 trust the bank not to send you non-conforming data...
*/
/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef LIBOFX_H
#define LIBOFX_H
#include <time.h>
#ifdef __cplusplus
#define CFCT extern "C"
#else
#define CFCT
#define true 1
#define false 0
#endif

#define OFX_ELEMENT_NAME_LENGTH         100
#define OFX_SVRTID2_LENGTH             36 + 1
#define OFX_CHECK_NUMBER_LENGTH        12 + 1
#define OFX_REFERENCE_NUMBER_LENGTH    32 + 1
#define OFX_FITID_LENGTH               255 + 1
#define OFX_TOKEN2_LENGTH              36 + 1
#define OFX_MEMO2_LENGTH               390 + 1
#define OFX_BALANCE_NAME_LENGTH        32 + 1
#define OFX_BALANCE_DESCRIPTION_LENGTH 80 + 1
#define OFX_CURRENCY_LENGTH            3 + 1 /* In ISO-4217 format */
#define OFX_BANKID_LENGTH              9
#define OFX_BRANCHID_LENGTH            22 + 1
#define OFX_ACCTID_LENGTH              22 + 1 
#define OFX_ACCTKEY_LENGTH             22 + 1
#define OFX_BROKERID_LENGTH            22 + 1
/* Must be MAX of <BANKID>+<BRANCHID>+<ACCTID>, <ACCTID>+<ACCTKEY> and <ACCTID>+<BROKERID> */
#define OFX_ACCOUNT_ID_LENGTH OFX_BANKID_LENGTH + OFX_BRANCHID_LENGTH + OFX_ACCTID_LENGTH + 1
#define OFX_ACCOUNT_NAME_LENGTH        255
#define OFX_MARKETING_INFO_LENGTH      360 + 1
#define OFX_TRANSACTION_NAME_LENGTH    32 + 1
#define OFX_UNIQUE_ID_LENGTH           32 + 1
#define OFX_UNIQUE_ID_TYPE_LENGTH      10 + 1
#define OFX_SECNAME_LENGTH             32 + 1
#define OFX_TICKER_LENGTH              32 + 1

CFCT void (*OfxCallbackFunc) ();


/**
 * \brief ofx_proc_file is the entry point of the library.  
 *
 *  libofx_proc_file must be called by the client, with a list of 1 or more OFX
 files to be parsed in command line format.
*/
CFCT int ofx_proc_file(int argc, char *argv[]);


/**
 * \brief An abstraction of an OFX STATUS element.
 *
 * The OfxStatusData structure represents a STATUS OFX element sent by the 
 OFX server.  Be carefull, you do not have much context except the entity 
 name so your application should probably ignore this status if code==0. 
 However, you should display a message if the status in non-zero,
 since an error probably occurred on the server side.  
 *
 * In a future version of this API, OfxStatusData structures might be
 linked from the OFX structures they are related to.
*/
struct OfxStatusData{  
  /** @name Additional information
   * To give a minimum of context, the name of the OFX SGML element where
   this <STATUS> is located is available.
  */
  char ofx_element_name[OFX_ELEMENT_NAME_LENGTH];/** Name of the OFX element
						     this status is relevant to */
  int ofx_element_name_valid;
  
  /** @name OFX mandatory elements
   * The OFX spec defines the following elements as mandatory.  The associated
   variables should all contain valid data but you should not trust the servers.
   Check if the associated *_valid is true before using them. */
  int code;            /**< Status code */
  char* name;          /**< Code short name */
  char* description;   /**< Code long description, from ofx_error_msg.h */
  int code_valid;      /**< If  code_valid is true, so is name and description
			  (They are obtained from a lookup table) */
  /** Severity of the error */
    enum Severity{OFX_INFO, /**< The status is an informational message */
		OFX_WARN, /**< The status is a warning */
		OFX_ERROR /**< The status is a true error */
  } severity;
  int severity_valid;
  
  /** @name OFX optional elements
   *  The OFX spec defines the following elements as optional. If the 
   associated *_valid is true, the corresponding element is present and the 
   associated variable contains valid data. */  

  char* server_message; /**< Explanation given by the server for the Status Code.
			   Especially important for generic errors. */
  int server_message_valid;
  /*@}*/
};


/**
 * \brief The callback function for the OfxStatusData stucture. 
 *
 * An ofx_proc_status_cb event is sent everytime the server has generated a OFX
 STATUS element.  As such, it could be received at any time(but not during
 other events).  An OfxStatusData structure is passed to this even.
*/
CFCT int ofx_proc_status_cb(const struct OfxStatusData data);

/**
 * \brief An abstraction of an account
 *
 *  The OfxAccountData structure gives information about a specific account, 
 including it's type, currency and unique id. 
*/
struct OfxAccountData{
  
  /** @name OFX mandatory elements
   * The OFX spec defines the following elements as mandatory.  The associated
   variables should all contain valid data but you should not trust the servers.
   Check if the associated *_valid is true before using them. */  

  /** The account_id is actually built from <BANKID><BRANCHID><ACCTID> for
      a bank account, and <ACCTID><ACCTKEY> for a credit card account.
      account_id is meant to be computer-readable.  It is a worldwide OFX unique
      identifier wich can be used for account matching, even in system with
      multiple users.*/
  char account_id[OFX_ACCOUNT_ID_LENGTH];
  /** The account_id_name is a string meant to allow the user to identify the
      account.  Currently it is <ACCTID> for a bank account and a credit 
      card account an <BROKERID>:<ACCTID> for investment accounts.
      account_id_name is not meant to be computer-readable and
      is not garanteed to be unique.*/
  char account_name[OFX_ACCOUNT_NAME_LENGTH];
  int account_id_valid;/* Use for both account_id and account_name */

  /** account_type tells you what kind of account this is.  See the AccountType enum */
    enum AccountType{
    OFX_CHECKING,  /**< A standard checking account */
    OFX_SAVINGS,   /**< A standard savings account */
    OFX_MONEYMRKT, /**< A money market account */
    OFX_CREDITLINE,/**< A line of credit */
    OFX_CMA,       /**< Cash Management Account */
    OFX_CREDITCARD,/**< A credit card account */
    OFX_INVESTMENT /**< An investment account */
  } account_type;
  int account_type_valid;
  char currency[OFX_CURRENCY_LENGTH]; /**< The currency is a string in ISO-4217 format */
  int currency_valid;

};

/**
 * \brief The callback function for the OfxAccountData stucture. 
 *
 * The ofx_proc_account_cb event is always generated first, to allow the
 application to create accounts or ask the user to match an existing
 account before the ofx_proc_statement and ofx_proc_transaction event are
 received.  An OfxAccountData is passed to this event.
 *
 Note however that this OfxAccountData structure will also be available as
 part of OfxStatementData structure passed to  ofx_proc_statement event.
*/
CFCT int ofx_proc_account_cb(const struct OfxAccountData data);

/**
 * \brief An abstraction of a security, such as a stock, mutual fund, etc.
 *
 * The OfxSecurityData stucture is used to hols the securyty information inside
a OfxTransactionData struct for investment transactions.
*/
struct OfxSecurityData{
  /** @name OFX mandatory elements
   * The OFX spec defines the following elements as mandatory.  The associated
   variables should all contain valid data but you should not trust the servers.
   Check if the associated *_valid is true before using them. */  

  char unique_id[OFX_UNIQUE_ID_LENGTH];   /**< The id of the security being traded.*/
  int unique_id_valid;
  char unique_id_type[OFX_UNIQUE_ID_TYPE_LENGTH];/**< Usially "CUSIP" for FIs in
						    north america*/ 
  int unique_id_type_valid;
  char secname[OFX_SECNAME_LENGTH];/**< The full name of the security */ 
  int secname_valid;

  /** @name OFX optional elements
   *  The OFX spec defines the following elements as optional. If the 
   associated *_valid is true, the corresponding element is present and
   the associated variable contains valid data. */  

  char ticker[OFX_TICKER_LENGTH];/**< The ticker symbol of the security */ 
  int ticker_valid;

  double unitprice;/**< The price of each unit of the security, as of  
		      date_unitprice */
  int unitprice_valid;

  time_t date_unitprice;/**< The date as of which the unit price was valid. */
  int date_unitprice_valid;

  char currency[OFX_CURRENCY_LENGTH]; /**< The currency is a string in ISO-4217 format.
					 It overrides the one defined in the statement
					 for the unit price */
  int currency_valid;
  char memo[OFX_MEMO2_LENGTH];/**< Extra information not included in name */
  int memo_valid;
};/* end struct OfxSecurityData */

/** 
 * \brief The callback function for the OfxSecurityData stucture. 
 *
 * An ofx_proc_security_cb event is generated for any securities listed in the
 ofx file.  It is generated after ofx_proc_statement but before 
 ofx_proc_transaction. It is meant to be used to allow the client to 
 create a new commodity or security (such as a new stock type).  Please note however
 that this information is usually also available as part of each OfxtransactionData.
 An OfxSecurityData structure is passed to this event.
*/
CFCT int ofx_proc_security_cb(const struct OfxSecurityData data);


/**
 * \brief An abstraction of a transaction in an account.
 *
 * The OfxTransactionData stucture contains all available information about
 an actual transaction in an account.
*/
struct OfxTransactionData{
  
  /** @name OFX mandatory elements
   * The OFX spec defines the following elements as mandatory.  The associated
   variables should all contain valid data but you should not trust the servers.
   Check if the associated *_valid is true before using them. */  

  char account_id[OFX_ACCOUNT_ID_LENGTH];/**< Use this for matching with
					    the relevant account in your
					    application */
  struct OfxAccountData * account_ptr; /**< Pointer to the full account structure,
					  see OfxAccountData */
  int account_id_valid;
  enum TransactionType{
    OFX_CREDIT,     /**< Generic credit */
    OFX_DEBIT,      /**< Generic debit */
    OFX_INT,        /**< Interest earned or paid (Note: Depends on signage of amount) */
    OFX_DIV,        /**< Dividend */
    OFX_FEE,        /**< FI fee */
    OFX_SRVCHG,     /**< Service charge */
    OFX_DEP,        /**< Deposit */
    OFX_ATM,        /**< ATM debit or credit (Note: Depends on signage of amount) */
    OFX_POS,        /**< Point of sale debit or credit (Note: Depends on signage of amount) */
    OFX_XFER,       /**< Transfer */
    OFX_CHECK,      /**< Check */
    OFX_PAYMENT,    /**< Electronic payment */
    OFX_CASH,       /**< Cash withdrawal */
    OFX_DIRECTDEP,  /**< Direct deposit */
    OFX_DIRECTDEBIT,/**< Merchant initiated debit */
    OFX_REPEATPMT,  /**< Repeating payment/standing order */
    OFX_OTHER       /**< Somer other type of transaction */
  } transactiontype;
  int transactiontype_valid;
 
  /**< Investment transaction type.  You should read this if 
     transactiontype == OFX_OTHER.  See OFX spec 1.6 p.442 to 445 
     for details*/
  enum InvTransactionType{
    OFX_BUYDEBT,        /**< Buy debt security */
    OFX_BUYMF,          /**< Buy mutual fund */
    OFX_BUYOPT,         /**< Buy option */
    OFX_BUYOTHER,       /**< Buy other security type */
    OFX_BUYSTOCK,       /**< Buy stock */
    OFX_CLOSUREOPT,     /**< Close a position for an option */
    OFX_INCOME,         /**< Investment income is realized as cash into the investment account */
    OFX_INVEXPENSE,     /**< Misc investment expense that is associated with a specific security */
    OFX_JRNLFUND,       /**< Journaling cash holdings between subaccounts within the same investment account */
    OFX_JRNLSEC,        /**< Journaling security holdings between subaccounts within the same investment account */
    OFX_MARGININTEREST, /**< Margin interest expense */
    OFX_REINVEST,       /**< Reinvestment of income */
    OFX_RETOFCAP,       /**< Return of capital */
    OFX_SELLDEBT,       /**< Sell debt security.  Used when debt is sold, called, or reached maturity */
    OFX_SELLMF,         /**< Sell mutual fund */
    OFX_SELLOPT,        /**< Sell option */
    OFX_SELLOTHER,      /**< Sell other type of security */
    OFX_SELLSTOCK,      /**< Sell stock */
    OFX_SPLIT,          /**< Stock or mutial fund split */
    OFX_TRANSFER        /**< Transfer holdings in and out of the investment account */
  }  invtransactiontype;

 int  invtransactiontype_valid;

  double units;     /**< Variation of the number of units of the commodity
		     Suppose units is -10, ave unitprice is 1.  If the 
		     commodity is stock, you have 10 less stock, but 10 more 
		     dollars in you amccount (fees not considered, see amount).
		     If commodity is money, you have 10 less dollars in your
		     pocket, but 10 more in your account */
  int units_valid;
  double unitprice; /**< Value of each unit, 1.00 if the commodity is
		       money */
  int unitprice_valid;
  double amount;    /**< Total monetary amount of the transaction, signage 
		       will determine if money went in or out. 
		       amount is the total amount:
		       -(units) * unitprice - various fees */
  int amount_valid;
  char fi_id[256];  /**< Generated by the financial institution (fi),
			unique id of the transaction, to be used to detect
			duplicate downloads */
  int fi_id_valid;
  
  /** @name OFX optional elements
   *  The OFX spec defines the following elements as optional. If the 
   associated *_valid is true, the corresponding element is present and
   the associated variable contains valid data. */  
  
  /** The id of the security being traded. Mandatory for investment
      transactions */
  char unique_id[OFX_UNIQUE_ID_LENGTH];  
  int unique_id_valid;
  char unique_id_type[OFX_UNIQUE_ID_TYPE_LENGTH];/**< Usially "CUSIP" for FIs in
						    north america*/ 
  int unique_id_type_valid;
  struct OfxSecurityData *security_data_ptr;  /** A pointer to the security's data.*/
  int security_data_valid;
  
  time_t date_posted;/**< Date the transaction took effect (ex: date it
			appeared on your credit card bill).  Setlement date;
			for stock split, execution date.
			*
			Mandatory for bank and credit card transactions */
  int date_posted_valid;
  
  time_t date_initiated;/**< Date the transaction was initiated (ex: 
			   date you bought something in a store for credit card; 
			   trade date for stocks;
			   day of record for stock split)
			   *
			   Mandatory for investment transactions */
  int date_initiated_valid;
  time_t date_funds_available;/**< Date the funds are available (not always
				 provided) (ex: the date you are allowed to 
				 withdraw a deposit */
  int date_funds_available_valid;
  /** IMPORTANT: if  fi_id_corrected is present, this transaction
      is meant to replace or delete the transaction with this fi_id. See
      OfxTransactionData::fi_id_correction_action to know what to do. */
  char fi_id_corrected[256];
  int fi_id_corrected_valid;
  /** The OfxTransactionData::FiIdCorrectionAction enum contains the action
      to be taken */
  enum FiIdCorrectionAction{
    OFX_DELETE, /**< The transaction with a fi_id matching fi_id_corrected should
	       be deleted */
    OFX_REPLACE /**< The transaction with a fi_id matching fi_id_corrected should 
	       be replaced with this one */
  } fi_id_correction_action;
  int fi_id_correction_action_valid;
  
  /** Used for user initiated transaction such as payment or funds transfer.
      Can be seen as a confirmation number. */
  char server_transaction_id[OFX_SVRTID2_LENGTH];
  int server_transaction_id_valid;
  /** The check number is most likely an integer and can probably be 
      converted properly with atoi().  However the spec allows for up to
      12 digits, so it is not garanteed to work */
char check_number[OFX_CHECK_NUMBER_LENGTH];
  int check_number_valid;
  /** Might present in addition to or instead of a check_number. 
      Not necessarily a number */
  char reference_number[OFX_REFERENCE_NUMBER_LENGTH];
  int reference_number_valid;
  long int standard_industrial_code;/**< The standard industrial code can have
				       at most 6 digits */
  int standard_industrial_code_valid;
  char payee_id[OFX_SVRTID2_LENGTH];/**< The identifier of the payee */
  int payee_id_valid;
  char name[OFX_TRANSACTION_NAME_LENGTH];/**< Can be the name of the payee or
					    the description of the transaction */
  int name_valid;
  char memo[OFX_MEMO2_LENGTH];/**< Extra information not included in name */
  int memo_valid;

  /*********** NOT YET COMPLETE!!! *********************/
};

/** 
 * \brief The callback function for the OfxTransactionData stucture. 
 *
 * An ofx_proc_transaction_cb event is generated for every transaction in the 
 ofx response, after ofx_proc_statement (and possibly ofx_proc_security is 
 generated. An OfxTransactionData structure is passed to this event.
*/
CFCT int ofx_proc_transaction_cb(const struct OfxTransactionData data);

/** 
 * \brief An abstraction of an account statement. 
 *
 * The OfxStatementData structure contains information about your account
 at the time the ofx response was generated, including the balance.  A 
 client should check that the total of his recorded transactions matches
 the total given here, and warn the user if they dont.
*/
struct OfxStatementData{

  /** @name OFX mandatory elements
   * The OFX spec defines the following elements as mandatory.  The 
   associated variables should all contain valid data but you should not 
   trust the servers. Check if the associated *_valid is true before using
   them.
  */  

  char currency[OFX_CURRENCY_LENGTH]; /**< The currency is a string in ISO-4217 format */
  int currency_valid;
  char account_id[OFX_ACCOUNT_ID_LENGTH];/**< Use this for matching this statement with
					    the relevant account in your application */
  struct OfxAccountData * account_ptr; /**< Pointer to the full account structure, see 
				      OfxAccountData */
  int account_id_valid;
   /** The actual balance, according to the FI.  The user should be warned
       of any discrepency between this and the balance in the application */
  double ledger_balance;
  int ledger_balance_valid;
  time_t ledger_balance_date;/**< Time of the ledger_balance snapshot */
  int ledger_balance_date_valid;
  
  /** @name OFX optional elements
   *  The OFX spec defines the following elements as optional. If the 
   associated *_valid is true, the corresponding element is present and the 
   associated variable contains valid data. */  
  
  double available_balance; /**< Amount of money available from the account.
			       Could be the credit left for a credit card, 
			       or amount that can be withdrawn using INTERAC) */
  int available_balance_valid;
  time_t available_balance_date;/** Time of the available_balance snapshot */
  int available_balance_date_valid;
  /** The start time of this statement.
   *
   All the transactions between date_start and date_end should have been 
   provided */
  time_t date_start;
  int date_start_valid;
   /** The end time of this statement.
    *
    If provided, the user can use this date as the start date of his next
    statement request.  He is then assured not to miss any transactions. */
  time_t date_end;
  int date_end_valid;
  /** marketing_info could be special offers or messages from the bank,
      or just about anything else */
  char marketing_info[OFX_MARKETING_INFO_LENGTH];
  int marketing_info_valid;
};

/**
 * \brief The callback function for the OfxStatementData stucture. 
 *
 * The ofx_proc_statement_cb event is sent after all ofx_proc_transaction 
 events have been sent. An OfxStatementData is passed to this event.
*/
CFCT int ofx_proc_statement_cb(const struct OfxStatementData data);

/** 
    \brief NOT YET SUPPORTED
*/
struct OfxCurrency{
  char currency[OFX_CURRENCY_LENGTH]; /**< Currency in ISO-4217 format */
  double exchange_rate;  /**< Exchange rate from the normal currency of the account */
  int must_convert;   /**< true or false */
};

#endif

