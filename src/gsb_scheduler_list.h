#ifndef GSB_SCHEDULER_LIST_H
#define GSB_SCHEDULER_LIST_H

#define COL_NB_DATE 0
#define COL_NB_ACCOUNT 1
#define COL_NB_PARTY 2
#define COL_NB_FREQUENCY 3
#define COL_NB_MODE 4
#define COL_NB_NOTES 5
#define COL_NB_AMOUNT 6		/* doit Ãªtre le dernier de la liste
				   Ã  cause de plusieurs boucles for */
#define NB_COLS_SCHEDULER 7

/* define the columns in the store
 * as the data are filled above, the number here
 * begin at NB_COLS_SCHEDULER */

#define SCHEDULER_COL_NB_BACKGROUND 8		/*< color of the background */
#define SCHEDULER_COL_NB_SAVE_BACKGROUND 9	/*< when selection, save of the normal color of background */
#define SCHEDULER_COL_NB_AMOUNT_COLOR 10 	/*< color of the amount */
#define SCHEDULER_COL_NB_TRANSACTION_NUMBER 11
#define SCHEDULER_COL_NB_FONT 12		/*< PangoFontDescription if used */
#define SCHEDULER_COL_NB_VIRTUAL_TRANSACTION 13 /*< to 1 if it's a calculated scheduled transaction (for longer view), so, cannot edit */

#define SCHEDULER_COL_NB_TOTAL 14


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
void gsb_scheduler_list_append_new_scheduled ( gint scheduled_number,
					       GDate *end_date );
gboolean gsb_scheduler_list_change_scheduler_view ( enum scheduler_periodicity periodicity );
GtkWidget *gsb_scheduler_list_create_list ( void );
gboolean gsb_scheduler_list_delete_scheduled_transaction ( gint scheduled_number );
gboolean gsb_scheduler_list_edit_transaction ( gint scheduled_number );
gboolean gsb_scheduler_list_execute_transaction ( gint scheduled_number );
gboolean gsb_scheduler_list_fill_list ( GtkWidget *tree_view );
gint gsb_scheduler_list_get_current_scheduled_number ( void );
GDate *gsb_scheduler_list_get_end_date_scheduled_showed ( void );
gint gsb_scheduler_list_get_last_scheduled_number ( void );
GtkWidget *gsb_scheduler_list_get_tree_view ( void );
gboolean gsb_scheduler_list_remove_transaction_from_list ( gint scheduled_number );
gboolean gsb_scheduler_list_select ( gint scheduled_number );
gboolean gsb_scheduler_list_set_background_color ( GtkWidget *tree_view );
gboolean gsb_scheduler_list_show_notes ( void );
gboolean gsb_scheduler_list_update_transaction_in_list ( gint scheduled_number );
/* END_DECLARATION */

#endif
