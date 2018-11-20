/* ************************************************************************** */
/*                                                                            */
/*     Copyright (C) 2007 Dominique Parisot                                   */
/*          zionly@free.org                                                   */
/*          2008-2012 Pierre Biava (grisbi@pierre.biava.name)                 */
/*          http://www.grisbi.org                                             */
/*                                                                            */
/*  This program is free software; you can redistribute it and/or modify      */
/*  it under the terms of the GNU General Public License as published by      */
/*  the Free Software Foundation; either version 2 of the License, or         */
/*  (at your option) any later version.                                       */
/*                                                                            */
/*  This program is distributed in the hope that it will be useful,           */
/*  but WITHOUT ANY WARRANTY; without even the implied warranty of            */
/*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the             */
/*  GNU General Public License for more details.                              */
/*                                                                            */
/*  You should have received a copy of the GNU General Public License         */
/*  along with this program; if not, write to the Free Software               */
/*  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA */
/*                                                                            */
/* ************************************************************************** */

/* ./configure --with-balance-estimate */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "include.h"
#include <gdk/gdkkeysyms.h>
#include <glib/gi18n.h>

/*START_INCLUDE*/
#include "bet_future.h"
#include "bet_tab.h"
#include "dialog.h"
#include "grisbi_app.h"
#include "gsb_calendar_entry.h"
#include "gsb_combo_box.h"
#include "gsb_data_account.h"
#include "gsb_data_budget.h"
#include "gsb_data_category.h"
#include "gsb_data_currency_link.h"
#include "gsb_data_form.h"
#include "gsb_data_partial_balance.h"
#include "gsb_data_payee.h"
#include "gsb_data_payment.h"
#include "gsb_form.h"
#include "gsb_form_scheduler.h"
#include "gsb_form_widget.h"
#include "gsb_fyear.h"
#include "gsb_payment_method.h"
#include "gtk_combofix.h"
#include "navigation.h"
#include "structures.h"
#include "utils.h"
#include "utils_dates.h"
#include "utils_editables.h"
#include "utils_real.h"
#include "utils_str.h"
#include "erreur.h"
/*END_INCLUDE*/


/*START_STATIC*/
static gboolean bet_form_button_press_event ( GtkWidget *entry,
                        GdkEventButton *ev,
                        gint *ptr_origin );
static gboolean bet_form_clean ( gint account_number );
static gboolean bet_form_create_current_form ( GtkWidget *dialog,
                        GtkWidget *table,
                        gint account_number );
static gboolean bet_form_create_scheduler_part ( GtkWidget *dialog, GtkWidget *table );
static gboolean bet_form_entry_get_focus ( GtkWidget *entry,
                        GdkEventFocus *ev,
                        GtkWidget *dialog );
static gboolean bet_form_entry_lose_focus ( GtkWidget *entry,
                        GdkEventFocus *ev,
                        gint *ptr_origin );
static gboolean bet_form_key_press_event ( GtkWidget *widget,
                        GdkEventKey *ev,
                        gint *ptr_origin );
static gboolean bet_form_scheduler_frequency_button_changed ( GtkWidget *combo_box,
                        GtkWidget *dialog );
static GtkWidget *bet_form_scheduler_get_element_widget ( gint element_number );
static GtkWidget *bet_form_widget_get_widget ( gint element_number );
static GtkWidget *bet_future_create_dialog ( gint account_number );
static gboolean bet_future_set_form_data_from_line ( gint account_number,
                        gint number  );
static gboolean bet_future_take_data_from_form (  FuturData *scheduled );
/* static void bet_transfert_auto_inc_toggle ( GtkToggleButton *button, gpointer data ); */
static GtkWidget *bet_transfert_create_account_list_part ( GtkWidget *dialog, gint account_number );
static gboolean bet_transfert_entry_lose_focus ( GtkWidget *entry,
                        GdkEventFocus *ev,
                        gint *ptr_origin );
static gboolean bet_transfert_selection_changed ( GtkTreeSelection *selection, GtkWidget *dialog );
/*END_STATIC*/

/*START_EXTERN*/
/*END_EXTERN*/

/* dimmension du formulaire de données futures */
#define BET_FUTURE_FORM_WIDTH 4
#define BET_FUTURE_FORM_HEIGHT 5

/** contains the list of the scheduled elements, ie list of link
 * between an element number and the pointer of its widget
 * for now, this list if filled at the opening of grisbi and never erased */
static GSList *bet_schedul_element_list = NULL;

/** contains a list of struct_element according to the current form */
static GSList *bet_form_list_widgets = NULL;

static GtkWidget *bet_futur_dialog = NULL;
static GtkWidget *bet_transfert_dialog = NULL;

/* organisation du formulaire */
struct BetFormOrganization
{
    /* 4 columns */
    gint columns;
    /* 5 rows */
    gint rows;
    gint form_table[BET_FUTURE_FORM_HEIGHT][BET_FUTURE_FORM_WIDTH];
};

static struct BetFormOrganization *bfo;

/**
 *
 *
 *
 *
 * */
void bet_future_initialise_dialog ( void )
 {
    gint tab[BET_FUTURE_FORM_HEIGHT][BET_FUTURE_FORM_WIDTH] = {
        { SCHEDULED_FORM_FREQUENCY_BUTTON, SCHEDULED_FORM_LIMIT_DATE, SCHEDULED_FORM_FREQUENCY_USER_ENTRY,
          SCHEDULED_FORM_FREQUENCY_USER_BUTTON },
        { TRANSACTION_FORM_DATE, TRANSACTION_FORM_PARTY, TRANSACTION_FORM_DEBIT, TRANSACTION_FORM_CREDIT },
        { TRANSACTION_FORM_EXERCICE, TRANSACTION_FORM_CATEGORY, TRANSACTION_FORM_TYPE, 0 },
        { 0, TRANSACTION_FORM_BUDGET, 0, 0 },
        { TRANSACTION_FORM_NOTES, 0, 0, 0 }
    };
    gint i, j;

    if ( bet_futur_dialog != NULL )
    {
        gtk_widget_destroy ( bet_futur_dialog );
        bet_futur_dialog = NULL;
    }

    if ( bet_transfert_dialog != NULL )
    {
        gtk_widget_destroy ( bet_transfert_dialog );
        bet_transfert_dialog = NULL;
    }

    bfo = g_malloc0 ( sizeof ( struct BetFormOrganization ) );

    if ( !bfo )
    {
        dialogue_error_memory ();
        return;
    }

    bfo->columns = BET_FUTURE_FORM_WIDTH;
    bfo->rows = BET_FUTURE_FORM_HEIGHT;

    for ( i = 0 ; i<BET_FUTURE_FORM_HEIGHT ; i++ )
    for ( j = 0 ; j<BET_FUTURE_FORM_WIDTH ; j++ )
        bfo->form_table[i][j] = tab[i][j];
 }


/**
 *
 *
 *
 *
 * */
gboolean bet_future_new_line_dialog ( GtkTreeModel *tab_model,
                        gchar *str_date )
{
    GtkWidget *widget;
    gchar *tmp_str;
    GDate *date;
    GDate *date_jour;
    gint result;
    gint account_number;

    account_number = gsb_gui_navigation_get_current_account ( );
    if ( account_number == -1 )
        return FALSE;

    if ( bet_futur_dialog == NULL )
    {
        bet_futur_dialog = bet_future_create_dialog ( account_number );
    }
    else
    {
        bet_form_clean ( account_number );
        gtk_widget_show ( bet_futur_dialog );
    }

    /* init data */
    widget = bet_form_widget_get_widget ( TRANSACTION_FORM_DATE );
    date = gsb_parse_date_string ( str_date );
    date_jour = gdate_today ( );

    if ( g_date_valid ( date ) )
    {
        if ( g_date_compare ( date_jour, date ) >= 0 )
        {
            g_date_free ( date );
            g_date_add_days ( date_jour, 1 );
            date = date_jour;
        }
    }
    else
    {
        g_date_add_days ( date_jour, 1 );
        date = date_jour;
    }

    gsb_form_widget_set_empty ( widget, FALSE );
    gsb_calendar_entry_set_date ( widget, date );

    gtk_dialog_set_response_sensitive ( GTK_DIALOG ( bet_futur_dialog ),
                        GTK_RESPONSE_OK, FALSE );

dialog_return:
	result = gtk_dialog_run ( GTK_DIALOG ( bet_futur_dialog ));

    if ( result == GTK_RESPONSE_OK )
    {
        FuturData *scheduled;

        scheduled = struct_initialise_bet_future ( );

        if ( !scheduled )
        {
            dialogue_error_memory ();
            gtk_widget_hide ( bet_futur_dialog );
            return FALSE;
        }

        if ( bet_future_take_data_from_form ( scheduled ) == FALSE )
        {
            tmp_str = g_strdup ( _("Error: the frequency defined by the user or the amount is "
                                 "not specified or the date is invalid.") );
            dialogue_warning_hint ( tmp_str, _("One field is not filled in") );
            g_free ( tmp_str );
            goto dialog_return;
        }
        else
            bet_data_future_add_lines ( scheduled );

        gsb_data_account_set_bet_maj ( account_number, BET_MAJ_ESTIMATE );
        bet_data_update_bet_module ( account_number, GSB_ESTIMATE_PAGE );
    }

    gtk_widget_hide ( bet_futur_dialog );

    return FALSE;
}


/**
 *
 *
 *
 *
 * */
GtkWidget *bet_future_create_dialog ( gint account_number )
{
    GtkWidget *dialog;
    GtkWidget *vbox;
    GtkWidget *table;

    /* Create the dialog */
    dialog = gtk_dialog_new_with_buttons ( _("Enter a budget line"),
					   GTK_WINDOW ( grisbi_app_get_active_window (NULL) ),
					   GTK_DIALOG_MODAL,
					   "gtk-cancel", GTK_RESPONSE_CANCEL,
					   "gtk-ok", GTK_RESPONSE_OK,
					   NULL );

    gtk_window_set_position ( GTK_WINDOW ( dialog ), GTK_WIN_POS_CENTER_ON_PARENT );
    gtk_window_set_resizable ( GTK_WINDOW ( dialog ), TRUE );
    gtk_dialog_set_default_response ( GTK_DIALOG ( dialog ), GTK_RESPONSE_OK );

	vbox = gtk_box_new ( GTK_ORIENTATION_VERTICAL, 0 );
	gtk_box_pack_start ( GTK_BOX ( dialog_get_content_area ( dialog ) ), vbox, TRUE, TRUE, 0 );
	gtk_container_set_border_width ( GTK_CONTAINER ( vbox ), BOX_BORDER_WIDTH );

    /* next we fill the bet_form */
    table = gtk_grid_new ();
    gtk_grid_set_column_spacing (GTK_GRID (table), 6);
    gtk_widget_show ( table );
    gtk_box_pack_start ( GTK_BOX ( vbox ), table, FALSE, FALSE, 5 );

    bet_form_create_scheduler_part ( dialog, table );
    bet_form_create_current_form ( dialog, table, account_number );

	gtk_widget_show ( vbox );

    return dialog;
 }


/**
 * create the scheduled part
 *
 * \param table a GtkTable with the dimension SCHEDULED_HEIGHT*SCHEDULED_WIDTH to be filled
 *
 * \return FALSE
 * */
gboolean bet_form_create_scheduler_part ( GtkWidget *dialog, GtkWidget *table )
{

    GtkWidget *separator;
    GtkWidget *combo = NULL;
    gint column;

    devel_debug (NULL);
    if (!table)
        return FALSE;

    /* ok, now fill the form
     * we play with height and width, but for now it's fix : 6 columns and 1 line */
	for ( column=0 ; column < SCHEDULED_WIDTH ; column++ )
	{
        GtkWidget *widget = NULL;
	    const gchar *tooltip_text = NULL;
	    gchar *text_frequency [] = { _("Once"), _("Weekly"), _("Monthly"), _("Bimonthly"),
                        _("Quarterly"), _("Yearly"), _("Custom"), NULL };
	    gchar *text_frequency_user [] = { _("Days"), _("Weeks"), _("Months"), _("Years"), NULL };
        gint element_number;
        struct_element *element;

        /* on tient compte que le premier widget utile est le troisième du formulaire */
	    element_number = column + 2;

	    switch ( element_number )
	    {
		case SCHEDULED_FORM_FREQUENCY_BUTTON:
		    widget = gsb_combo_box_new_with_index ( text_frequency,
                        G_CALLBACK ( bet_form_scheduler_frequency_button_changed ),
                        dialog );
            g_object_set_data ( G_OBJECT ( widget ), "form_type", GINT_TO_POINTER ( 1 ) );
            combo = widget;
		    tooltip_text = _("Frequency");
            gtk_widget_show ( widget );
		    break;

        case SCHEDULED_FORM_LIMIT_DATE:
            widget = gsb_calendar_entry_new ( FALSE );
            g_object_set_data ( G_OBJECT ( widget ), "form_type", GINT_TO_POINTER ( 1 ) );
            gsb_form_widget_set_empty ( widget, TRUE );
            gtk_entry_set_text ( GTK_ENTRY ( widget ),
					    _("Limit date") );
            g_signal_connect ( G_OBJECT (widget),
                        "button-press-event",
                        G_CALLBACK ( gsb_form_scheduler_button_press_event ),
                        GINT_TO_POINTER (element_number));
            g_signal_connect ( G_OBJECT (widget),
                        "focus-in-event",
                        G_CALLBACK ( gsb_form_entry_get_focus ),
                        GINT_TO_POINTER (element_number));
            g_signal_connect_after ( G_OBJECT (widget),
                        "focus-out-event",
                        G_CALLBACK ( gsb_form_scheduler_entry_lose_focus ),
                        GINT_TO_POINTER (element_number));
            g_signal_connect ( G_OBJECT ( widget ),
                       "key-press-event",
                       G_CALLBACK ( bet_form_key_press_event ),
                       GINT_TO_POINTER ( element_number ) );
            tooltip_text = _("Final date");
            gtk_widget_show ( widget );
            break;

		case SCHEDULED_FORM_FREQUENCY_USER_ENTRY:
		    widget = gtk_entry_new ();
            g_object_set_data ( G_OBJECT ( widget ), "form_type", GINT_TO_POINTER ( 1 ) );
            gtk_entry_set_text ( GTK_ENTRY ( widget ),
					    _("Own frequency") );
            gsb_form_widget_set_empty ( widget, TRUE );
            g_signal_connect ( G_OBJECT (widget),
                        "focus-in-event",
                        G_CALLBACK (gsb_form_entry_get_focus),
                        GINT_TO_POINTER (element_number));
            g_signal_connect_after ( G_OBJECT (widget),
                        "focus-out-event",
                        G_CALLBACK ( gsb_form_scheduler_entry_lose_focus),
                        GINT_TO_POINTER (element_number));
            g_signal_connect ( G_OBJECT ( widget ),
                       "key-press-event",
                       G_CALLBACK ( bet_form_key_press_event ),
                       GINT_TO_POINTER ( element_number ) );
		    tooltip_text = _("Custom frequency");
		    break;

		case SCHEDULED_FORM_FREQUENCY_USER_BUTTON:
		    widget = gsb_combo_box_new_with_index ( text_frequency_user,
							    NULL, NULL );
            g_object_set_data ( G_OBJECT ( widget ), "form_type", GINT_TO_POINTER ( 1 ) );
            g_signal_connect ( G_OBJECT ( widget ),
                        "key-press-event",
                        G_CALLBACK ( bet_form_key_press_event ),
                        GINT_TO_POINTER ( element_number ) );
		    tooltip_text = _("Custom frequency");
            gsb_combo_box_set_index ( widget, 2 );
		    break;
	    }

	    if (!widget)
            continue;

	    if (tooltip_text)
            gtk_widget_set_tooltip_text ( GTK_WIDGET (widget),
                        tooltip_text);

        /* save the element */
	    element = g_malloc0 ( sizeof ( struct_element ) );
	    element -> element_number = element_number;
	    element -> element_widget = widget;
	    bet_schedul_element_list = g_slist_append ( bet_schedul_element_list, element );

	    /* set in the form */
        gtk_grid_attach (GTK_GRID (table), widget, column, 0, 1, 1);
	}

    separator = gtk_separator_new ( GTK_ORIENTATION_HORIZONTAL );
    gtk_widget_show ( separator );
    gtk_grid_attach (GTK_GRID (table), separator, 0, 1, 4, 1);

    gsb_combo_box_set_index ( combo, 0 );

    return FALSE;
}


