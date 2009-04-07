#ifndef _GSB_DATA_BANK_H
#define _GSB_DATA_BANK_H (1)


/* START_INCLUDE_H */
/* END_INCLUDE_H */

/* START_DECLARATION */
const gchar *gsb_data_bank_get_bank_address ( gint bank_number );
GSList *gsb_data_bank_get_bank_list ( void );
const gchar *gsb_data_bank_get_bank_mail ( gint bank_number );
const gchar *gsb_data_bank_get_bank_note ( gint bank_number );
const gchar *gsb_data_bank_get_bank_tel ( gint bank_number );
const gchar *gsb_data_bank_get_bank_web ( gint bank_number );
const gchar *gsb_data_bank_get_bic ( gint bank_number );
const gchar *gsb_data_bank_get_code ( gint bank_number );
const gchar *gsb_data_bank_get_correspondent_fax ( gint bank_number );
const gchar *gsb_data_bank_get_correspondent_mail ( gint bank_number );
const gchar *gsb_data_bank_get_correspondent_name ( gint bank_number );
const gchar *gsb_data_bank_get_correspondent_tel ( gint bank_number );
const gchar *gsb_data_bank_get_name ( gint bank_number );
gint gsb_data_bank_get_no_bank ( gpointer bank_ptr );
gboolean gsb_data_bank_init_variables ( void );
gint gsb_data_bank_max_number ( void );
gint gsb_data_bank_new ( const gchar *name );
gboolean gsb_data_bank_remove ( gint bank_number );
gboolean gsb_data_bank_set_bank_address ( gint bank_number,
					  const gchar *bank_address );
gboolean gsb_data_bank_set_bank_mail ( gint bank_number,
				       const gchar *bank_mail );
gboolean gsb_data_bank_set_bank_note ( gint bank_number,
				       const gchar *bank_note );
gboolean gsb_data_bank_set_bank_tel ( gint bank_number,
				      const gchar *bank_tel );
gboolean gsb_data_bank_set_bank_web ( gint bank_number,
				      const gchar *bank_web );
gboolean gsb_data_bank_set_bic ( gint bank_number, const gchar *bank_BIC );
gboolean gsb_data_bank_set_code ( gint bank_number,
				  const gchar *bank_code );
gboolean gsb_data_bank_set_correspondent_fax ( gint bank_number,
					       const gchar *correspondent_fax );
gboolean gsb_data_bank_set_correspondent_mail ( gint bank_number,
						const gchar *correspondent_mail );
gboolean gsb_data_bank_set_correspondent_name ( gint bank_number,
						const gchar *correspondent_name );
gboolean gsb_data_bank_set_correspondent_tel ( gint bank_number,
					       const gchar *correspondent_tel );
gboolean gsb_data_bank_set_name ( gint bank_number,
				  const gchar *name );
gint gsb_data_bank_set_new_number ( gint bank_number,
				    gint new_no_bank );
/* END_DECLARATION */
#endif
