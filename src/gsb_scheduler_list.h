#ifndef GSB_SCHEDULER_LIST_H
#define GSB_SCHEDULER_LIST_H

enum
{
    COL_NB_DATE = 0,
    COL_NB_ACCOUNT,
    COL_NB_PARTY,
    COL_NB_FREQUENCY,
    COL_NB_MODE,
    COL_NB_NOTES,
    COL_NB_AMOUNT,

    SCHEDULER_COL_NB_BACKGROUND,		/*< color of the background */
    SCHEDULER_COL_NB_SAVE_BACKGROUND,		/*< when selection, save of the normal color of background */
    SCHEDULER_COL_NB_AMOUNT_COLOR, 		/*< color of the amount */
    SCHEDULER_COL_NB_TRANSACTION_NUMBER,
    SCHEDULER_COL_NB_FONT,			/*< PangoFontDescription if used */
    SCHEDULER_COL_NB_VIRTUAL_TRANSACTION,	/*< to 1 if it's a calculated scheduled transaction (for longer view), so, cannot edit */

    SCHEDULER_COL_NB_TOTAL,
};


/* number of visible columns */
#define SCHEDULER_COL_VISIBLE_COLUMNS 7

/* define the columns in the store
 * as the data are filled above, the number here
 * begin at NB_COLS_SCHEDULER */


enum scheduler_periodicity {
    SCHEDULER_PERIODICITY_ONCE_VIEW = 0,
    SCHEDULER_PERIODICITY_WEEK_VIEW,
    SCHEDULER_PERIODICITY_MONTH_VIEW,
    SCHEDULER_PERIODICITY_TWO_MONTHS_VIEW,
    SCHEDULER_PERIODICITY_TRIMESTER_VIEW,
    SCHEDULER_PERIODICITY_YEAR_VIEW,
    SCHEDULER_PERIODICITY_CUSTOM_VIEW,
    SCHEDULER_PERIODICITY_NB_CHOICES,
};


enum periodicity_units {
    PERIODICITY_DAYS,
    PERIODICITY_WEEKS,
    PERIODICITY_MONTHS,
    PERIODICITY_YEARS,
};


/* START_INCLUDE_H */
/* END_INCLUDE_H */


/* START_DECLARATION */
gboolean gsb_scheduler_list_append_new_scheduled ( gint scheduled_number,
                        GDate *end_date );
gboolean gsb_scheduler_list_change_scheduler_view ( enum scheduler_periodicity periodicity,
                        gpointer item );
GtkWidget *gsb_scheduler_list_create_list ( void );
gboolean gsb_scheduler_list_delete_scheduled_transaction ( gint scheduled_number,
                        gboolean show_warning );
gboolean gsb_scheduler_list_delete_scheduled_transaction_by_menu ( GtkWidget *button,
                        gpointer null );
gboolean gsb_scheduler_list_edit_transaction ( gint scheduled_number );
gboolean gsb_scheduler_list_execute_transaction ( gint scheduled_number );
gboolean gsb_scheduler_list_fill_list ( GtkWidget *tree_view );
gint gsb_scheduler_list_get_current_scheduled_number ( void );
GDate *gsb_scheduler_list_get_end_date_scheduled_showed ( void );
gint gsb_scheduler_list_get_last_scheduled_number ( void );
GtkWidget *gsb_scheduler_list_get_tree_view ( void );
void gsb_scheduler_list_init_variables ( void );
gboolean gsb_scheduler_list_redraw ( void );
gboolean gsb_scheduler_list_remove_transaction_from_list ( gint scheduled_number );
gboolean gsb_scheduler_list_select ( gint scheduled_number );
gboolean gsb_scheduler_list_set_background_color ( GtkWidget *tree_view );
gboolean gsb_scheduler_list_set_largeur_col ( void );
gboolean gsb_scheduler_list_show_notes ( void );
gboolean gsb_scheduler_list_update_transaction_in_list ( gint scheduled_number );
/* END_DECLARATION */

#endif