/**
 * fill the form according to the account_number :
 *
 * \param account_number the number of account
 *
 * \return FALSE
 * */
gboolean bet_form_create_current_form ( GtkWidget *dialog,
                        GtkWidget *table,
                        gint account_number )
{
	GtkWidget *widget;
    gint element_number;
    gint row = 2;
    gint column = 0;
    struct_element *element;
    GSList *tmp_list;

    account_number = gsb_gui_navigation_get_current_account ( );

    element_number = TRANSACTION_FORM_DATE;
    widget = gsb_calendar_entry_new ( FALSE );
    gtk_widget_show ( widget );
    gtk_grid_attach (GTK_GRID (table), widget, column, row, 1, 1);
    element = g_malloc0 ( sizeof ( struct_element ) );
    element -> element_number = element_number;
    element -> element_widget = widget;
    bet_form_list_widgets = g_slist_append ( bet_form_list_widgets, element );
    column ++;

    element_number = TRANSACTION_FORM_PARTY;
	tmp_list = gsb_data_payee_get_name_and_report_list ();
	widget = gtk_combofix_new_with_properties (tmp_list,
											   TRUE,							/* on ne peut pas créer d'item */
											   etat.combofix_case_sensitive,
											   FALSE,
											   METATREE_PAYEE);
	gsb_data_payee_free_name_and_report_list (tmp_list);
    gtk_widget_show ( widget );
    gtk_grid_attach (GTK_GRID (table), widget, column, row, 1, 1);
    element = g_malloc0 ( sizeof ( struct_element ) );
    element -> element_number = element_number;
    element -> element_widget = widget;
    bet_form_list_widgets = g_slist_append ( bet_form_list_widgets, element );
    column ++;

    element_number = TRANSACTION_FORM_DEBIT;
    widget = gtk_entry_new ( );
    g_object_set_data ( G_OBJECT ( widget ), "element_number",
                        GINT_TO_POINTER ( TRANSACTION_FORM_DEBIT ) );
    g_signal_connect ( G_OBJECT ( widget ),
		                "changed",
		                G_CALLBACK ( gsb_form_widget_amount_entry_changed ),
		                NULL );
    gtk_widget_show ( widget );
    gtk_grid_attach (GTK_GRID (table), widget, column, row, 1, 1);
    element = g_malloc0 ( sizeof ( struct_element ) );
    element -> element_number = element_number;
    element -> element_widget = widget;
    bet_form_list_widgets = g_slist_append ( bet_form_list_widgets, element );
    column ++;

    element_number = TRANSACTION_FORM_CREDIT;
    widget = gtk_entry_new ( );
    g_object_set_data ( G_OBJECT ( widget ), "element_number",
                        GINT_TO_POINTER ( TRANSACTION_FORM_CREDIT ) );
    g_signal_connect ( G_OBJECT ( widget ),
		                "changed",
		                G_CALLBACK ( gsb_form_widget_amount_entry_changed ),
		                NULL );
    gtk_widget_show ( widget );
    gtk_grid_attach (GTK_GRID (table), widget, column, row, 1, 1);
    element = g_malloc0 ( sizeof ( struct_element ) );
    element -> element_number = element_number;
    element -> element_widget = widget;
    bet_form_list_widgets = g_slist_append ( bet_form_list_widgets, element );
    column = 0;
    row ++;

    element_number = TRANSACTION_FORM_EXERCICE;
    widget = gsb_fyear_make_combobox (TRUE);
    gtk_widget_set_tooltip_text ( GTK_WIDGET (widget),
					  _("Choose the financial year") );
    gtk_widget_show ( widget );
    gtk_grid_attach (GTK_GRID (table), widget, column, row, 1, 1);
    element = g_malloc0 ( sizeof ( struct_element ) );
    element -> element_number = element_number;
    element -> element_widget = widget;
    bet_form_list_widgets = g_slist_append ( bet_form_list_widgets, element );
    column ++;

    element_number = TRANSACTION_FORM_CATEGORY;
	tmp_list = gsb_data_category_get_name_list (TRUE, TRUE, TRUE, TRUE);
	widget = gtk_combofix_new_with_properties (tmp_list,
											   TRUE,							/* on ne peut pas créer d'item */
											   etat.combofix_case_sensitive,
											   etat.combofix_mixed_sort,
											   METATREE_CATEGORY);
	gsb_data_categorie_free_name_list (tmp_list);
    gtk_widget_show ( widget );
    gtk_grid_attach (GTK_GRID (table), widget, column, row, 1, 1);
    element = g_malloc0 ( sizeof ( struct_element ) );
    element -> element_number = element_number;
    element -> element_widget = widget;
    bet_form_list_widgets = g_slist_append ( bet_form_list_widgets, element );
    column ++;

    element_number = TRANSACTION_FORM_TYPE;
    widget = gtk_combo_box_new ();
    gsb_payment_method_create_combo_list ( widget,
                        GSB_PAYMENT_DEBIT,
                        account_number, 0, FALSE );
    gtk_combo_box_set_active ( GTK_COMBO_BOX (widget), 0 );
    gtk_widget_set_tooltip_text ( GTK_WIDGET (widget),
                        _("Choose the method of payment") );
    gtk_widget_show ( widget );
    gtk_grid_attach (GTK_GRID (table), widget, column, row, 1, 1);

    element = g_malloc0 ( sizeof ( struct_element ) );
    element -> element_number = element_number;
    element -> element_widget = widget;
    bet_form_list_widgets = g_slist_append ( bet_form_list_widgets, element );
    column = 1;
    row ++;

    element_number = TRANSACTION_FORM_BUDGET;
	tmp_list = gsb_data_budget_get_name_list (TRUE, TRUE);
	widget = gtk_combofix_new_with_properties (tmp_list,
											   TRUE,							/* on ne peut pas créer d'item */
											   etat.combofix_case_sensitive,
											   etat.combofix_mixed_sort,
											   METATREE_BUDGET);
	gsb_data_categorie_free_name_list (tmp_list);
    gtk_widget_show ( widget );
    gtk_grid_attach (GTK_GRID (table), widget, column, row, 1, 1);
    element = g_malloc0 ( sizeof ( struct_element ) );
    element -> element_number = element_number;
    element -> element_widget = widget;
    bet_form_list_widgets = g_slist_append ( bet_form_list_widgets, element );
    column = 0;
    row ++;

    element_number = TRANSACTION_FORM_NOTES;
	widget = gtk_entry_new();
    gtk_widget_show ( widget );
    gtk_grid_attach (GTK_GRID (table), widget, column, row, 4, 1);
    element = g_malloc0 ( sizeof ( struct_element ) );
    element -> element_number = element_number;
    element -> element_widget = widget;
    bet_form_list_widgets = g_slist_append ( bet_form_list_widgets, element );

    tmp_list = bet_form_list_widgets;

    while ( tmp_list )
    {
        struct_element *element_list;

        element_list = tmp_list -> data;

		widget = element_list->element_widget;
        if ( GTK_IS_ENTRY ( widget ))
        {
            g_signal_connect ( G_OBJECT ( widget ),
                       "focus-in-event",
                       G_CALLBACK ( bet_form_entry_get_focus ),
                       dialog );
            g_signal_connect ( G_OBJECT ( widget ),
                       "focus-out-event",
                       G_CALLBACK ( bet_form_entry_lose_focus ),
                       GINT_TO_POINTER ( element_list->element_number ) );
            g_signal_connect ( G_OBJECT ( widget ),
                       "button-press-event",
                       G_CALLBACK ( bet_form_button_press_event ),
                       GINT_TO_POINTER ( element_list->element_number ) );
            g_signal_connect ( G_OBJECT ( widget ),
                       "key-press-event",
                       G_CALLBACK ( bet_form_key_press_event ),
                       GINT_TO_POINTER ( element_list->element_number ) );
        }
        else
        {
            if ( GTK_IS_COMBOFIX ( widget ))
            {
				GtkWidget *entry;

				entry = gtk_combofix_get_entry (GTK_COMBOFIX (widget));
                g_signal_connect (G_OBJECT (entry),
                           "focus-in-event",
                           G_CALLBACK ( bet_form_entry_get_focus ),
                           dialog );
                g_signal_connect (G_OBJECT (entry),
                           "focus-out-event",
                           G_CALLBACK ( bet_form_entry_lose_focus ),
                           GINT_TO_POINTER ( element_list->element_number ) );
                g_signal_connect (G_OBJECT (entry),
                           "button-press-event",
                           G_CALLBACK ( bet_form_button_press_event ),
                           GINT_TO_POINTER ( element_list->element_number ) );
                g_signal_connect (G_OBJECT (entry),
                           "key-press-event",
                           G_CALLBACK ( bet_form_key_press_event ),
                           GINT_TO_POINTER ( element_list->element_number ) );
            }
            else
            /* neither an entry, neither a combofix */
            g_signal_connect ( G_OBJECT ( widget ),
                       "key-press-event",
                       G_CALLBACK ( bet_form_key_press_event ),
                       GINT_TO_POINTER ( element_list->element_number ) );

            g_object_set_data ( G_OBJECT ( widget ), "form_type", GINT_TO_POINTER ( 0 ) );
        }
        tmp_list = tmp_list -> next;
    }

    bet_form_clean ( account_number );

    return FALSE;
}


/**
 * clean the form according to the account_number
 * and set the default values
 *
 * \param account number
 *
 * \return FALSE
 * */
gboolean bet_form_clean ( gint account_number )
{
    GSList *tmp_list;

    /* clean the scheduled widget */
    tmp_list = bet_schedul_element_list;

    while (tmp_list)
    {
        struct_element *element;

        element = tmp_list -> data;

        /* better to protect here if widget != NULL (bad experience...) */
        if (element -> element_widget)
        {
            gtk_widget_set_sensitive ( element -> element_widget, TRUE );

            switch (element -> element_number)
            {
            case SCHEDULED_FORM_FREQUENCY_BUTTON:
                gsb_combo_box_set_index ( element -> element_widget, 0 );
                break;
            case SCHEDULED_FORM_LIMIT_DATE:
                gsb_form_widget_set_empty ( element -> element_widget, TRUE );
                gtk_entry_set_text ( GTK_ENTRY ( element -> element_widget ),
                        _("Limit date") );
                break;
            case SCHEDULED_FORM_FREQUENCY_USER_ENTRY:
                gtk_entry_set_text ( GTK_ENTRY ( element -> element_widget ),
                            _("Own frequency") );
                gsb_form_widget_set_empty ( element -> element_widget, TRUE );
                break;
    		case SCHEDULED_FORM_FREQUENCY_USER_BUTTON:
            gsb_combo_box_set_index ( element -> element_widget, 2 );
		    break;
            }
        }

        tmp_list = tmp_list -> next;
    }

    /* clean the transactions widget */
    tmp_list = bet_form_list_widgets;

    while (tmp_list)
    {
        struct_element *element;

        element = tmp_list -> data;

        /* better to protect here if widget != NULL (bad experience...) */
        if (element -> element_widget)
        {
            /* some widgets can be set unsensitive because of the children of splits,
             * so resensitive all to be sure */
            gtk_widget_set_sensitive ( element -> element_widget, TRUE );

            switch (element -> element_number)
            {
            case TRANSACTION_FORM_DATE:
                if ( !strlen ( gtk_entry_get_text ( GTK_ENTRY (
                 element -> element_widget ) ) ) )
                {
                    gsb_form_widget_set_empty ( element -> element_widget, TRUE );
                    gtk_entry_set_text ( GTK_ENTRY ( element -> element_widget ), _("Date") );
                }
                break;

            case TRANSACTION_FORM_EXERCICE:
                /* editing a transaction can show some fyear which shouldn't be showed,
                 * so hide them here */
                gsb_fyear_update_fyear_list ();

                /* set the combo_box on 'Automatic' */
                gsb_fyear_set_combobox_history ( element -> element_widget, 0 );

                break;

            case TRANSACTION_FORM_PARTY:
                gsb_form_widget_set_empty (element -> element_widget, TRUE);
                gtk_combofix_set_text ( GTK_COMBOFIX (element -> element_widget), ("Payee"));
                break;

            case TRANSACTION_FORM_DEBIT:
                gsb_form_widget_set_empty ( element -> element_widget,
                            TRUE );
                gtk_entry_set_text ( GTK_ENTRY ( element -> element_widget ),
                         _("Debit") );
                break;

            case TRANSACTION_FORM_CREDIT:
                gsb_form_widget_set_empty ( element -> element_widget,
                            TRUE );
                gtk_entry_set_text ( GTK_ENTRY ( element -> element_widget ),
                         _("Credit") );
                break;

            case TRANSACTION_FORM_CATEGORY:
               gsb_form_widget_set_empty (element -> element_widget, TRUE);
                gtk_combofix_set_text ( GTK_COMBOFIX ( element -> element_widget ),
                            _("Categories : Sub-categories") );
                break;

            case TRANSACTION_FORM_BUDGET:
					gsb_form_widget_set_empty (element -> element_widget, TRUE);
					gtk_combofix_set_text ( GTK_COMBOFIX ( element -> element_widget ),
                            _("Budgetary line") );
                break;

            case TRANSACTION_FORM_NOTES:
                gsb_form_widget_set_empty ( element -> element_widget,
                            TRUE );
                gtk_entry_set_text ( GTK_ENTRY ( element -> element_widget ),
                         _("Notes") );
                break;

            case TRANSACTION_FORM_TYPE:
                gsb_payment_method_set_combobox_history ( element -> element_widget,
                                      gsb_data_account_get_default_debit ( account_number ), TRUE );
                break;

            }
        }
        tmp_list = tmp_list -> next;
    }

    return FALSE;
}


