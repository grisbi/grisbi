#ifndef _GSB_DATA_FYEAR_H
#define _GSB_DATA_FYEAR_H (1)

/**
 * \struct 
 * Describe a fyear 
 */
typedef struct
{
    guint fyear_number;
    gchar *fyear_name;
    GDate *beginning_date;
    GDate *end_date;
    gboolean showed_in_form;

    /* 0 if the fyear is valid, >0 if invalid (the number
     * contains why it's invalid) */
    gint invalid_fyear;
} struct_fyear;


/* START_INCLUDE_H */
/* END_INCLUDE_H */

/* START_DECLARATION */
void gsb_data_fyear_check_all_for_invalid ( void );
gboolean gsb_data_fyear_check_for_invalid ( gint fyear_number );
gint gsb_data_fyear_compare ( gint fyear_number_1, gint fyear_number_2 );
gint gsb_data_fyear_compare_from_struct ( struct_fyear *fyear_1,
                        struct_fyear *fyear_2 );
GDate *gsb_data_fyear_get_beginning_date ( gint fyear_number );
GDate *gsb_data_fyear_get_end_date ( gint fyear_number );
gboolean gsb_data_fyear_get_form_show ( gint fyear_number );
gint gsb_data_fyear_get_from_date ( const GDate *date );
GSList *gsb_data_fyear_get_fyears_list ( void );
gint gsb_data_fyear_get_invalid ( gint fyear_number );
const gchar *gsb_data_fyear_get_invalid_message ( gint fyear_number );
const gchar *gsb_data_fyear_get_name ( gint fyear_number );
gint gsb_data_fyear_get_no_fyear ( gpointer fyear_ptr );
gboolean gsb_data_fyear_init_variables ( void );
gint gsb_data_fyear_new ( const gchar *name );
gboolean gsb_data_fyear_remove ( gint fyear_number );
gboolean gsb_data_fyear_set_beginning_date ( gint fyear_number,
					    GDate *date );
gboolean gsb_data_fyear_set_end_date ( gint fyear_number,
				       GDate *date );
gboolean gsb_data_fyear_set_form_show ( gint fyear_number,
					gboolean showed_in_form );
gboolean gsb_data_fyear_set_name ( gint fyear_number,
				   const gchar *name );
gint gsb_data_fyear_set_new_number ( gint fyear_number,
                        gint new_no_fyear );
/* END_DECLARATION */
#endif
