#ifndef _GSB_DATA_ARCHIVE_STORE_H
#define _GSB_DATA_ARCHIVE_STORE_H (1)


/* START_INCLUDE_H */
#include "gsb_real.h"
/* END_INCLUDE_H */


/**
 * \struct
 * Describe an archive store
 */
typedef struct _StoreArchive	StoreArchive;

struct _StoreArchive
{
    gint archive_store_number;

    /* the corresponding archive (1 archive contains several archive store) */
    gint archive_number;

    /* account we are working on */
    gint account_number;

    /* balance of all the transactions of the archive for that account */
    GsbReal balance;

    /* number of transactions in the archive for that account */
    gint nb_transactions;

    /* les transactions archivées sont visibles dans la vue des opérations FALSE par défaut */
    gboolean transactions_visibles;
};



/* START_DECLARATION */
gboolean 	gsb_data_archive_store_account_have_transactions_visibles 	(gint account_number);
void 		gsb_data_archive_store_create_list 							(void);
gint 		gsb_data_archive_store_get_account_number 					(gint archive_store_number);
gint 		gsb_data_archive_store_get_archive_number 					(gint archive_store_number);
GsbReal 	gsb_data_archive_store_get_archives_balance 				(gint account_number);
GSList *	gsb_data_archive_store_get_archives_list 					(void);
GsbReal 	gsb_data_archive_store_get_balance 							(gint archive_store_number);
gint 		gsb_data_archive_store_get_number 							(gpointer archive_ptr);
gpointer 	gsb_data_archive_store_get_structure 						(gint archive_store_number);
gint 		gsb_data_archive_store_get_transactions_number 				(gint archive_store_number);
gboolean 	gsb_data_archive_store_get_transactions_visibles 			(gint archive_number,
																		 gint account_number);
gboolean 	gsb_data_archive_store_init_variables 						(void);
gboolean 	gsb_data_archive_store_remove 								(gint archive_store_number);
gboolean 	gsb_data_archive_store_remove_by_archive 					(gint archive_number);
gboolean 	gsb_data_archive_store_set_transactions_visibles 			(gint archive_number,
																		 gint account_number,
																		 gboolean transactions_visibles);
/* END_DECLARATION */
#endif