/**
 * called when the frequency button is changed
 * show/hide the necessary widget according to its state
 *
 * \param combo_box
 *
 * \return FALSE
 * */
gboolean bet_form_scheduler_frequency_button_changed ( GtkWidget *combo_box,
                        GtkWidget *dialog )
{
    gchar *selected_item;

	selected_item = gsb_combo_box_get_active_text (GTK_COMBO_BOX (combo_box));

    if ( !strcmp ( selected_item, _("Once") ) )
    {
        gtk_widget_hide ( bet_form_scheduler_get_element_widget (
                        SCHEDULED_FORM_LIMIT_DATE ) );
        gtk_widget_hide ( bet_form_scheduler_get_element_widget (
                        SCHEDULED_FORM_FREQUENCY_USER_ENTRY ) );
        gtk_widget_hide ( bet_form_scheduler_get_element_widget (
                        SCHEDULED_FORM_FREQUENCY_USER_BUTTON ) );
    }
    else
    {
        gtk_widget_show ( bet_form_scheduler_get_element_widget (
                        SCHEDULED_FORM_LIMIT_DATE ) );

        if ( !strcmp ( selected_item, _("Custom") ) )
        {
            gtk_widget_show ( bet_form_scheduler_get_element_widget (
                        SCHEDULED_FORM_FREQUENCY_USER_ENTRY));
            gtk_widget_show ( bet_form_scheduler_get_element_widget (
                        SCHEDULED_FORM_FREQUENCY_USER_BUTTON ) );
        }
        else
        {
            gtk_widget_hide ( gsb_form_scheduler_get_element_widget (
                        SCHEDULED_FORM_FREQUENCY_USER_ENTRY ) );
            gtk_widget_hide ( gsb_form_scheduler_get_element_widget (
                        SCHEDULED_FORM_FREQUENCY_USER_BUTTON ) );
        }
    }
    g_free ( selected_item );
    gtk_dialog_set_response_sensitive ( GTK_DIALOG ( dialog ), GTK_RESPONSE_OK, TRUE );

    return FALSE;
}


/**
 * called when an entry get the focus, if the entry is free,
 * set it normal and erase the help content
 *
 * \param entry
 *
 * \return FALSE
 * */
gboolean bet_form_entry_get_focus ( GtkWidget *entry,
                        GdkEventFocus *ev,
                        GtkWidget *dialog )
{
    /* the entry can be a combofix or a real entry */
    if (GTK_IS_COMBOFIX ( entry ))
    {
        if (gsb_form_widget_check_empty (entry))
        {
            gtk_combofix_set_text ( GTK_COMBOFIX (entry), "" );
            gsb_form_widget_set_empty (entry, FALSE);
        }
    }
    else
    {
        if ( gsb_form_widget_check_empty (entry) )
        {
            gtk_entry_set_text ( GTK_ENTRY (entry), "" );
            gsb_form_widget_set_empty ( entry, FALSE );
        }
    }
    /* sensitive the valid button */
    if ( dialog && GTK_IS_DIALOG ( dialog ) )
        gtk_dialog_set_response_sensitive ( GTK_DIALOG ( dialog ), GTK_RESPONSE_OK, TRUE );

    return FALSE;
}


/**
 * called when an entry lose the focus
 *
 * \param entry
 * \param ev
 * \param ptr_origin a pointer gint which is the number of the element
 *
 * \return FALSE
 * */
gboolean bet_form_entry_lose_focus ( GtkWidget *entry,
                        GdkEventFocus *ev,
                        gint *ptr_origin )
{
    GtkWidget *widget;
    gchar *string;
    gint element_number;
    gint account_number;

    /* still not found, if change the content of the form, something come in entry
     * which is nothing, so protect here */
    if ( !GTK_IS_WIDGET ( entry )
     ||
     !GTK_IS_ENTRY ( entry ))
        return FALSE;

    /* remove the selection */
    gtk_editable_select_region ( GTK_EDITABLE ( entry ), 0, 0 );
    element_number = GPOINTER_TO_INT ( ptr_origin );
    account_number = gsb_form_get_account_number ();

    /* sometimes the combofix popus stays showed, so remove here */
    if ( element_number == TRANSACTION_FORM_PARTY
     ||
     element_number == TRANSACTION_FORM_CATEGORY
     ||
     element_number == TRANSACTION_FORM_BUDGET )
    {
        widget = bet_form_widget_get_widget ( element_number );
        gtk_combofix_hide_popup ( GTK_COMBOFIX ( widget ) );
    }

    /* string will be filled only if the field is empty */
    string = NULL;
    switch ( element_number )
    {
    case TRANSACTION_FORM_DATE :
        if ( !strlen ( gtk_entry_get_text ( GTK_ENTRY ( entry ) ) ) )
            string = gsb_form_widget_get_name ( TRANSACTION_FORM_DATE );
        break;

    case TRANSACTION_FORM_PARTY :
        if ( !strlen ( gtk_entry_get_text ( GTK_ENTRY ( entry ) ) ) )
            string = gsb_form_widget_get_name ( TRANSACTION_FORM_PARTY );
        break;

    case TRANSACTION_FORM_DEBIT :
        /* we change the payment method to adapt it for the debit */
        if ( strlen ( gtk_entry_get_text ( GTK_ENTRY ( entry ) ) ) )
        {
        /* empty the credit */
        widget = bet_form_widget_get_widget ( TRANSACTION_FORM_CREDIT );
        if ( !gsb_form_widget_check_empty ( widget ) )
        {
            gtk_entry_set_text ( GTK_ENTRY ( widget ),
                        gsb_form_widget_get_name ( TRANSACTION_FORM_CREDIT ) );
            gsb_form_widget_set_empty ( widget, TRUE );
        }

        widget = bet_form_widget_get_widget ( TRANSACTION_FORM_TYPE );

        /* change the method of payment if necessary
         * (if grey, it's a child of split so do nothing) */
        if ( widget
            &&
            gtk_widget_get_sensitive ( widget ) )
        {
            /* change the signe of the method of payment and the contra */
            if ( gsb_payment_method_get_combo_sign ( widget ) == GSB_PAYMENT_CREDIT )
            {
                gsb_payment_method_create_combo_list ( widget,
                                            GSB_PAYMENT_DEBIT,
                                            account_number, 0, FALSE );
            }
        }
        gsb_form_check_auto_separator (entry);
        }
        else
        {
            /* si pas de nouveau débit on essaie de remettre l'ancien crédit */
            if ( (string = gsb_form_widget_get_old_credit ( ) ) )
            {
                GtkWidget *widget_prov;

                widget_prov = bet_form_widget_get_widget ( TRANSACTION_FORM_CREDIT );

                gtk_entry_set_text ( GTK_ENTRY ( widget_prov ), string );
                gsb_form_widget_set_empty ( widget_prov, FALSE );
                g_free ( string );

                widget = bet_form_widget_get_widget ( TRANSACTION_FORM_TYPE );
                if ( widget
                     &&
                     gtk_widget_get_sensitive ( widget ) )
                {
                    /* change the signe of the method of payment and the contra */
                    if ( gsb_payment_method_get_combo_sign ( widget ) == GSB_PAYMENT_DEBIT )
                    {
                        gsb_payment_method_create_combo_list ( widget,
                                            GSB_PAYMENT_CREDIT,
                                            account_number, 0, FALSE );
                    }
                }
            }
            string = gsb_form_widget_get_name ( TRANSACTION_FORM_DEBIT );
        }
        break;

    case TRANSACTION_FORM_CREDIT :
        /* we change the payment method to adapt it for the debit */
        if ( strlen ( gtk_entry_get_text ( GTK_ENTRY ( entry ) ) ) )
        {
        /* empty the credit */
        widget = bet_form_widget_get_widget ( TRANSACTION_FORM_DEBIT );
        if ( !gsb_form_widget_check_empty ( widget ) )
        {
            gtk_entry_set_text ( GTK_ENTRY ( widget ),
                        gsb_form_widget_get_name ( TRANSACTION_FORM_DEBIT ) );
            gsb_form_widget_set_empty ( widget, TRUE );
        }
        widget = bet_form_widget_get_widget ( TRANSACTION_FORM_TYPE);

        /* change the method of payment if necessary
         * (if grey, it's a child of split so do nothing) */
        if ( widget
             &&
             gtk_widget_get_sensitive (widget))
        {
            /* change the signe of the method of payment and the contra */
            if ( gsb_payment_method_get_combo_sign ( widget ) == GSB_PAYMENT_DEBIT )
            {
                gsb_payment_method_create_combo_list ( widget,
                        GSB_PAYMENT_CREDIT,
                        account_number, 0, FALSE );
            }
        }
        gsb_form_check_auto_separator (entry);
        }
        else
        {
            /* si pas de nouveau credit on essaie de remettre l'ancien débit */
            if ( (string = gsb_form_widget_get_old_debit ( ) ) )
            {
                GtkWidget * widget_prov;

                widget_prov = bet_form_widget_get_widget ( TRANSACTION_FORM_DEBIT );

                gtk_entry_set_text ( GTK_ENTRY ( widget_prov ), string );
                gsb_form_widget_set_empty ( widget_prov, FALSE );
                g_free ( string );

                widget = bet_form_widget_get_widget ( TRANSACTION_FORM_TYPE );
                if ( widget
                     &&
                     gtk_widget_get_sensitive ( widget ) )
                {
                    /* change the signe of the method of payment and the contra */
                    if ( gsb_payment_method_get_combo_sign ( widget ) == GSB_PAYMENT_CREDIT )
                    {
                        gsb_payment_method_create_combo_list ( widget,
                                            GSB_PAYMENT_DEBIT,
                                            account_number, 0, FALSE );
                    }
                }
            }
            string = gsb_form_widget_get_name ( TRANSACTION_FORM_CREDIT );
        }
        break;

    case TRANSACTION_FORM_CATEGORY :
        if ( !strlen ( gtk_entry_get_text ( GTK_ENTRY ( entry ) ) ) )
            string = gsb_form_widget_get_name ( TRANSACTION_FORM_CATEGORY );
        break;

    case TRANSACTION_FORM_BUDGET :
        if ( !strlen ( gtk_entry_get_text ( GTK_ENTRY ( entry ) ) ) )
            string = gsb_form_widget_get_name ( TRANSACTION_FORM_BUDGET );
        break;

    case TRANSACTION_FORM_NOTES :
        if ( !strlen ( gtk_entry_get_text ( GTK_ENTRY ( entry ) ) ) )
            string = _(gsb_form_widget_get_name ( element_number ) );
        break;

    default :
        break;

    }

    /* if string is not NULL, the entry is empty so set the empty field to TRUE */
    if ( string )
    {
        switch ( element_number)
        {
            case TRANSACTION_FORM_PARTY :
            case TRANSACTION_FORM_CATEGORY :
            case TRANSACTION_FORM_BUDGET :
            /* need to work with the combofix to avoid some signals if we work
             * directly on the entry */
            gtk_combofix_set_text ( GTK_COMBOFIX (
                        bet_form_widget_get_widget ( element_number ) ),
                        _(string) );
            break;

            default:
                gtk_entry_set_text ( GTK_ENTRY ( entry ), string );
            break;
        }
        gsb_form_widget_set_empty ( entry, TRUE );
    }

    return FALSE;
}


/**
 * return the widget of the element_number given in param,
 * for the bet scheduler part of the form
 *
 * \param element_number
 *
 * \return a GtkWidget * or NULL
 * */
GtkWidget *bet_form_scheduler_get_element_widget ( gint element_number )
{
    return gsb_form_get_element_widget_from_list ( element_number,
                        bet_schedul_element_list );
}


/**
 * return the pointer to the widget corresponding to the given element
 *
 * \param element_number
 *
 * \return the widget or NULL
 * */
GtkWidget *bet_form_widget_get_widget ( gint element_number )
{
    return gsb_form_get_element_widget_from_list ( element_number,
                        bet_form_list_widgets );
}


/**
 * check if the given element can receive focus
 *
 * \param element_number
 *
 * \return TRUE : can receive focus, or FALSE
 * */
static gboolean bet_form_widget_can_focus ( gint element_number,
                        gint form_type )
{
    GtkWidget *widget;

    /* if element_number is -1 or -2, the iteration while must
     * stop, so return TRUE */
    if ( element_number == -1 || element_number == -2 )
        return TRUE;

    if ( form_type )
        widget = bet_form_scheduler_get_element_widget ( element_number );
    else
        widget = bet_form_widget_get_widget ( element_number );

    if ( !widget )
        return FALSE;

    if ( !gtk_widget_get_visible ( widget ) )
        return FALSE;

    if ( !gtk_widget_get_sensitive ( widget ) )
        return FALSE;

    if ( !( GTK_IS_COMBOFIX ( widget )
       ||
       GTK_IS_ENTRY ( widget )
       ||
       GTK_IS_BUTTON ( widget )
       ||
       GTK_IS_COMBO_BOX ( widget ) ) )
        return FALSE;

    return TRUE;
}


