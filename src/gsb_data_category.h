#ifndef _GSB_CATEGORY_DATA_H
#define _GSB_CATEGORY_DATA_H (1)

/* START_INCLUDE_H */
#include "gsb_data_category.h"
/* END_INCLUDE_H */

/* START_DECLARATION */
void gsb_data_category_add_transaction_to_category ( gint transaction_number,
						     gint category_id,
						     gint sub_category_id );
void gsb_data_category_create_default_category_list ( void );
gdouble gsb_data_category_get_balance ( gint no_category );
GSList *gsb_data_category_get_categories_list ( void );
gdouble gsb_data_category_get_direct_balance ( gint no_category );
gchar *gsb_data_category_get_name ( gint no_category,
				    gint no_sub_category,
				    gchar *return_value_error );
GSList *gsb_data_category_get_name_list ( gboolean set_debit,
					  gboolean set_credit,
					  gboolean set_special,
					  gboolean set_breakdown );
gint gsb_data_category_get_nb_direct_transactions ( gint no_category );
gint gsb_data_category_get_nb_transactions ( gint no_category );
gint gsb_data_category_get_no_category ( gpointer category_ptr );
gint gsb_data_category_get_no_category_from_sub_category ( gpointer sub_category_ptr );
gint gsb_data_category_get_no_sub_category ( gpointer sub_category_ptr );
gint gsb_data_category_get_number_by_name ( gchar *name,
					    gboolean create,
					    gint category_type );
gpointer gsb_data_category_get_structure ( gint no_category );
gdouble gsb_data_category_get_sub_category_balance ( gint no_category,
						     gint no_sub_category );
GSList *gsb_data_category_get_sub_category_list ( gint no_category );
gchar *gsb_data_category_get_sub_category_name ( gint no_category,
						 gint no_sub_category,
						 gchar *return_value_error );
gint gsb_data_category_get_sub_category_nb_transactions ( gint no_category,
							  gint no_sub_category );
gint gsb_data_category_get_sub_category_number_by_name ( gint category_number,
							 gchar *name,
							 gboolean create );
gpointer gsb_data_category_get_sub_category_structure ( gint no_category,
							gint no_sub_category );
gint gsb_data_category_get_type ( gint no_category );
gboolean gsb_data_category_init_variables ( void );
gboolean gsb_data_category_merge_category_list ( GSList *list_to_merge );
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
void gsb_data_category_update_counters ( void );
/* END_DECLARATION */
#endif
