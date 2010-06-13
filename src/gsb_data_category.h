#ifndef _GSB_CATEGORY_DATA_H
#define _GSB_CATEGORY_DATA_H (1)

/* START_INCLUDE_H */
#include "gsb_real.h"
/* END_INCLUDE_H */

/* used to choose the kind of categories list */
enum category_choice_values {
    CATEGORY_CHOICE_NONE = 0,
    CATEGORY_CHOICE_DEFAULT,
    CATEGORY_CHOICE_ASSOCIATION,
    CATEGORY_CHOICE_LIBERAL
};


/* START_DECLARATION */
void gsb_data_category_add_transaction_to_category ( gint transaction_number,
						     gint category_id,
						     gint sub_category_id );
gboolean gsb_data_category_fill_transaction_by_string ( gint transaction_number,
                        const gchar *string,
                        gboolean is_transaction );
gsb_real gsb_data_category_get_balance ( gint no_category );
GSList *gsb_data_category_get_categories_list ( void );
gsb_real gsb_data_category_get_direct_balance ( gint no_category );
gpointer gsb_data_category_get_empty_category ( void );
gchar *gsb_data_category_get_name ( gint no_category,
				    gint no_sub_category,
				    const gchar *return_value_error );
GSList *gsb_data_category_get_name_list ( gboolean set_debit,
					  gboolean set_credit,
					  gboolean set_special,
					  gboolean set_split );
gint gsb_data_category_get_nb_direct_transactions ( gint no_category );
gint gsb_data_category_get_nb_transactions ( gint no_category );
gint gsb_data_category_get_no_category ( gpointer category_ptr );
gint gsb_data_category_get_no_sub_category ( gpointer sub_category_ptr );
gint gsb_data_category_get_number_by_name ( const gchar *name,
                        gboolean create,
					    gint category_type );
gpointer gsb_data_category_get_structure ( gint no_category );
gsb_real gsb_data_category_get_sub_category_balance ( gint no_category,
						      gint no_sub_category );
GSList *gsb_data_category_get_sub_category_list ( gint no_category );
gchar *gsb_data_category_get_sub_category_name ( gint no_category,
						 gint no_sub_category,
						 const gchar *return_value_error );
gint gsb_data_category_get_sub_category_nb_transactions ( gint no_category,
							  gint no_sub_category );
gint gsb_data_category_get_sub_category_number_by_name ( gint category_number,
							 const gchar *name,
							 gboolean create );
gpointer gsb_data_category_get_sub_category_structure ( gint no_category,
                        gint no_sub_category );
gint gsb_data_category_get_type ( gint no_category );
gboolean gsb_data_category_init_variables ( void );
gint gsb_data_category_new_sub_category_with_number ( gint number,
                        gint category_number);
gint gsb_data_category_new_with_number ( gint number );
gboolean gsb_data_category_remove ( gint no_category );
void gsb_data_category_remove_transaction_from_category ( gint transaction_number );
gboolean gsb_data_category_set_name ( gint no_category,
				      const gchar *name );
gboolean gsb_data_category_set_sub_category_name ( gint no_category,
						   gint no_sub_category,
						   const gchar *name );
gboolean gsb_data_category_set_type ( gint no_category,
				      gint category_type );
gboolean gsb_data_category_sub_category_remove ( gint no_category,
                        gint no_sub_category );
gint gsb_data_category_test_create_category ( gint no_category,
                        const gchar *name,
                        gint category_type );
gboolean gsb_data_category_test_create_sub_category ( gint no_category,
                        gint no_sub_category,
                        const gchar *name );
void gsb_data_category_update_counters ( void );
gchar * gsb_debug_duplicate_categ_check ();
gboolean gsb_debug_duplicate_categ_fix ();
/* END_DECLARATION */
#endif