static GtkWidget *bet_form_widget_get_next_element ( gint element_number,
                        gint form_type )
{
    gint row = 0;
    gint column = 0;
    gint return_value_number = 0;
    gint form_column_number = BET_FUTURE_FORM_WIDTH;
    gint form_row_number = 5;

    if ( form_type ) /* le widget initial est sur la partie scheduled du formulaire */
    {
        for ( column=0 ; column < bfo->columns ; column++ )
        {
            if ( bfo->form_table[row][column] == element_number )
            {
                form_row_number = 1;
                while ( !bet_form_widget_can_focus ( return_value_number, form_type ) )
                {
                    if ( column == ( form_column_number - 1)
                         &&
                         row == ( form_row_number - 1 ) )
                    {
                        /* we are on the bottom right, we finish the edition or
                         * go to the upper left */
                        if ( !conf.entree )
                        {
                            return_value_number = -2;
                            continue;
                        }
                        column = -1;
                        row = 0;
                    }

                    if ( ++column == form_column_number )
                    {
                        column = 0;
                        row++;
                    }
                    return_value_number = bfo->form_table[row][column];
                }
                break;
            }
        }
    }
    else
    {
        gboolean trouve = FALSE;
        for ( row=1 ; row < bfo->rows; row++ )
        {
            for ( column=0 ; column < bfo->columns ; column++ )
            {
                if ( bfo->form_table[row][column] == element_number )
                {
                    trouve = TRUE;
                    break;
                }
            }
            if ( trouve )
                break;
        }
        while ( !bet_form_widget_can_focus ( return_value_number, form_type ) )
        {
            if ( column == ( form_column_number - 1)
                 &&
                 row == ( form_row_number - 1 ) )
            {
                /* we are on the bottom right, we finish the edition or
                 * go to the upper left */
                if ( !conf.entree )
                {
                    return_value_number = -2;
                    continue;
                }
                column = -1;
                row = 0;
            }

            if ( ++column == form_column_number )
            {
                column = 0;
                row++;
            }
            return_value_number = bfo->form_table[row][column];
        }
    }

    /* return value */
    if ( row == 0 )
        return gsb_form_get_element_widget_from_list ( return_value_number,
                        bet_schedul_element_list );
    else
        return gsb_form_get_element_widget_from_list ( return_value_number,
                        bet_form_list_widgets );
}


/**
 * called when press a key on an element of the form
 *
 * \param widget which receive the signal
 * \param ev
 * \param ptr_origin a pointer number of the element
 *
 * \return FALSE
 * */
gboolean bet_form_key_press_event ( GtkWidget *widget,
                        GdkEventKey *ev,
                        gint *ptr_origin )
{
    GtkWidget *widget_suivant;
    GtkWidget *widget_prov;
    gint element_number;
    gint form_type = 0;

    form_type = GPOINTER_TO_INT ( g_object_get_data ( G_OBJECT ( widget ), "form_type" ) );

    /* on sort si on est sur la partie scheduler du formulaire sauf pour GDK_Tab */
    if ( form_type && ev -> keyval != GDK_KEY_Tab )
        return FALSE;

    element_number = GPOINTER_TO_INT (ptr_origin);

    /* if conf.entree = 1, entry finish the transaction, else does as tab */
    if ( !conf.entree
	 &&
	 ( ev -> keyval == GDK_KEY_Return
	   ||
	   ev -> keyval == GDK_KEY_KP_Enter ))
	ev->keyval = GDK_KEY_Tab ;

    switch ( ev -> keyval )
    {
    case GDK_KEY_1:
    case GDK_KEY_2:
    case GDK_KEY_3:
    case GDK_KEY_4:
    case GDK_KEY_5:
    case GDK_KEY_6:
    case GDK_KEY_7:
    case GDK_KEY_8:
    case GDK_KEY_9:
    case GDK_KEY_0:
        switch ( element_number )
        {
        case TRANSACTION_FORM_DEBIT:
            widget_prov = bet_form_widget_get_widget ( TRANSACTION_FORM_CREDIT );
            if ( !gsb_form_widget_check_empty ( widget_prov ) )
            {
                gtk_entry_set_text ( GTK_ENTRY ( widget_prov ),
                         gsb_form_widget_get_name ( TRANSACTION_FORM_CREDIT ) );
                gsb_form_widget_set_empty ( widget_prov, TRUE );
            }
            break;
        case TRANSACTION_FORM_CREDIT:
            widget_prov = bet_form_widget_get_widget ( TRANSACTION_FORM_DEBIT );
            if ( !gsb_form_widget_check_empty ( widget_prov ) )
            {
                gtk_entry_set_text ( GTK_ENTRY (widget_prov),
                            gsb_form_widget_get_name (TRANSACTION_FORM_DEBIT));
                gsb_form_widget_set_empty ( widget_prov, TRUE );
            }
            break;
        }
        break;
	case GDK_KEY_Escape :
	    gsb_form_escape_form ();
	    break;

	case GDK_KEY_KP_Enter :
	case GDK_KEY_Return :

	    break;

	case GDK_KEY_KP_Add:
	case GDK_KEY_plus:
	case GDK_KEY_equal:		/* This should make all our US users happy */

	    /* increase the check of 1 */
	    if (element_number == TRANSACTION_FORM_CHEQUE)
	    {
		increment_decrement_champ ( widget,
					    1 );
		return TRUE;
	    }
	    break;

	case GDK_KEY_KP_Subtract:
	case GDK_KEY_minus:

	    /* decrease the check of 1 */
	    if (element_number == TRANSACTION_FORM_CHEQUE)
	    {
		increment_decrement_champ ( widget,
					    -1 );
		return TRUE;
	    }
	    break;

    case GDK_KEY_Tab :
        widget_suivant = bet_form_widget_get_next_element ( element_number, form_type );

        if ( widget_suivant )
        {
            if ( GTK_IS_COMBOFIX ( widget_suivant ) )
                gtk_widget_grab_focus (gtk_combofix_get_entry (GTK_COMBOFIX (widget_suivant)));
            else
                gtk_widget_grab_focus ( widget_suivant );

            return TRUE;
        }
        break;
    }

    return FALSE;
}


/**
 * called when we press the button in an entry field in
 * the form
 *
 * \param entry which receive the signal
 * \param ev can be NULL
 * \param ptr_origin a pointer to int on the element_number
 *
 * \return FALSE
 * */
gboolean bet_form_button_press_event ( GtkWidget *entry,
                        GdkEventButton *ev,
                        gint *ptr_origin )
{
    GtkWidget *date_entry;

	/* set the current date into the date entry */
	date_entry = bet_form_widget_get_widget (TRANSACTION_FORM_DATE);
	if ( gsb_form_widget_check_empty ( date_entry ) )
	{
        gtk_entry_set_text ( GTK_ENTRY ( date_entry ), gsb_date_today ( ) );
	    gsb_form_widget_set_empty ( date_entry, FALSE );
	}

    return FALSE;
}


/**
 * récupère le tiers
 *
 *
 * \return FALSE
 * */
static gboolean bet_future_get_payee_data ( GtkWidget *widget,
                        gint struct_type,
                        gpointer *value )
{
    const gchar *string;
    TransfertData *sd = (TransfertData *) value;

    string = gtk_combofix_get_text ( GTK_COMBOFIX ( widget ) );
    if ( string && strlen ( string ) > 0 )
    {
        if ( struct_type == 1 )
            sd->main_payee_number = gsb_data_payee_get_number_by_name ( string, FALSE );
        else
            sd->card_payee_number = gsb_data_payee_get_number_by_name ( string, FALSE );
    }
    else
    {
        if ( struct_type == 1 )
            sd->main_payee_number = 0;
        else
            sd->card_payee_number = 0;
    }

    return FALSE;
}


/**
 * récupère le moyen de payement
 *
 *
 * \return FALSE
 * */
static gboolean bet_future_get_payment_data ( GtkWidget *widget,
                        gpointer *value )
{
    gint payment_number;
    TransfertData *sd = (TransfertData *) value;

    payment_number = gsb_payment_method_get_selected_number ( widget );
    if ( payment_number > 0 )
    {
        sd->main_payment_number = payment_number;
    }
    else
    {
        sd->main_payee_number = gsb_data_account_get_default_debit ( sd->account_number );
    }

    return FALSE;
}


/**
 * récupère la catégorie et la sous catégorie
 *
 *
 * \return FALSE
 * */
static gboolean bet_future_get_category_data ( GtkWidget *widget,
                        gint struct_type,
                        gpointer *value )
{
    const gchar *string;
    gchar **tab_char;
    gint category_number = 0;
    gint sub_category_number = 0;

    string = gtk_combofix_get_text ( GTK_COMBOFIX ( widget ) );

    if ( struct_type == 0 )
    {
        FuturData *sd = ( FuturData *) value;

        if ( string && strlen ( string ) > 0 )
        {
            tab_char = g_strsplit ( string, " : ", 2 );
            if ( my_strcasecmp ( tab_char[0], _("Transfer") ) == 0 )
            {
                sd -> is_transfert = TRUE;
                sd -> account_transfert =  gsb_data_account_get_no_account_by_name ( tab_char[1] );
            }
            else
            {
                category_number = gsb_data_category_get_number_by_name (
                                tab_char[0], FALSE, 0 );

                if ( tab_char[1] && strlen ( tab_char[1] ) )
                    sub_category_number = gsb_data_category_get_sub_category_number_by_name (
                                category_number, tab_char[1], FALSE );
                else
                    sub_category_number = 0;
            }

            g_strfreev ( tab_char );
        }
        else
        {
            category_number = 0;
            sub_category_number = 0;
        }

        sd -> category_number = category_number;
        sd -> sub_category_number = sub_category_number;
    }
    else if ( struct_type == 1 )
    {
        TransfertData *sd = ( TransfertData *) value;

        if ( string && strlen ( string ) > 0 )
        {
            tab_char = g_strsplit ( string, " : ", 2 );
            category_number = gsb_data_category_get_number_by_name (
                            tab_char[0], FALSE, 0 );

            if ( tab_char[1] && strlen ( tab_char[1] ) )
                sub_category_number = gsb_data_category_get_sub_category_number_by_name (
                            category_number, tab_char[1], FALSE );
            else
                sub_category_number = 0;

            g_strfreev ( tab_char );
        }
        else
        {
            category_number = 0;
            sub_category_number = 0;
        }

        sd->main_category_number = category_number;
        sd->main_sub_category_number = sub_category_number;
    }
    else if ( struct_type == 2 )
    {
        /* on est toujours avec une struture transfert mais on récupère les éléments pour le compte carte */
        TransfertData *sd = ( TransfertData *) value;

        if ( string && strlen ( string ) > 0 )
        {
            tab_char = g_strsplit ( string, " : ", 2 );
            category_number = gsb_data_category_get_number_by_name (
                            tab_char[0], FALSE, 0 );

            if ( tab_char[1] && strlen ( tab_char[1] ) )
                sub_category_number = gsb_data_category_get_sub_category_number_by_name (
                            category_number, tab_char[1], FALSE );
            else
                sub_category_number = 0;

            g_strfreev ( tab_char );
        }
        else
        {
            category_number = 0;
            sub_category_number = 0;
        }

        sd->card_category_number = category_number;
        sd->card_sub_category_number = sub_category_number;
    }

    return FALSE;
}


/**
 * récupère l'imputation et la sous imputation budgétaire
 *
 *
 * \return FALSE
 * */
static gboolean bet_future_get_budget_data ( GtkWidget *widget,
                        gint struct_type,
                        gpointer *value )
{
    const gchar *string;
    gchar **tab_char;
    gint budgetary_number;
    gint sub_budgetary_number;

    string = gtk_combofix_get_text ( GTK_COMBOFIX ( widget ) );
    if ( string && strlen ( string ) > 0 )
    {
        tab_char = g_strsplit ( string, " : ", 2 );
        budgetary_number = gsb_data_budget_get_number_by_name (
                        tab_char[0], FALSE, 0 );

        if ( tab_char[1] && strlen ( tab_char[1] ) )
            sub_budgetary_number = gsb_data_budget_get_sub_budget_number_by_name (
                        budgetary_number, tab_char[1], FALSE );
        else
            sub_budgetary_number = 0;

        g_strfreev ( tab_char );
    }
    else
    {
        budgetary_number = 0;
        sub_budgetary_number = 0;
    }

    if ( struct_type == 0 )
    {
        FuturData *sd = ( FuturData *) value;

        sd -> budgetary_number = budgetary_number;
        sd -> sub_budgetary_number = sub_budgetary_number;
    }
    else if ( struct_type == 1 )
    {
        TransfertData *sd = ( TransfertData *) value;

        sd->main_budgetary_number = budgetary_number;
        sd->main_sub_budgetary_number = sub_budgetary_number;
    }

    return FALSE;
}


/**
 * initialise les données du formulaire
 *
 *
 *
 * */
gboolean bet_future_set_form_data_from_line ( gint account_number,
                        gint number  )
{
    GtkWidget *widget;
    GHashTable *future_list;
    gchar *key;
    const gchar *tmp_str;
    FuturData *scheduled;

    if ( account_number == 0 )
        key = g_strconcat ("0:", utils_str_itoa ( number ), NULL );
    else
        key = g_strconcat ( utils_str_itoa ( account_number ), ":",
                        utils_str_itoa ( number ), NULL );

    future_list = bet_data_future_get_list ( );

    scheduled = g_hash_table_lookup ( future_list, key );
    if ( scheduled == NULL )
        return FALSE;

    /* On traite les données de la planification */
    widget = bet_form_scheduler_get_element_widget ( SCHEDULED_FORM_FREQUENCY_BUTTON );
    gsb_combo_box_set_index ( widget, scheduled -> frequency );
    gtk_widget_set_sensitive ( widget, FALSE );

    if ( scheduled -> frequency > 0 )
    {
        if ( scheduled -> limit_date && g_date_valid ( scheduled -> limit_date ) )
        {
            widget = bet_form_scheduler_get_element_widget ( SCHEDULED_FORM_LIMIT_DATE );
            gsb_form_widget_set_empty ( widget, FALSE );
            gsb_calendar_entry_set_date ( widget, scheduled -> limit_date );
            gtk_widget_set_sensitive ( widget, FALSE );
        }

        if ( scheduled -> user_entry > 0 )
        {
            widget = bet_form_scheduler_get_element_widget (
                        SCHEDULED_FORM_FREQUENCY_USER_ENTRY );
            gsb_form_widget_set_empty ( widget, FALSE );
            gtk_entry_set_text ( GTK_ENTRY ( widget ),
                        utils_str_itoa ( scheduled -> user_entry ) );
            gtk_widget_set_sensitive ( widget, FALSE );

            widget = bet_form_scheduler_get_element_widget (
                    SCHEDULED_FORM_FREQUENCY_USER_BUTTON );
            gsb_combo_box_set_index ( widget, scheduled -> user_interval );
            gtk_widget_set_sensitive ( widget, FALSE );
        }
    }

    /* On traite les données de transaction */
    widget = bet_form_widget_get_widget ( TRANSACTION_FORM_DATE );
    gsb_calendar_entry_set_date ( widget, scheduled -> date );
    gsb_form_widget_set_empty ( widget, FALSE );

    if ( scheduled -> fyear_number > 0 )
    {
        widget = bet_form_widget_get_widget ( TRANSACTION_FORM_EXERCICE );
        gsb_fyear_set_combobox_history ( widget, scheduled -> fyear_number );
    }

    widget = bet_form_widget_get_widget ( TRANSACTION_FORM_PARTY );
    tmp_str = gsb_data_payee_get_name ( scheduled -> party_number, TRUE );
    if ( tmp_str && strlen ( tmp_str ) )
    {
        gsb_form_widget_set_empty (widget, FALSE);
        gtk_combofix_set_text ( GTK_COMBOFIX ( widget ), tmp_str );
        gtk_editable_set_position (GTK_EDITABLE (gtk_combofix_get_entry (GTK_COMBOFIX (widget))), 0);
    }

    if ( scheduled -> amount.mantissa < 0 )
    {
        widget = bet_form_widget_get_widget ( TRANSACTION_FORM_DEBIT );
        gtk_entry_set_text ( GTK_ENTRY ( widget ), utils_real_get_string (
                        gsb_real_opposite ( scheduled -> amount ) ) );
    }
    else
    {
        widget = bet_form_widget_get_widget ( TRANSACTION_FORM_CREDIT );
        gtk_entry_set_text ( GTK_ENTRY ( widget ), utils_real_get_string ( scheduled -> amount ) );
    }
    gsb_form_widget_set_empty ( widget, FALSE );

    widget = bet_form_widget_get_widget ( TRANSACTION_FORM_TYPE );
    gsb_payment_method_set_combobox_history ( widget, scheduled -> payment_number, TRUE );

    if ( scheduled -> category_number > 0 )
    {
        widget = bet_form_widget_get_widget ( TRANSACTION_FORM_CATEGORY );
        gsb_form_widget_set_empty (widget, FALSE);
        gtk_combofix_set_text ( GTK_COMBOFIX ( widget ),
                        gsb_data_category_get_name ( scheduled -> category_number,
                        scheduled -> sub_category_number, NULL) );
    }
    else if ( scheduled -> is_transfert )
    {
        gchar *tmp_str_2;

        widget = bet_form_widget_get_widget ( TRANSACTION_FORM_CATEGORY );
        gsb_form_widget_set_empty (widget, FALSE);
        tmp_str_2 = g_strconcat ( _("Transfer"), " : ",
                        gsb_data_account_get_name ( scheduled -> account_transfert ),
                        NULL );

        gtk_combofix_set_text ( GTK_COMBOFIX ( widget ), tmp_str_2 );
        g_free ( tmp_str_2 );
    }

    if ( scheduled -> budgetary_number > 0 )
    {
        widget = bet_form_widget_get_widget ( TRANSACTION_FORM_BUDGET );
        gsb_form_widget_set_empty (widget, FALSE);
        gtk_combofix_set_text ( GTK_COMBOFIX ( widget ),
                        gsb_data_budget_get_name (  scheduled -> budgetary_number,
                        scheduled -> sub_budgetary_number, NULL ) );
    }

    if ( scheduled -> notes && strlen ( scheduled -> notes ) > 0 )
    {
        widget = bet_form_widget_get_widget ( TRANSACTION_FORM_NOTES );
        gsb_form_widget_set_empty ( widget, FALSE );
        gtk_entry_set_text ( GTK_ENTRY ( widget ), scheduled -> notes );
    }

    return TRUE;
}


/**
 * récupère les données du formulaire
 *
 *
 *
 * */
gboolean bet_future_take_data_from_form (  FuturData *scheduled )
{
    GtkWidget *widget;

    /* données liées au compte */
    scheduled -> account_number = gsb_gui_navigation_get_current_account ( );

    /* On traite les données de la planification */
    widget = bet_form_scheduler_get_element_widget ( SCHEDULED_FORM_FREQUENCY_BUTTON );
    scheduled -> frequency = gsb_combo_box_get_index ( widget );

    switch ( scheduled -> frequency )
    {
        case 1:
        case 2:
        case 3:
        case 4:
        case 5:
            widget = bet_form_scheduler_get_element_widget ( SCHEDULED_FORM_LIMIT_DATE );
            if ( gsb_form_widget_check_empty ( widget ) == FALSE )
                scheduled -> limit_date = gsb_calendar_entry_get_date ( widget );
            else
                scheduled -> limit_date = NULL;
            break;
        case 6:
            widget = bet_form_scheduler_get_element_widget ( SCHEDULED_FORM_LIMIT_DATE );
            if ( gsb_form_widget_check_empty ( widget ) == FALSE )
                scheduled -> limit_date = gsb_calendar_entry_get_date ( widget );
            else
                scheduled -> limit_date = NULL;

            widget = bet_form_scheduler_get_element_widget (
                        SCHEDULED_FORM_FREQUENCY_USER_ENTRY );
            if ( gsb_form_widget_check_empty ( widget ) == FALSE )
                scheduled -> user_entry = utils_str_atoi (
                        gtk_entry_get_text ( GTK_ENTRY ( widget ) ) );
            if ( scheduled -> user_entry )
            {
                widget = bet_form_scheduler_get_element_widget (
                        SCHEDULED_FORM_FREQUENCY_USER_BUTTON );
                scheduled -> user_interval = gsb_combo_box_get_index ( widget );
            }
            else
                return FALSE;
    }

    /* On traite les données de transaction */
    widget = bet_form_widget_get_widget ( TRANSACTION_FORM_DATE );
    if ( gsb_form_widget_check_empty ( widget ) == FALSE )
    {
        GDate *date_tomorrow;

        date_tomorrow = gsb_date_tomorrow ( );
        scheduled -> date = gsb_calendar_entry_get_date ( widget );
        if ( scheduled -> date == NULL
         ||
         g_date_compare ( scheduled -> date, date_tomorrow ) < 0 )
        {
            g_date_free ( date_tomorrow );
            return FALSE;
        }
        g_date_free ( date_tomorrow );
    }
    else
        return FALSE;

    widget = bet_form_widget_get_widget ( TRANSACTION_FORM_EXERCICE );
    if ( gsb_form_widget_check_empty( widget ) == FALSE )
        scheduled -> fyear_number = gsb_fyear_get_fyear_from_combobox ( widget,
                        scheduled -> date );
    else
        scheduled -> fyear_number = 0;

    widget = bet_form_widget_get_widget ( TRANSACTION_FORM_PARTY );
    if ( gsb_form_widget_check_empty ( widget ) == FALSE )
        scheduled -> party_number = gsb_data_payee_get_number_by_name (
                        gtk_combofix_get_text ( GTK_COMBOFIX ( widget ) ), TRUE );
    else
        scheduled -> party_number = 0;

    widget = bet_form_widget_get_widget ( TRANSACTION_FORM_DEBIT );
    if ( gsb_form_widget_check_empty ( widget ) == FALSE )
    {
        gsb_form_check_auto_separator ( widget );
        scheduled -> amount = gsb_real_opposite ( utils_real_get_calculate_entry ( widget ) );
    }
    else
    {
        widget = bet_form_widget_get_widget ( TRANSACTION_FORM_CREDIT );
        if ( gsb_form_widget_check_empty ( widget ) == FALSE )
        {
            gsb_form_check_auto_separator ( widget );
            scheduled -> amount = utils_real_get_calculate_entry ( widget );
        }
        else
            return FALSE;
    }

    widget = bet_form_widget_get_widget ( TRANSACTION_FORM_TYPE );
    if ( gsb_form_widget_check_empty( widget ) == FALSE )
        scheduled -> payment_number =
                        gsb_payment_method_get_selected_number ( widget );
    else
        scheduled -> payment_number = 0;

    widget = bet_form_widget_get_widget ( TRANSACTION_FORM_CATEGORY );
    if ( gsb_form_widget_check_empty( widget ) == FALSE )
        bet_future_get_category_data ( widget, 0, ( gpointer ) scheduled );
    else
    {
        scheduled -> category_number = 0;
        scheduled -> sub_category_number = 0;
    }

    widget = bet_form_widget_get_widget ( TRANSACTION_FORM_BUDGET );
    if ( gsb_form_widget_check_empty( widget ) == FALSE )
        bet_future_get_budget_data ( widget, 0, ( gpointer ) scheduled );
    else
    {
        scheduled -> budgetary_number = 0;
        scheduled -> sub_budgetary_number = 0;
    }

    widget = bet_form_widget_get_widget ( TRANSACTION_FORM_NOTES );
    if ( gsb_form_widget_check_empty( widget ) == FALSE )
        scheduled -> notes = g_strdup ( gtk_entry_get_text ( GTK_ENTRY ( widget ) ) );
    else
        scheduled -> notes = NULL;

    return TRUE;
}


/**
 *
 *
 *
 *
 * */
gboolean bet_future_modify_line ( gint account_number,
                        gint number,
                        gint mother_row )
{
    gchar *tmp_str;
    gint result;

    if ( bet_futur_dialog == NULL )
    {
        bet_futur_dialog = bet_future_create_dialog ( account_number );
    }
    else
    {
        bet_form_clean ( account_number );
        gtk_widget_show ( bet_futur_dialog );
    }

     /* init data */
    bet_future_set_form_data_from_line ( account_number, number  );

dialog_return:
    result = gtk_dialog_run ( GTK_DIALOG ( bet_futur_dialog ) );

    if ( result == GTK_RESPONSE_OK )
    {
        FuturData *scheduled;

        scheduled = struct_initialise_bet_future ( );

        if ( !scheduled )
        {
            dialogue_error_memory ();
            gtk_widget_hide ( bet_futur_dialog );
            return FALSE;
        }

        if ( bet_future_take_data_from_form ( scheduled ) == FALSE )
        {
            tmp_str = g_strdup ( _("Error: the frequency defined by the user or the amount is "
                                 "not specified or the date is invalid.") );
            dialogue_warning_hint ( tmp_str, _("One field is not filled in") );
            g_free ( tmp_str );
            goto dialog_return;
        }
        else
        {
            scheduled -> number = number;
            scheduled -> mother_row = mother_row;
            bet_data_future_modify_lines ( scheduled );
        }

        gsb_data_account_set_bet_maj ( account_number, BET_MAJ_ESTIMATE );
        bet_data_update_bet_module ( account_number, GSB_ESTIMATE_PAGE );
    }

    gtk_widget_hide ( bet_futur_dialog );

    return FALSE;
}


/* BET TRANSFERT */
/**
 * sensibilise ou insensibilise la boite en fonction de l'état du bouton
 *
 * \param button
 * \param box
 *
 * \return
 * */
static void bet_transfert_last_banking_day_toggle ( GtkToggleButton *button,
                        GtkWidget *entry )
{
    if ( gtk_toggle_button_get_active ( button ) )
    {
        GDate *date;
        GDate *tmp_date;

        date = gsb_calendar_entry_get_date ( entry );
        if ( g_date_valid ( date ) )
        {
            tmp_date = gsb_date_get_last_banking_day_of_month ( date );
            g_date_free ( date );

            if ( g_date_valid ( tmp_date ) )
            {
                gsb_calendar_entry_set_date ( entry, tmp_date );
                gtk_widget_set_sensitive ( entry, FALSE );
                g_date_free ( tmp_date );
            }
        }
    }
    else
    {
        gtk_widget_set_sensitive ( entry, TRUE );
    }
}


/**
 * sensibilise ou insensibilise la boite en fonction de l'état du bouton
 *
 * \param button
 * \param box
 *
 * \return
 * */
static void bet_transfert_replace_data_toggle ( GtkToggleButton *button,
                        GtkWidget *box )
{
    if ( gtk_toggle_button_get_active ( button ) )
    {
        gtk_widget_set_sensitive ( box, TRUE );
    }
    else
    {
        GtkWidget *widget;

        widget = g_object_get_data ( G_OBJECT ( bet_transfert_dialog ), "bet_transfert_direct_debit" );
        if ( GTK_TOGGLE_BUTTON ( widget ) == button )
            widget = g_object_get_data ( G_OBJECT ( bet_transfert_dialog ), "bet_transfert_replace_data" );

        if ( gtk_toggle_button_get_active ( GTK_TOGGLE_BUTTON ( widget ) ) == FALSE )
            gtk_widget_set_sensitive ( box, FALSE );
    }
}


/**
 * retourne une list_store avec la liste des comptes concernés
 *
 *
 *
 * */
static GtkListStore *bet_transfert_create_account_list_store ( gint account_number )
{
    GtkListStore *list_store;
    GSList *tmp_list;
    gint nbre_rows = 0;

    /* create the model */
    list_store = gtk_list_store_new ( 4,
                        G_TYPE_STRING,      /* account name */
                        G_TYPE_STRING,      /* account kind : Cash account or Partial balance */
                        G_TYPE_INT,         /* account_number or partial_balance_number */
                        G_TYPE_BOOLEAN );   /* FALSE = account, TRUE = partial balance */

    /* populate the model if necessary */
    tmp_list = gsb_data_account_get_list_accounts ( );
    while ( tmp_list )
    {
        gint tmp_account_number;
        gint bet_credit_card;
        GtkTreeIter iter;

        tmp_account_number = gsb_data_account_get_no_account ( tmp_list -> data );

        if ( tmp_account_number != account_number )
        {
            bet_credit_card = gsb_data_account_get_bet_credit_card ( tmp_account_number );
            if ( bet_credit_card )
            {
                KindAccount kind;
                gchar *tmp_str = NULL;

                kind = gsb_data_account_get_kind ( tmp_account_number );
                switch ( kind )
                {
                case GSB_TYPE_BANK:
                    tmp_str = g_strdup ( _("Bank account") );
                    break;
                case GSB_TYPE_CASH:
                    tmp_str = g_strdup ( _("Cash account") );
                    break;
                case GSB_TYPE_LIABILITIES:
                    tmp_str = g_strdup ( _("Liabilities account") );
                    break;
                case GSB_TYPE_BALANCE:
                    tmp_str = NULL;
                    break;
                case GSB_TYPE_ASSET:
                    tmp_str = NULL;
                    break;

                }
                gtk_list_store_append ( list_store, &iter );
                gtk_list_store_set ( list_store,
                        &iter,
                        0, gsb_data_account_get_name ( tmp_account_number ),
                        1, tmp_str,
                        2, tmp_account_number,
                        3, FALSE,
                        -1 );
                nbre_rows ++;
                g_free ( tmp_str );
            }
        }

        tmp_list = tmp_list -> next;
    }

    tmp_list = gsb_data_partial_balance_get_list ( );
    while ( tmp_list )
    {
        gint tmp_number;
        GtkTreeIter iter;
        const gchar *liste_cptes;
        gchar **tab;
        gint bet_credit_card = 0;
        gint i;

        tmp_number = gsb_data_partial_balance_get_number ( tmp_list -> data );

        liste_cptes = gsb_data_partial_balance_get_liste_cptes ( tmp_number );
        tab = g_strsplit ( liste_cptes, ";", 0 );
        for ( i = 0; i < (gint) g_strv_length ( tab ); i++ )
        {
            gint tmp_account_number;

            tmp_account_number = utils_str_atoi ( tab[i] );
            bet_credit_card = gsb_data_account_get_bet_credit_card ( tmp_account_number );
        }

        g_strfreev ( tab );

        if ( bet_credit_card )
        {
            gtk_list_store_append ( list_store, &iter );
            gtk_list_store_set ( list_store,
                        &iter,
                        0, gsb_data_partial_balance_get_name ( tmp_number ),
                        1, _("Partial balance"),
                        2, tmp_number,
                        3, TRUE,
                        -1 );
                nbre_rows ++;
        }

        tmp_list = tmp_list -> next;
    }

    if ( nbre_rows == 0 )
        return NULL;
    else
        return list_store;
}


/**
 *
 *
 *
 *
 * */
static GtkWidget *bet_transfert_create_dialog ( gint account_number )
{
    GtkWidget *dialog;
    GtkWidget *main_vbox;
    GtkWidget *vbox;
    GtkWidget *paddingbox;
    GtkWidget *hbox;
    GtkWidget *date_bascule;
    GtkWidget *date_entry;
    GtkWidget *button;
    GtkWidget *combo;
    GtkWidget *sw;
    GtkWidget *tree_view;
    GtkWidget *label;
    GtkWidget *icon;
	GtkWidget *entry;
    GSList *tmp_list;

    /* Create the dialog */
    dialog = gtk_dialog_new_with_buttons ( _("Configuring a deferred debit account"),
                        GTK_WINDOW ( grisbi_app_get_active_window (NULL) ),
                        GTK_DIALOG_MODAL,
                        "gtk-cancel", GTK_RESPONSE_CANCEL,
                        "gtk-ok", GTK_RESPONSE_OK,
                        NULL );

    gtk_window_set_position ( GTK_WINDOW ( dialog ), GTK_WIN_POS_CENTER_ON_PARENT );
    gtk_window_set_resizable ( GTK_WINDOW ( dialog ), TRUE );
    gtk_dialog_set_default_response ( GTK_DIALOG ( dialog ), GTK_RESPONSE_OK );

    main_vbox = gtk_box_new ( GTK_ORIENTATION_VERTICAL, 0 );
    gtk_box_pack_start ( GTK_BOX ( dialog_get_content_area ( dialog ) ), main_vbox, TRUE, TRUE, 0 );
    gtk_container_set_border_width ( GTK_CONTAINER ( main_vbox ), BOX_BORDER_WIDTH );

    /* list of accounts */
    paddingbox = new_paddingbox_with_title ( main_vbox, FALSE,  _("List of accounts") );

    hbox = gtk_box_new ( GTK_ORIENTATION_HORIZONTAL, MARGIN_BOX );
    gtk_box_pack_start ( GTK_BOX ( paddingbox ), hbox, TRUE, TRUE, 0 );

    sw = gtk_scrolled_window_new (NULL, NULL);
	gtk_widget_set_size_request (GTK_WIDGET (sw), SW_MIN_HEIGHT, -1);
    gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW ( sw ),
                        GTK_SHADOW_ETCHED_IN);
    gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (sw),
                        GTK_POLICY_AUTOMATIC,
                        GTK_POLICY_ALWAYS);
    gtk_box_pack_start ( GTK_BOX ( hbox ), sw, TRUE,TRUE, 0 );

    /* create the account list */
    tree_view = bet_transfert_create_account_list_part ( dialog, account_number );

    if ( tree_view == NULL )
        return NULL;

    g_object_set_data ( G_OBJECT ( dialog ), "tree_view", tree_view );

    gtk_container_add (GTK_CONTAINER ( sw ), tree_view );

    /* Account with deferred debit card */
    paddingbox = new_paddingbox_with_title ( main_vbox, FALSE, _("Deferred debit card") );

    /* Effective Date */
    hbox = gtk_box_new ( GTK_ORIENTATION_HORIZONTAL, MARGIN_BOX );
    gtk_box_pack_start ( GTK_BOX ( paddingbox ), hbox, FALSE, TRUE, 0 );

    label = gtk_label_new ( _("Date of beginning of period: ") );
    gtk_box_pack_start ( GTK_BOX ( hbox ), label, FALSE, FALSE, 0 );

    date_bascule = gsb_calendar_entry_new ( FALSE );
    g_object_set_data ( G_OBJECT ( dialog ), "date_bascule", date_bascule );
    gtk_box_pack_start ( GTK_BOX ( hbox ), date_bascule, FALSE, FALSE, 0 );

    /* création de la boite de sélection du tiers */
    hbox = gtk_box_new ( GTK_ORIENTATION_HORIZONTAL, MARGIN_BOX );
    gtk_box_pack_start ( GTK_BOX ( paddingbox ), hbox, FALSE, TRUE, 0 );

    icon = utils_get_image_with_etat ( GTK_MESSAGE_ERROR, FALSE, _("This field is required"), NULL );
    gtk_box_pack_start ( GTK_BOX ( hbox ), icon, FALSE, FALSE, 0 );

	tmp_list = gsb_data_payee_get_name_and_report_list ();
	combo = gtk_combofix_new_with_properties (tmp_list,
											   etat.combofix_force_payee,
											   etat.combofix_case_sensitive,
											   FALSE,
											   METATREE_PAYEE);
	gsb_data_payee_free_name_and_report_list (tmp_list);

    gtk_box_pack_start ( GTK_BOX ( hbox ), combo, TRUE, TRUE, 0 );
    gsb_form_widget_set_empty (combo, TRUE);
    gtk_combofix_set_text ( GTK_COMBOFIX ( combo ), _("Payee") );

	entry = gtk_combofix_get_entry (GTK_COMBOFIX (combo));
    g_object_set_data ( G_OBJECT ( entry), "combo", combo );
    g_object_set_data ( G_OBJECT ( combo ), "icon", icon );
    g_object_set_data ( G_OBJECT ( dialog ), "bet_transfert_card_payee_combo", combo );

    g_signal_connect ( G_OBJECT (entry ),
                        "focus-in-event",
                        G_CALLBACK ( bet_form_entry_get_focus ),
                        NULL );
    g_signal_connect ( G_OBJECT (entry ),
                        "focus-out-event",
                        G_CALLBACK ( bet_transfert_entry_lose_focus ),
                        GINT_TO_POINTER ( TRANSACTION_FORM_PARTY ) );

    /* saisie des (sous)catégories et (sous)IB */
    hbox = gtk_box_new ( GTK_ORIENTATION_HORIZONTAL, MARGIN_BOX );
    gtk_box_pack_start ( GTK_BOX ( paddingbox ), hbox, FALSE, TRUE, 0 );

    icon = utils_get_image_with_etat ( GTK_MESSAGE_WARNING, FALSE, _("One of the two fields is recommended"), NULL );

    gtk_box_pack_start ( GTK_BOX ( hbox ), icon, FALSE, FALSE, 0 );

	/* set categories */
	tmp_list = gsb_data_category_get_name_list (TRUE, TRUE, TRUE, TRUE);
	combo = gtk_combofix_new_with_properties (tmp_list,
											   etat.combofix_force_category,
											   etat.combofix_case_sensitive,
											   etat.combofix_mixed_sort,
											   METATREE_CATEGORY);
	gsb_data_categorie_free_name_list (tmp_list);

    gtk_box_pack_start ( GTK_BOX ( hbox ), combo, FALSE, FALSE, 0 );

    gsb_form_widget_set_empty (combo, TRUE);
    gtk_combofix_set_text ( GTK_COMBOFIX ( combo ), _("Categories : Sub-categories") );

	entry = gtk_combofix_get_entry (GTK_COMBOFIX (combo));
    g_object_set_data ( G_OBJECT ( entry), "combo", combo );
    g_object_set_data ( G_OBJECT ( combo ), "icon", icon );
    g_object_set_data ( G_OBJECT ( dialog ), "bet_transfert_card_category_combo", combo );

    g_signal_connect ( G_OBJECT (entry ),
                        "focus-in-event",
                        G_CALLBACK ( bet_form_entry_get_focus ),
                        NULL );
    g_signal_connect ( G_OBJECT (entry ),
                        "focus-out-event",
                        G_CALLBACK ( bet_transfert_entry_lose_focus ),
                        GINT_TO_POINTER ( TRANSACTION_FORM_CATEGORY ) );

	/* set budget */
	tmp_list = gsb_data_budget_get_name_list (TRUE, TRUE);
	combo = gtk_combofix_new_with_properties (tmp_list,
											   etat.combofix_force_category,
											   etat.combofix_case_sensitive,
                        					   etat.combofix_mixed_sort,
											   METATREE_BUDGET);
	gsb_data_categorie_free_name_list (tmp_list);
    gtk_box_pack_start ( GTK_BOX ( hbox ), combo, FALSE, FALSE, 0 );

    gsb_form_widget_set_empty (combo, TRUE);
    gtk_combofix_set_text ( GTK_COMBOFIX ( combo ), _("Budgetary line") );

	entry = gtk_combofix_get_entry (GTK_COMBOFIX (combo));
    g_object_set_data ( G_OBJECT ( entry), "combo", combo );
    g_object_set_data ( G_OBJECT ( combo ), "icon", icon );
    g_object_set_data ( G_OBJECT ( dialog ), "bet_transfert_card_budget_combo", combo );

    g_signal_connect ( G_OBJECT (entry ),
                        "focus-in-event",
                        G_CALLBACK ( bet_form_entry_get_focus ),
                        NULL );
    g_signal_connect ( G_OBJECT (entry ),
                        "focus-out-event",
                        G_CALLBACK ( bet_transfert_entry_lose_focus ),
                        GINT_TO_POINTER ( TRANSACTION_FORM_BUDGET ) );

    /* Main account */
    paddingbox = new_paddingbox_with_title ( main_vbox, FALSE, _("Main account") );

    hbox = gtk_box_new ( GTK_ORIENTATION_HORIZONTAL, MARGIN_BOX );
    gtk_box_pack_start ( GTK_BOX ( paddingbox ), hbox, FALSE, TRUE, 0 );

    /* Effective Date */
    label = gtk_label_new ( _("Date of debit: ") );
    gtk_box_pack_start ( GTK_BOX ( hbox ), label, FALSE, FALSE, 0 );

    date_entry = gsb_calendar_entry_new ( FALSE );
    g_object_set_data ( G_OBJECT ( dialog ), "date_entry", date_entry );
    gtk_box_pack_start ( GTK_BOX ( hbox ), date_entry, FALSE, FALSE, 0 );

    button = gtk_check_button_new_with_label (
                        _("Last banking day of the month") );
    gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( button ), FALSE );
    g_object_set_data ( G_OBJECT ( dialog ), "bet_transfert_last_banking_day", button );
    gtk_box_pack_start ( GTK_BOX ( hbox ), button, FALSE, FALSE, 0 );

    g_signal_connect ( G_OBJECT ( button ),
                        "toggled",
                        G_CALLBACK ( bet_transfert_last_banking_day_toggle ),
                        date_entry );

    /* boite verticale pour rendre actif ou non les champs ci-dessous */
    vbox = gtk_box_new ( GTK_ORIENTATION_VERTICAL, MARGIN_BOX );
    g_object_set_data ( G_OBJECT ( dialog ), "bet_transfert_main_vbox_data", vbox );

    /* check button Automatic creation of the direct debit transaction */
    button = gtk_check_button_new_with_label (
                        _("Automatic creation of the direct debit transaction") );
    gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( button ), FALSE );
    g_object_set_data ( G_OBJECT ( dialog ), "bet_transfert_direct_debit", button );
    gtk_box_pack_start ( GTK_BOX ( paddingbox ), button, FALSE, FALSE, 0 );

    g_signal_connect ( G_OBJECT ( button ),
                        "toggled",
                        G_CALLBACK ( bet_transfert_replace_data_toggle ),
                        vbox );

    /* check button replace planned line */
    button = gtk_check_button_new_with_label (
                        _("Replacement of the scheduled transaction in the forecast") );
    gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( button ), FALSE );
    g_object_set_data ( G_OBJECT ( dialog ), "bet_transfert_replace_data", button );
    gtk_box_pack_start ( GTK_BOX ( paddingbox ), button, FALSE, FALSE, 0 );

    g_signal_connect ( G_OBJECT ( button ),
                        "toggled",
                        G_CALLBACK ( bet_transfert_replace_data_toggle ),
                        vbox );

    gtk_box_pack_start ( GTK_BOX ( paddingbox ), vbox, FALSE, FALSE, 0 );

    /* création de la boite de sélection du tiers */
    hbox = gtk_box_new ( GTK_ORIENTATION_HORIZONTAL, MARGIN_BOX );
    gtk_box_pack_start ( GTK_BOX ( vbox ), hbox, FALSE, TRUE, 0 );

    icon = utils_get_image_with_etat ( GTK_MESSAGE_ERROR, FALSE, _("This field is required"), NULL );
    gtk_box_pack_start ( GTK_BOX ( hbox ), icon, FALSE, FALSE, 0 );

	tmp_list = gsb_data_payee_get_name_and_report_list ();
	combo = gtk_combofix_new_with_properties (tmp_list,
											  etat.combofix_force_payee,
											  etat.combofix_case_sensitive,
											  FALSE,
											  METATREE_PAYEE);
	gsb_data_payee_free_name_and_report_list (tmp_list);
    gtk_box_pack_start ( GTK_BOX ( hbox ), combo, TRUE, TRUE, 0 );
    gsb_form_widget_set_empty (combo, TRUE);
    gtk_combofix_set_text ( GTK_COMBOFIX ( combo ), _("Payee") );

	entry = gtk_combofix_get_entry (GTK_COMBOFIX (combo));
    g_object_set_data ( G_OBJECT ( entry), "combo", combo );
    g_object_set_data ( G_OBJECT ( combo ), "icon", icon );
    g_object_set_data ( G_OBJECT ( dialog ), "bet_transfert_main_payee_combo", combo );

    g_signal_connect ( G_OBJECT (entry ),
                        "focus-in-event",
                        G_CALLBACK ( bet_form_entry_get_focus ),
                        NULL );
    g_signal_connect ( G_OBJECT (entry ),
                        "focus-out-event",
                        G_CALLBACK ( bet_transfert_entry_lose_focus ),
                        GINT_TO_POINTER ( TRANSACTION_FORM_PARTY ) );

    /* Création du mode de payement */
    combo = gtk_combo_box_new ();
    gsb_payment_method_create_combo_list ( combo,
                        GSB_PAYMENT_DEBIT,
                        account_number, 0, FALSE );
    gtk_combo_box_set_active ( GTK_COMBO_BOX ( combo ), 0 );
    gtk_widget_set_tooltip_text ( GTK_WIDGET ( combo ),
                        _("Choose the method of payment") );
    g_object_set_data ( G_OBJECT ( dialog ), "bet_transfert_main_payment_combo", combo );

    gtk_box_pack_start ( GTK_BOX ( hbox ), combo, FALSE, FALSE, 0 );

    /* saisie des (sous)catégories et (sous)IB */
    hbox = gtk_box_new ( GTK_ORIENTATION_HORIZONTAL, MARGIN_BOX );
    gtk_box_pack_start ( GTK_BOX ( vbox ), hbox, FALSE, TRUE, 0 );
    g_object_set_data ( G_OBJECT ( dialog ), "bet_transfert_hbox_replace_data", hbox );

    icon = utils_get_image_with_etat ( GTK_MESSAGE_ERROR, FALSE, _("One of the two fields is required"), NULL );
    gtk_box_pack_start ( GTK_BOX ( hbox ), icon, FALSE, FALSE, 0 );

	/* set categories */
	tmp_list = gsb_data_category_get_name_list (TRUE, TRUE, TRUE, TRUE);
	combo = gtk_combofix_new_with_properties (tmp_list,
											   etat.combofix_force_category,
											   etat.combofix_case_sensitive,
											   etat.combofix_mixed_sort,
											   METATREE_CATEGORY);
	gsb_data_categorie_free_name_list (tmp_list);
    gtk_box_pack_start ( GTK_BOX ( hbox ), combo, FALSE, FALSE, 0 );
    gsb_form_widget_set_empty (combo, TRUE);
    gtk_combofix_set_text ( GTK_COMBOFIX ( combo ), _("Categories : Sub-categories") );

	entry = gtk_combofix_get_entry (GTK_COMBOFIX (combo));
    g_object_set_data ( G_OBJECT ( entry ), "combo", combo );
    g_object_set_data ( G_OBJECT ( combo ), "icon", icon );
    g_object_set_data ( G_OBJECT ( dialog ), "bet_transfert_main_category_combo", combo );

    g_signal_connect ( G_OBJECT (entry ),
                        "focus-in-event",
                        G_CALLBACK ( bet_form_entry_get_focus ),
                        NULL );
    g_signal_connect ( G_OBJECT (entry ),
                        "focus-out-event",
                        G_CALLBACK ( bet_transfert_entry_lose_focus ),
                        GINT_TO_POINTER ( TRANSACTION_FORM_CATEGORY ) );

	/* set budget */
	tmp_list = gsb_data_budget_get_name_list (TRUE, TRUE);
	combo = gtk_combofix_new_with_properties (tmp_list,
											   etat.combofix_force_category,
											   etat.combofix_case_sensitive,
                        					   etat.combofix_mixed_sort,
											   METATREE_BUDGET);
    gtk_box_pack_start ( GTK_BOX ( hbox ), combo, FALSE, FALSE, 0 );
    gsb_data_categorie_free_name_list (tmp_list);
    gsb_form_widget_set_empty (combo, TRUE);
    gtk_combofix_set_text ( GTK_COMBOFIX ( combo ), _("Budgetary line") );

	entry = gtk_combofix_get_entry (GTK_COMBOFIX (combo));
	g_object_set_data ( G_OBJECT ( entry ), "combo", combo );
    g_object_set_data ( G_OBJECT ( combo ), "icon", icon );
    g_object_set_data ( G_OBJECT ( dialog ), "bet_transfert_main_budget_combo", combo );

    g_signal_connect ( G_OBJECT (entry ),
                        "focus-in-event",
                        G_CALLBACK ( bet_form_entry_get_focus ),
                        NULL );
    g_signal_connect ( G_OBJECT (entry ),
                        "focus-out-event",
                        G_CALLBACK ( bet_transfert_entry_lose_focus ),
                        GINT_TO_POINTER ( TRANSACTION_FORM_BUDGET ) );

    /* on grise vbox */
    gtk_widget_set_sensitive ( vbox, FALSE );

    gtk_widget_show_all ( dialog );

    /* return */
    return dialog;
}


/**
 * récupère les données du formulaire dialog
 *
 * \param   structure transfert
 * \param   dialog
 *
 *\return TRUE if OK FALSE if error
 * */
static gboolean bet_transfert_take_data (  TransfertData *transfert,
                        GtkWidget *dialog )
{
    GtkWidget *widget;
    GtkTreeView *tree_view;
    GtkTreeModel *model;
    GtkTreeIter iter;
    gint replace_account;
    gint type;

    tree_view = g_object_get_data ( G_OBJECT ( dialog ), "tree_view" );
    if (! gtk_tree_selection_get_selected ( GTK_TREE_SELECTION (
                        gtk_tree_view_get_selection ( GTK_TREE_VIEW ( tree_view ) ) ),
                        &model, &iter ))
        return FALSE;
    gtk_tree_model_get ( GTK_TREE_MODEL ( model ), &iter,
                        2, &replace_account,
                        3, &type,
                        -1 );
    transfert -> replace_account = replace_account;
    transfert -> type = type;

    /* Account with deferred debit card */
    widget = g_object_get_data ( G_OBJECT ( dialog ), "date_bascule" );
    if ( gsb_form_widget_check_empty ( widget ) == FALSE )
    {
        transfert -> date_bascule = gsb_calendar_entry_get_date ( widget );
        if ( transfert -> date_bascule == NULL )
            return FALSE;
    }
    else
        return FALSE;

    widget = g_object_get_data ( G_OBJECT ( dialog ), "bet_transfert_card_payee_combo" );
    if ( gsb_form_widget_check_empty ( widget ) == FALSE )
    {
        bet_future_get_payee_data ( widget, 2, ( gpointer ) transfert );
        if ( transfert->card_payee_number == 0 )
        {
            //~ printf ("bet_transfert_card_payee_combo est vide\n");
            return FALSE;
        }
    }
    else
        return FALSE;

    widget = g_object_get_data ( G_OBJECT ( dialog ), "bet_transfert_card_category_combo" );
    if ( gsb_form_widget_check_empty( widget ) == FALSE )
    {
        bet_future_get_category_data ( widget, 2, ( gpointer ) transfert );
    }

    widget = g_object_get_data ( G_OBJECT ( dialog ), "bet_transfert_card_budget_combo" );
    if ( gsb_form_widget_check_empty( widget ) == FALSE )
    {
        bet_future_get_budget_data ( widget, 2, ( gpointer ) transfert );
    }

    /* Main account */
    widget = g_object_get_data ( G_OBJECT ( dialog ), "date_entry" );
    if ( gsb_form_widget_check_empty ( widget ) == FALSE )
    {
        transfert -> date = gsb_calendar_entry_get_date ( widget );
        if ( transfert -> date == NULL )
            return FALSE;
    }
    else
        return FALSE;

    widget = g_object_get_data ( G_OBJECT ( dialog ), "bet_transfert_last_banking_day" );
    transfert->main_last_banking_date = gtk_toggle_button_get_active ( GTK_TOGGLE_BUTTON ( widget ) );

    widget = g_object_get_data ( G_OBJECT ( dialog ), "bet_transfert_replace_data" );
    transfert -> replace_transaction = gtk_toggle_button_get_active ( GTK_TOGGLE_BUTTON ( widget ) );

    widget = g_object_get_data ( G_OBJECT ( dialog ), "bet_transfert_direct_debit" );
    transfert->direct_debit = gtk_toggle_button_get_active ( GTK_TOGGLE_BUTTON ( widget ) );


    if ( transfert->replace_transaction || transfert->direct_debit )
    {
        gboolean empty = TRUE;

        widget = g_object_get_data ( G_OBJECT ( dialog ), "bet_transfert_main_payee_combo" );
        if ( gsb_form_widget_check_empty( widget ) == FALSE )
        {
            bet_future_get_payee_data ( widget, 1, ( gpointer ) transfert );
            if ( transfert->main_payee_number > 0 )
                empty = FALSE;
        }

        widget = g_object_get_data ( G_OBJECT ( dialog ), "bet_transfert_main_payment_combo" );
        bet_future_get_payment_data ( widget, ( gpointer ) transfert );

        widget = g_object_get_data ( G_OBJECT ( dialog ), "bet_transfert_main_category_combo" );
        if ( gsb_form_widget_check_empty( widget ) == FALSE )
        {
            bet_future_get_category_data ( widget, 1, ( gpointer ) transfert );
            empty = FALSE;
        }

        widget = g_object_get_data ( G_OBJECT ( dialog ), "bet_transfert_main_budget_combo" );
        if ( gsb_form_widget_check_empty( widget ) == FALSE )
        {
            bet_future_get_budget_data ( widget, 1, ( gpointer ) transfert );
            empty = FALSE;
        }

        if ( empty )
            return FALSE;
    }

    return TRUE;
}


/**
 * sélectionne le compte concerné par la modification du solde.
 *
 * \param struct transfert
 *
 * \return
 * */
static void bet_transfert_select_account_in_treeview ( TransfertData *transfert )
{
    GtkWidget *tree_view;
    GtkTreeModel *model;
    GtkTreeIter iter;
    gboolean valid;

    tree_view = g_object_get_data ( G_OBJECT ( bet_transfert_dialog ), "tree_view" );
    model = gtk_tree_view_get_model ( GTK_TREE_VIEW ( tree_view ) );

    valid = gtk_tree_model_get_iter_first ( model, &iter );
    while( valid)
    {
        gint tmp_number;
        gint type_de_compte;

        gtk_tree_model_get ( GTK_TREE_MODEL ( model ), &iter, 2, &tmp_number, 3, &type_de_compte, -1 );

        if ( transfert->type == type_de_compte && tmp_number == transfert->replace_account )
        {
            gtk_tree_selection_select_iter ( GTK_TREE_SELECTION (
                        gtk_tree_view_get_selection ( GTK_TREE_VIEW ( tree_view ) ) ), &iter );
            break;
        }
        valid = gtk_tree_model_iter_next ( GTK_TREE_MODEL ( model ), &iter );
    }
}


/**
 * remplit la boite de dialogue avec les données du solde de compte
 *
 * \param account_number
 * \param number            numéro du solde partiel
 *
 * \return                  TRUE if OK or FALSE
 * */
static gboolean bet_transfert_set_form_data_from_line ( gint account_number,
                        gint number )
{
    GtkWidget *widget;
    GHashTable *transfert_list;
    gchar *key;
    TransfertData *transfert;

    if ( account_number == 0 )
        key = g_strconcat ("0:", utils_str_itoa ( number ), NULL );
    else
        key = g_strconcat ( utils_str_itoa ( account_number ), ":",
                        utils_str_itoa ( number ), NULL );

    transfert_list = bet_data_transfert_get_list ( );

    transfert = g_hash_table_lookup ( transfert_list, key );
    if ( transfert == NULL )
        return FALSE;

    bet_transfert_select_account_in_treeview ( transfert );

    /* Account with deferred debit card */
    widget = g_object_get_data ( G_OBJECT ( bet_transfert_dialog ), "date_bascule" );
    if ( transfert->date_bascule )
    {
        gsb_calendar_entry_set_date ( widget, transfert -> date_bascule );
        gsb_form_widget_set_empty ( widget, FALSE );
    }
    else
        gsb_form_widget_set_empty ( widget, TRUE );

    widget = g_object_get_data ( G_OBJECT ( bet_transfert_dialog ), "bet_transfert_card_payee_combo" );
    if ( transfert->card_payee_number > 0 )
    {
        gtk_combofix_set_text ( GTK_COMBOFIX ( widget ),
                        gsb_data_payee_get_name ( transfert->card_payee_number, FALSE ) );

        gsb_form_widget_set_empty (widget, FALSE);
        gtk_editable_set_position (GTK_EDITABLE (gtk_combofix_get_entry (GTK_COMBOFIX (widget))), 0);

        /* on actualise l'état de l'icone associée */
        utils_set_image_with_etat ( widget, TRUE );
    }
    else
    {
        gtk_combofix_set_text ( GTK_COMBOFIX ( widget ), _("Payee") );
        gsb_form_widget_set_empty (widget, TRUE);

        /* on actualise l'état de l'icone associée */
        utils_set_image_with_etat ( widget, FALSE );
    }

    widget = g_object_get_data ( G_OBJECT ( bet_transfert_dialog ), "bet_transfert_card_category_combo" );
    if ( transfert->card_category_number > 0 )
    {
        gtk_combofix_set_text ( GTK_COMBOFIX ( widget ),
                        gsb_data_category_get_name ( transfert->card_category_number,
                        transfert->card_sub_category_number,
                        NULL ) );
        gsb_form_widget_set_empty (widget, FALSE);
        gtk_editable_set_position (GTK_EDITABLE (gtk_combofix_get_entry (GTK_COMBOFIX (widget))), 0);
    }
    else
    {
        gtk_combofix_set_text ( GTK_COMBOFIX ( widget ), _("Categories : Sub-categories") );
        gsb_form_widget_set_empty (widget, TRUE);
    }

    widget = g_object_get_data ( G_OBJECT ( bet_transfert_dialog ), "bet_transfert_card_budget_combo" );
    if ( transfert->card_budgetary_number > 0 )
    {
        gtk_combofix_set_text ( GTK_COMBOFIX ( widget ),
                        gsb_data_budget_get_name ( transfert->card_budgetary_number,
                        transfert->card_sub_budgetary_number,
                        NULL ) );
        gsb_form_widget_set_empty (widget, FALSE);
    }
    else
    {
        gtk_combofix_set_text ( GTK_COMBOFIX ( widget ), _("Budgetary line") );
        gsb_form_widget_set_empty (widget, TRUE);
    }

    if ( transfert->card_category_number > 0 || transfert->card_budgetary_number > 0 )
        utils_set_image_with_etat ( widget, TRUE );
    else
        utils_set_image_with_etat ( widget, FALSE );


    /* Main account */
    widget = g_object_get_data ( G_OBJECT ( bet_transfert_dialog ), "date_entry" );
    gsb_calendar_entry_set_date ( widget, transfert -> date );
    gsb_form_widget_set_empty ( widget, FALSE );
	if (transfert->main_last_banking_date)
		gtk_widget_set_sensitive (widget, FALSE);
	else
		gtk_widget_set_sensitive (widget, TRUE);

    widget = g_object_get_data ( G_OBJECT ( bet_transfert_dialog ), "bet_transfert_last_banking_day" );
    gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( widget ), transfert->main_last_banking_date );

    widget = g_object_get_data ( G_OBJECT ( bet_transfert_dialog ), "bet_transfert_main_payee_combo" );
    if ( transfert->main_payee_number > 0 )
    {
        gtk_combofix_set_text ( GTK_COMBOFIX ( widget ),
                        gsb_data_payee_get_name ( transfert->main_payee_number, FALSE ) );

        gsb_form_widget_set_empty (widget, FALSE );
        gtk_editable_set_position (GTK_EDITABLE (gtk_combofix_get_entry (GTK_COMBOFIX (widget))), 0);

         /* on actualise l'état de l'icone associée */
        utils_set_image_with_etat ( widget, TRUE );
   }
    else
    {
        gtk_combofix_set_text ( GTK_COMBOFIX ( widget ), _("Payee") );
        gsb_form_widget_set_empty (widget, TRUE );
    }

    widget = g_object_get_data ( G_OBJECT ( bet_transfert_dialog ), "bet_transfert_main_payment_combo" );
    gsb_payment_method_set_payment_position ( widget, transfert->main_payment_number );

    widget = g_object_get_data ( G_OBJECT ( bet_transfert_dialog ), "bet_transfert_main_category_combo" );
    if ( transfert->main_category_number > 0 )
    {
        gtk_combofix_set_text ( GTK_COMBOFIX ( widget ),
                        gsb_data_category_get_name ( transfert->main_category_number,
                        transfert->main_sub_category_number,
                        NULL ) );
        gsb_form_widget_set_empty (widget, FALSE );
        gtk_editable_set_position (GTK_EDITABLE (gtk_combofix_get_entry (GTK_COMBOFIX (widget))), 0);
    }
    else
    {
        gtk_combofix_set_text ( GTK_COMBOFIX ( widget ), _("Categories : Sub-categories") );
        gsb_form_widget_set_empty (widget, TRUE );

        /* on actualise l'état de l'icone associée */
        utils_set_image_with_etat ( widget, FALSE );
    }

    widget = g_object_get_data ( G_OBJECT ( bet_transfert_dialog ), "bet_transfert_main_budget_combo" );
    if ( transfert->main_budgetary_number > 0 )
    {
        gtk_combofix_set_text ( GTK_COMBOFIX ( widget ),
                        gsb_data_budget_get_name ( transfert->main_budgetary_number,
                        transfert->main_sub_budgetary_number,
                        NULL ) );
        gsb_form_widget_set_empty (widget, FALSE );
    }
    else
    {
        gtk_combofix_set_text ( GTK_COMBOFIX ( widget ), _("Budgetary line") );
        gsb_form_widget_set_empty (widget, TRUE);
    }

    if ( transfert->main_category_number > 0 || transfert->main_budgetary_number > 0 )
        utils_set_image_with_etat ( widget, TRUE );
    else
        utils_set_image_with_etat ( widget, FALSE );

    widget = g_object_get_data ( G_OBJECT ( bet_transfert_dialog ), "bet_transfert_replace_data" );
    gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( widget ), transfert->replace_transaction );

    widget = g_object_get_data ( G_OBJECT ( bet_transfert_dialog ), "bet_transfert_direct_debit" );
    gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( widget ), transfert->direct_debit );

    /* données de l'opération pour le compte principal */
    widget = g_object_get_data ( G_OBJECT ( bet_transfert_dialog ), "bet_transfert_main_vbox_data" );
    gtk_widget_set_sensitive ( widget, ( transfert->replace_transaction + transfert->direct_debit ) );


    return TRUE;
}


/**
 *
 *
 *
 *
 * */
gboolean bet_transfert_new_line_dialog ( GtkTreeModel *tab_model,
                        gchar *str_date )
{
    GtkWidget *widget;
    GDate *date;
    GDate *date_jour;
    gint account_number;
    gint result;

    account_number = gsb_gui_navigation_get_current_account ( );
    if ( account_number == -1 )
        return FALSE;

    /* Create the dialog */
    bet_transfert_dialog = bet_transfert_create_dialog ( account_number );
    if ( bet_transfert_dialog == NULL )
    {
        dialogue_warning ( _("You must have at least one account with deferred debit card.") );
        return FALSE;
    }

    /* init data */
    widget = g_object_get_data ( G_OBJECT ( bet_transfert_dialog ), "date_entry" );
    date = gsb_parse_date_string ( str_date );
    date_jour = gdate_today ( );

    if ( g_date_valid ( date ) == FALSE )
        date = date_jour;

    gsb_form_widget_set_empty ( widget, FALSE );
    gsb_calendar_entry_set_date ( widget, date );

dialog_return:
    result = gtk_dialog_run ( GTK_DIALOG ( bet_transfert_dialog ) );

    if ( result == GTK_RESPONSE_OK )
    {
        GtkWidget *tree_view;
        gchar *tmp_str;
        TransfertData *transfert;

        transfert = struct_initialise_bet_transfert ( );

        if ( !transfert )
        {
            dialogue_error_memory ();
            gtk_widget_hide ( bet_transfert_dialog );
            return FALSE;
        }
        transfert -> account_number = account_number;

        tree_view = g_object_get_data ( G_OBJECT ( bet_transfert_dialog ), "tree_view" );
        if ( !gtk_tree_selection_get_selected ( GTK_TREE_SELECTION (
                        gtk_tree_view_get_selection ( GTK_TREE_VIEW ( tree_view ) ) ),
                        NULL, NULL ) )
        {
            tmp_str = g_strdup ( _("Error: You must select an account.") );
            dialogue_warning_hint ( tmp_str, _("Missing data") );
            g_free ( tmp_str );
            gtk_dialog_set_response_sensitive ( GTK_DIALOG ( bet_transfert_dialog ),
                        GTK_RESPONSE_OK, FALSE );
            goto dialog_return;
        }
        else if ( bet_transfert_take_data (  transfert, bet_transfert_dialog ) == FALSE )
        {
            tmp_str = g_strdup ( _("Error: a date is not present or a field is in error.") );
            dialogue_warning_hint ( tmp_str, _("One field is not filled in") );
            g_free ( tmp_str );
            goto dialog_return;
        }
        else
            bet_data_transfert_add_line ( transfert );

        gsb_data_account_set_bet_maj ( account_number, BET_MAJ_ESTIMATE );
        bet_data_update_bet_module ( account_number, GSB_ESTIMATE_PAGE );
    }

    gtk_widget_destroy ( bet_transfert_dialog );
    bet_transfert_dialog = NULL;

    return FALSE;
}


/**
 *
 *
 *
 *
 * */
GtkWidget *bet_transfert_create_account_list_part ( GtkWidget *dialog,
                        gint account_number )
{
    GtkWidget *tree_view;
    GtkListStore *list_store;
    GtkTreeViewColumn *column;
    GtkCellRenderer *cell;
    GtkTreeSelection *selection;

    list_store = bet_transfert_create_account_list_store ( account_number );
    if ( list_store == NULL )
        return NULL;

    /* create the treeview */
    tree_view = gtk_tree_view_new_with_model ( GTK_TREE_MODEL ( list_store ) );
    g_object_unref ( list_store );

    /* set the color of selected row */
    gtk_widget_set_size_request ( tree_view, 400, 150 );

    /* Account_name */
    cell = gtk_cell_renderer_text_new ( );
    column = gtk_tree_view_column_new_with_attributes ( _("Name"),
                        cell, "text", 0, NULL );
    gtk_tree_view_column_set_expand ( column, TRUE );
    gtk_tree_view_column_set_alignment ( column, 0.5 );
    gtk_tree_view_column_set_sort_column_id ( column, 0 );
    gtk_tree_view_append_column ( GTK_TREE_VIEW ( tree_view ), column );

    /* type of account */
    cell = gtk_cell_renderer_text_new ( );
    column = gtk_tree_view_column_new_with_attributes ( _("Type"),
                        cell, "text", 1, NULL );
    gtk_tree_view_column_set_expand ( column, TRUE );
    gtk_tree_view_column_set_alignment ( column, 0.5 );
    gtk_tree_view_column_set_sort_column_id ( column, 1 );
    gtk_tree_view_append_column ( GTK_TREE_VIEW ( tree_view ), column );

    selection = gtk_tree_view_get_selection ( GTK_TREE_VIEW ( tree_view ) );
    g_signal_connect ( G_OBJECT ( selection ),
                        "changed",
                        G_CALLBACK ( bet_transfert_selection_changed ),
                        dialog );

    return tree_view;
}


/**
 *
 *
 *
 *
 * */
gboolean bet_transfert_selection_changed ( GtkTreeSelection *selection,
                        GtkWidget *dialog )
{
    if ( !GTK_IS_DIALOG ( dialog ) )
        return FALSE;

    if ( gtk_tree_selection_get_selected ( GTK_TREE_SELECTION ( selection ), NULL, NULL ) )
        gtk_dialog_set_response_sensitive ( GTK_DIALOG ( dialog ), GTK_RESPONSE_OK, TRUE );
    else
        gtk_dialog_set_response_sensitive ( GTK_DIALOG ( dialog ), GTK_RESPONSE_OK, FALSE );

    return TRUE;
}


/**
 *
 *
 *
 *
 * */
gboolean bet_transfert_entry_lose_focus ( GtkWidget *entry,
                        GdkEventFocus *ev,
                        gint *ptr_origin )
{
    GtkWidget *widget;
    gchar *string;
    gint element_number;

    /* still not found, if change the content of the form, something come in entry
     * which is nothing, so protect here */
    if ( !GTK_IS_WIDGET ( entry )
     ||
     !GTK_IS_ENTRY ( entry ) )
        return FALSE;

    /* remove the selection */
    gtk_editable_select_region ( GTK_EDITABLE ( entry ), 0, 0 );

    element_number = GPOINTER_TO_INT ( ptr_origin );
    if ( element_number != TRANSACTION_FORM_PARTY
     &&
     element_number != TRANSACTION_FORM_CATEGORY
     &&
     element_number != TRANSACTION_FORM_BUDGET )
        return FALSE;

    widget = g_object_get_data ( G_OBJECT ( entry ), "combo" );

    /* sometimes the combofix popus stays showed, so remove here */
    gtk_combofix_hide_popup ( GTK_COMBOFIX ( widget ) );

    /* string will be filled only if the field is empty */
    string = NULL;
    switch ( element_number )
    {
    case TRANSACTION_FORM_PARTY :
        if ( !strlen ( gtk_entry_get_text ( GTK_ENTRY ( entry ) ) ) )
            string = gsb_form_widget_get_name ( TRANSACTION_FORM_PARTY );
        break;

    case TRANSACTION_FORM_CATEGORY :
        if ( !strlen ( gtk_entry_get_text ( GTK_ENTRY ( entry ) ) ) )
            string = gsb_form_widget_get_name ( TRANSACTION_FORM_CATEGORY );
        break;

    case TRANSACTION_FORM_BUDGET :
        if ( !strlen ( gtk_entry_get_text ( GTK_ENTRY ( entry ) ) ) )
            string = gsb_form_widget_get_name ( TRANSACTION_FORM_BUDGET );
        break;

    default :
        break;

    }

    /* if string is not NULL, the entry is empty so set the empty field to TRUE */
    if ( string )
    {
        switch ( element_number)
        {
            case TRANSACTION_FORM_PARTY :
            case TRANSACTION_FORM_CATEGORY :
            case TRANSACTION_FORM_BUDGET :
            /* need to work with the combofix to avoid some signals if we work
             * directly on the entry */
            gtk_combofix_set_text ( GTK_COMBOFIX ( widget ), _(string) );
            break;

            default:
                gtk_entry_set_text ( GTK_ENTRY ( entry ), string );
            break;
        }
        gsb_form_widget_set_empty ( entry, TRUE );
    }

    return FALSE;
}


/**
 *
 *
 *
 *
 * */
gsb_real gsb_transfert_get_str_amount ( gsb_real amount,
                        gint account_currency,
                        gint replace_currency,
                        gint floating_point )
{
    gsb_real tmp_real = null_real;
    gint link_number;

    if ( ( link_number = gsb_data_currency_link_search ( account_currency, replace_currency ) ) )
    {
        if ( gsb_data_currency_link_get_first_currency ( link_number ) == replace_currency )
            tmp_real = gsb_real_mul ( amount,
                        gsb_data_currency_link_get_change_rate ( link_number ) );
        else
            tmp_real = gsb_real_div ( amount,
                        gsb_data_currency_link_get_change_rate ( link_number ) );
    }
    else if ( account_currency > 0 && replace_currency > 0 )
    {
        gchar *tmp_str;

        tmp_str = g_strdup ( _("Error: is missing one or more links between currencies.\n"
                        "You need to fix it and start over.") );
        dialogue_error ( tmp_str );

        g_free ( tmp_str );
    }

    return tmp_real;
}


/**
 *
 *
 *
 *
 * */
gboolean bet_transfert_modify_line ( gint account_number,
                        gint number )
{
    gint result;

    bet_transfert_dialog = bet_transfert_create_dialog ( account_number );
    if ( bet_transfert_dialog == NULL )
    {
        gchar *tmp_str;

        tmp_str = g_strdup ( _("There is an inconsistency in the balance of "
                    "a deferred debit account.\n"
                    "You should delete it and create another") );

        dialogue_error_hint ( tmp_str, _("Error of configuration") );

        return FALSE;
    }

    /* init data */
    bet_transfert_set_form_data_from_line ( account_number, number  );

dialog_return:
    result = gtk_dialog_run ( GTK_DIALOG ( bet_transfert_dialog ) );

    if ( result == GTK_RESPONSE_OK )
    {
        GtkWidget *tree_view;
        gchar *tmp_str;
        TransfertData *transfert;

        transfert = struct_initialise_bet_transfert ( );

        if ( !transfert )
        {
            dialogue_error_memory ();
            gtk_widget_hide ( bet_transfert_dialog );
            return FALSE;
        }
        transfert -> account_number = account_number;

        tree_view = g_object_get_data ( G_OBJECT ( bet_transfert_dialog ), "tree_view" );
        if ( !gtk_tree_selection_get_selected ( GTK_TREE_SELECTION (
                        gtk_tree_view_get_selection ( GTK_TREE_VIEW ( tree_view ) ) ),
                        NULL, NULL ) )
        {
            tmp_str = g_strdup ( _("Error: You must select an account.") );
            dialogue_warning_hint ( tmp_str, _("Missing data") );
            g_free ( tmp_str );
            gtk_dialog_set_response_sensitive ( GTK_DIALOG ( bet_transfert_dialog ),
                        GTK_RESPONSE_OK, FALSE );
            goto dialog_return;
        }
        else if ( bet_transfert_take_data (  transfert, bet_transfert_dialog ) == FALSE )
        {
            tmp_str = g_strdup ( _("Error: a date is not present or a field is in error.") );
            dialogue_warning_hint ( tmp_str, _("One field is not filled in") );
            g_free ( tmp_str );
            goto dialog_return;
        }
        else
        {
            transfert -> number = number;
            bet_data_transfert_modify_line ( transfert );
        }

        gsb_data_account_set_bet_maj ( account_number, BET_MAJ_ESTIMATE );
        bet_data_update_bet_module ( account_number, GSB_ESTIMATE_PAGE );
    }

    gtk_widget_destroy ( bet_transfert_dialog );
    bet_transfert_dialog = NULL;

    return TRUE;
}


/**
 *
 *
 * \param
 *
 * \return
 * */
/* Local Variables: */
/* c-basic-offset: 4 */
/* End: */
