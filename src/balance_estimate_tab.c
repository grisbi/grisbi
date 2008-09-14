/*     Copyright (C) 2007 Dominique Parisot
 * 			zionly@free.org
 * 			http://www.grisbi.org
 *
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 *
 *     This program is distributed in the hope that it will be useful,
 *     but WITHOUT ANY WARRANTY; without even the implied warranty of
 *     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *     GNU General Public License for more details.
 *
 *     You should have received a copy of the GNU General Public License
 *     along with this program; if not, write to the Free Software
 *     Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

/*
 * prefix bet : Balance Estimate Tab
 *
 * TODO : change the color of each line in the graph :
 * red if balance is less than 0.
 * orange if balance is less than the minimum wanted balance.
 * TODO : add a select button to display the selected line in the array
 * in the scheduler tab or in the account tab.
 */
#include "include.h"
#include <config.h>

#ifdef ENABLE_BALANCE_ESTIMATE

/*START_INCLUDE*/
#include "balance_estimate_tab.h"
#include "./utils_dates.h"
#include "./gsb_data_account.h"
#include "./gsb_data_scheduled.h"
#include "./gsb_data_transaction.h"
#include "./gsb_real.h"
#include "./gsb_scheduler.h"
#include "./gsb_transactions_list_sort.h"
#include "./main.h"
#include "./include.h"
#include "./gsb_real.h"
/*END_INCLUDE*/

/*START_STATIC*/
static  void bet_account_selection_changed (GtkTreeSelection *treeselection, gpointer user_data);
static  gint bet_date_sort_function (GtkTreeModel *model, GtkTreeIter *itera, GtkTreeIter *iterb, gpointer user_data);
static  void bet_duration_button_clicked (GtkToggleButton *togglebutton, gpointer data);
static  void bet_estimate_refresh();
static  gboolean bet_update_average_column (GtkTreeModel *model, GtkTreePath *path, GtkTreeIter *iter, gpointer data);
static  gboolean bet_update_graph (GtkTreeModel *model, GtkTreePath *path, GtkTreeIter *iter, gpointer data);
/*END_STATIC*/

/*START_EXTERN*/
extern gsb_real null_real ;
extern GtkTreeSelection * selection ;
extern GtkWidget *window ;
/*END_EXTERN*/



#include "balance_estimate_tab.h"
#include "gsb_data_account.h"
#include "gsb_scheduler.h"
#include "gsb_data_scheduled.h"
#include "gsb_data_account.h"
#include "gsb_data_transaction.h"
#include "utils_dates.h"
#include "gsb_transactions_list_sort.h"
enum bet_account_tree_columns {
	SPP_ACCOUNT_TREE_NAME_COLUMN,
	SPP_ACCOUNT_TREE_NUM_COLUMNS
};
enum bet_estimation_tree_columns {
	SPP_ESTIMATE_TREE_DATE_COLUMN,
	SPP_ESTIMATE_TREE_DESC_COLUMN,
	SPP_ESTIMATE_TREE_DEBIT_COLUMN,
	SPP_ESTIMATE_TREE_CREDIT_COLUMN,
	SPP_ESTIMATE_TREE_BALANCE_COLUMN,
	SPP_ESTIMATE_TREE_SORT_DATE_COLUMN,
	SPP_ESTIMATE_TREE_NUM_COLUMNS
};
static gchar* bet_duration_array[] = {
	N_("One month"),
	N_("Two months"),
	N_("Three months"),
	N_("Six months"),
	N_("One year"),
	N_("Two years"),
	NULL
};
static gint bet_months_array[] = {
	1, 2, 3, 6, 12, 24
};
struct bet_range
{
	GDate min_date;
	GDate max_date;
	gsb_real min_balance;
	gsb_real max_balance;
	gsb_real current_balance;
};
static GtkWidget *bet_container = NULL;
gint bet_date_sort_function (GtkTreeModel *model, GtkTreeIter *a, GtkTreeIter *b, gpointer user_data);

/*
 * bet_duration_button_clicked
 * This function is called when a radio button is called to change the estimate duration.
 * It copies the new durations from the data parameter (of the radio button) into
 * the bet_months property of the bet container
 */
static void bet_duration_button_clicked (GtkToggleButton *togglebutton, gpointer data)
{
	gint months = GPOINTER_TO_INT(data);
    	g_object_set_data (G_OBJECT(bet_container),
		"bet_months", GINT_TO_POINTER(months));
	bet_estimate_refresh();
}

/*
 * bet_account_selection_changed
 * This function is called for each change in the selected line in the account list.
 * It refreshs balance estimation.
 */
static void bet_account_selection_changed (GtkTreeSelection *treeselection, gpointer user_data)
{
	bet_estimate_refresh();
}

/*
 * bet_create_balance_estimate_tab
 *
 * This function create the widget (notebook) which contains all the
 * balance estimate interface. This widget is added in the main window
 */
GtkWidget *bet_create_balance_estimate_tab(void)
{
	/* create a notebook for array and graph */
	GtkWidget* notebook = gtk_notebook_new();
	gtk_widget_show(notebook);
	bet_container = notebook;
	g_signal_connect ( G_OBJECT (bet_container ), "destroy",
			G_CALLBACK ( gtk_widget_destroyed), &bet_container );

	/****** Parameter page ******/
	GtkWidget *widget = gtk_label_new(_("Estimate parameters"));
	gtk_widget_show(GTK_WIDGET(widget));
	GtkWidget *page = gtk_vbox_new(FALSE, 5);
	gtk_widget_show(GTK_WIDGET(page));
	gtk_notebook_append_page(GTK_NOTEBOOK(notebook),
		GTK_WIDGET(page), GTK_WIDGET(widget));

	GtkWidget *hbox = gtk_hbox_new(FALSE, 5);
	gtk_widget_show(GTK_WIDGET(hbox));
	gtk_box_pack_start(GTK_BOX(page), hbox, FALSE, FALSE, 15);

	widget = gtk_image_new_from_stock(
		GTK_STOCK_DIALOG_INFO, GTK_ICON_SIZE_DIALOG);
	gtk_widget_show(GTK_WIDGET(widget));
	gtk_box_pack_start(GTK_BOX(hbox), widget, FALSE, FALSE, 5);
	widget = gtk_label_new(NULL);
	gtk_label_set_markup(GTK_LABEL(widget),
		_("Please select an account and a duration\n"\
		"and select the estimate array tab."));
	gtk_widget_show(GTK_WIDGET(widget));
	gtk_box_pack_start(GTK_BOX(hbox), widget, FALSE, FALSE, 5);

	hbox = gtk_hbox_new(FALSE, 5);
	gtk_widget_show(GTK_WIDGET(hbox));
	gtk_box_pack_start(GTK_BOX(page), hbox, FALSE, FALSE, 5);

	/* create the account list */
	GtkWidget *tree_view = gtk_tree_view_new();
    	g_object_set_data (G_OBJECT(notebook), "bet_account_treeview", tree_view);
	gtk_widget_show(tree_view);
	GtkTreeStore *tree_model = gtk_tree_store_new(SPP_ACCOUNT_TREE_NUM_COLUMNS, G_TYPE_STRING);
	gtk_tree_view_set_model(GTK_TREE_VIEW(tree_view), GTK_TREE_MODEL(tree_model));
	g_object_unref (G_OBJECT(tree_model));
	GtkTreeSelection* tree_selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(tree_view));
	gtk_tree_selection_set_mode(tree_selection, GTK_SELECTION_SINGLE);
	g_signal_connect(G_OBJECT(tree_selection), "changed",
		G_CALLBACK(bet_account_selection_changed), NULL);

	GtkWidget *scrolled_window = gtk_scrolled_window_new ( NULL, NULL );
	gtk_widget_show(scrolled_window);
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolled_window),
		GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
	gtk_container_add (GTK_CONTAINER(scrolled_window), tree_view);
	gtk_widget_show(scrolled_window);
	gtk_box_pack_start(GTK_BOX(hbox), GTK_WIDGET(scrolled_window), TRUE, TRUE, 0);

	GtkCellRenderer *cell = gtk_cell_renderer_text_new ();
	GtkTreeViewColumn *column = gtk_tree_view_column_new_with_attributes (
		_("Account"), cell,
		"text", SPP_ACCOUNT_TREE_NAME_COLUMN,
		NULL);
	gtk_tree_view_append_column(GTK_TREE_VIEW(tree_view),
		GTK_TREE_VIEW_COLUMN(column));

	/* create duration selection */
	GtkWidget *vbox = gtk_vbox_new(FALSE, 5);
	gtk_widget_show(GTK_WIDGET(vbox));
	gtk_box_pack_start(GTK_BOX(hbox), GTK_WIDGET(vbox), FALSE, FALSE, 5);

	gint iduration;
	GtkWidget *previous = NULL;
	for (iduration = 0; bet_duration_array[iduration] != NULL; iduration++)
	{
		GtkWidget *widget;
		if (previous == NULL)
		{
			widget = gtk_radio_button_new_with_label(NULL,
				_(bet_duration_array[iduration]));
			previous = widget;
		} else {
			widget = gtk_radio_button_new_with_label_from_widget(
				GTK_RADIO_BUTTON(previous),
				_(bet_duration_array[iduration]));
		}
		gtk_widget_show(GTK_WIDGET(widget));
		gtk_box_pack_start(GTK_BOX(vbox), GTK_WIDGET(widget), FALSE, FALSE, 5);
		g_signal_connect(GTK_OBJECT(widget),
			"toggled",
			G_CALLBACK(bet_duration_button_clicked),
			GINT_TO_POINTER(bet_months_array[iduration]));
	}

	/****** Estimation array page ******/
	widget = gtk_label_new(_("Estimate array"));
	gtk_widget_show(widget);
	vbox = gtk_vbox_new(FALSE, 5);
	gtk_widget_show(GTK_WIDGET(vbox));
	gtk_notebook_append_page(GTK_NOTEBOOK(notebook),
		GTK_WIDGET(vbox), GTK_WIDGET(widget));

	/* create the title */
	widget = gtk_label_new("Estimation array");
	gtk_widget_show(GTK_WIDGET(widget));
	gtk_box_pack_start(GTK_BOX(vbox), GTK_WIDGET(widget), FALSE, FALSE, 5);
    	g_object_set_data (G_OBJECT(notebook), "bet_array_title", widget);

	/* create the estimate treeview */
	tree_view = gtk_tree_view_new();
    	g_object_set_data (G_OBJECT(bet_container), "bet_estimate_treeview", tree_view);
	gtk_widget_show(tree_view);
	tree_model = gtk_tree_store_new(SPP_ESTIMATE_TREE_NUM_COLUMNS,
		G_TYPE_STRING, /* SPP_ESTIMATE_TREE_DATE_COLUMN */
		G_TYPE_STRING, /* SPP_ESTIMATE_TREE_DESC_COLUMN */
		G_TYPE_STRING, /* SPP_ESTIMATE_TREE_DEBIT_COLUMN */
		G_TYPE_STRING, /* SPP_ESTIMATE_TREE_CREDIT_COLUMN */
		G_TYPE_STRING, /* SPP_ESTIMATE_TREE_BALANCE_COLUMN */
		G_TYPE_DATE);  /* SPP_ESTIMATE_TREE_SORT_DATE_COLUMN */
	gtk_tree_view_set_model(GTK_TREE_VIEW(tree_view), GTK_TREE_MODEL(tree_model));
	g_object_unref (G_OBJECT(tree_model));

	/* sort by date */
	gtk_tree_sortable_set_sort_func ( GTK_TREE_SORTABLE(tree_model),
					  SPP_ESTIMATE_TREE_SORT_DATE_COLUMN,
					  (GtkTreeIterCompareFunc) bet_date_sort_function,
					  NULL, NULL );
	gtk_tree_sortable_set_sort_column_id(GTK_TREE_SORTABLE(tree_model),
					     SPP_ESTIMATE_TREE_SORT_DATE_COLUMN, GTK_SORT_DESCENDING);

	scrolled_window = gtk_scrolled_window_new ( NULL, NULL );
	gtk_widget_show(scrolled_window);
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolled_window),
		GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
	gtk_container_add (GTK_CONTAINER(scrolled_window), tree_view);
	gtk_widget_show(scrolled_window);
	gtk_box_pack_start(GTK_BOX(vbox),
		GTK_WIDGET(scrolled_window), TRUE, TRUE, 5);

	/* Date column */
	cell = gtk_cell_renderer_text_new ();
	column = gtk_tree_view_column_new_with_attributes (
		_("Date"), cell,
		"text", SPP_ESTIMATE_TREE_DATE_COLUMN,
		NULL);
	gtk_tree_view_append_column(GTK_TREE_VIEW(tree_view),
		GTK_TREE_VIEW_COLUMN(column));
	gtk_tree_view_column_set_min_width(GTK_TREE_VIEW_COLUMN(column), 160);

	/* Description column */
	cell = gtk_cell_renderer_text_new ();
	column = gtk_tree_view_column_new_with_attributes (
		_("Description"), cell,
		"text", SPP_ESTIMATE_TREE_DESC_COLUMN,
		NULL);
	gtk_tree_view_append_column(GTK_TREE_VIEW(tree_view),
		GTK_TREE_VIEW_COLUMN(column));
	gtk_tree_view_column_set_min_width(column, 300);
	gtk_tree_view_column_set_expand(GTK_TREE_VIEW_COLUMN(column), TRUE);

	/* Debit column */
	cell = gtk_cell_renderer_text_new ();
	column = gtk_tree_view_column_new_with_attributes (
		_("Debit"), cell,
		"text", SPP_ESTIMATE_TREE_DEBIT_COLUMN,
		NULL);
	gtk_tree_view_append_column(GTK_TREE_VIEW(tree_view),
		GTK_TREE_VIEW_COLUMN(column));
	gtk_tree_view_column_set_min_width(column, 150);
	g_object_set(G_OBJECT(GTK_CELL_RENDERER(cell)), "xalign", 1.0, NULL );
	gtk_tree_view_column_set_alignment(column, 1);
	gtk_tree_view_column_set_expand(GTK_TREE_VIEW_COLUMN(column), FALSE);

	/* Credit column */
	cell = gtk_cell_renderer_text_new ();
	g_object_set(G_OBJECT(GTK_CELL_RENDERER(cell)), "xalign", 1, NULL );
	column = gtk_tree_view_column_new_with_attributes (
		_("Credit"), cell,
		"text", SPP_ESTIMATE_TREE_CREDIT_COLUMN,
		NULL);
	gtk_tree_view_append_column(GTK_TREE_VIEW(tree_view),
		GTK_TREE_VIEW_COLUMN(column));
	gtk_tree_view_column_set_min_width(column, 150);
	g_object_set(G_OBJECT(GTK_CELL_RENDERER(cell)), "xalign", 1.0, NULL );
	gtk_tree_view_column_set_alignment(column, 1);
	gtk_tree_view_column_set_expand(GTK_TREE_VIEW_COLUMN(column), FALSE);

	/* Average column */
	cell = gtk_cell_renderer_text_new ();
	column = gtk_tree_view_column_new_with_attributes (
		_("Average"), cell,
		"text", SPP_ESTIMATE_TREE_BALANCE_COLUMN,
		NULL);
	gtk_tree_view_append_column(GTK_TREE_VIEW(tree_view),
		GTK_TREE_VIEW_COLUMN(column));
	gtk_tree_view_column_set_min_width(column, 170);
	g_object_set(G_OBJECT(GTK_CELL_RENDERER(cell)), "xalign", 1.0, NULL );
	gtk_tree_view_column_set_alignment(column, 1);
	gtk_tree_view_column_set_expand(GTK_TREE_VIEW_COLUMN(column), FALSE);

	/****** Estimation graph page ******/
	widget = gtk_label_new(_("Estimation graph"));
	gtk_widget_show(widget);
	vbox = gtk_vbox_new(FALSE, 5);
	gtk_widget_show(GTK_WIDGET(vbox));
	gtk_notebook_append_page(GTK_NOTEBOOK(notebook),
		GTK_WIDGET(vbox), GTK_WIDGET(widget));

	/* create the title */
	widget = gtk_label_new("Estimation graph");
	gtk_widget_show(GTK_WIDGET(widget));
	gtk_box_pack_start(GTK_BOX(vbox), GTK_WIDGET(widget), FALSE, FALSE, 5);
    	g_object_set_data (G_OBJECT(notebook), "bet_graph_title", widget);

	widget = gtk_curve_new();
	gtk_widget_show(GTK_WIDGET(widget));
	gtk_box_pack_start(GTK_BOX(vbox),
		GTK_WIDGET(widget), TRUE, TRUE, 5);
    	g_object_set_data (G_OBJECT(notebook), "bet_graph_curve", widget);

	return notebook;
}

/*
 * bet_update_balance_estimate_tab
 *
 * This function is called each time that "Balance estimate" is selected in the selection tree.
 */
void bet_update_balance_estimate_tab(void)
{
	/* find the selected account */
	GtkTreeIter iter;
	gchar* previous_account_name = NULL;
    	GtkWidget *tree_view = g_object_get_data (G_OBJECT(bet_container), "bet_account_treeview");
	GtkTreeModel *tree_model = gtk_tree_view_get_model(GTK_TREE_VIEW(tree_view));
	GtkTreeSelection* tree_selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(tree_view));
	if (gtk_tree_selection_get_selected(GTK_TREE_SELECTION(tree_selection), &tree_model, &iter))
		gtk_tree_model_get (tree_model, &iter, 0, &previous_account_name, -1);

	/* fill the account list */
	gtk_tree_store_clear ( GTK_TREE_STORE (tree_model) );
	GSList *tmp_list = gsb_data_account_get_list_accounts ();
	while ( tmp_list )
	{
		gint i = gsb_data_account_get_no_account(tmp_list->data);
		tmp_list = tmp_list->next;
		if (gsb_data_account_get_closed_account(i))
			continue;
		gchar* account_name = gsb_data_account_get_name(i);
		gtk_tree_store_append (GTK_TREE_STORE(tree_model), &iter, NULL);
		gtk_tree_store_set(GTK_TREE_STORE(tree_model), &iter,
			SPP_ACCOUNT_TREE_NAME_COLUMN, account_name, -1);
		/* select the previous selected account */
		if (previous_account_name && strcmp(account_name, previous_account_name) == 0)
			gtk_tree_selection_select_iter(GTK_TREE_SELECTION(tree_selection), &iter);
	}
	g_free(previous_account_name);

	/* if no row is selected, select the first line (if it exists) of the account list */
	if (!gtk_tree_selection_get_selected(GTK_TREE_SELECTION(tree_selection), &tree_model, &iter))
	{
		gtk_tree_model_get_iter_first(GTK_TREE_MODEL(tree_model), &iter);
		gtk_tree_selection_select_iter(GTK_TREE_SELECTION(tree_selection), &iter);
	}

	/* update the estimate array and graph (if account list is not empty) */
	/* TODO clear the graph and the account list if no account is selected */
	if (gtk_tree_selection_get_selected(GTK_TREE_SELECTION(tree_selection), &tree_model, &iter))
		bet_estimate_refresh();
}

/*
 * bet_date_sort_function
 * This function is called by the Tree Model to sort
 * two lines by date.
 */
static gint bet_date_sort_function (GtkTreeModel *model, GtkTreeIter *itera, GtkTreeIter *iterb, gpointer user_data)
{
	g_assert(itera != NULL && iterb != NULL);

	/* get first date to compare */
	GValue date_value_a = {0,};
	gtk_tree_model_get_value(GTK_TREE_MODEL(model), itera,
		SPP_ESTIMATE_TREE_SORT_DATE_COLUMN,
		&date_value_a);
	GDate* datea = g_value_get_boxed(&date_value_a);
	g_assert(datea != NULL);
	g_assert(g_date_valid(datea));

	/* get second date to compare */
	GValue date_value_b = {0,};
	gtk_tree_model_get_value(GTK_TREE_MODEL(model), iterb,
		SPP_ESTIMATE_TREE_SORT_DATE_COLUMN,
		&date_value_b);
	GDate* dateb = g_value_get_boxed(&date_value_b);
	g_assert(dateb != NULL);
	g_assert(g_date_valid(dateb));

	gint result = g_date_compare(dateb, datea);

	g_value_unset(&date_value_b);
	g_value_unset(&date_value_a);

	return result;
}

/*
 * bet_update_average_column
 *
 * This function is called for each line of the array.
 * It calculates the balance column by adding the amount of the line
 * to the balance of the previous line.
 * It calculates the minimum and the maximum values of the balance column.
 */
static gboolean bet_update_average_column (GtkTreeModel *model, GtkTreePath *path, GtkTreeIter *iter, gpointer data)
{
	struct bet_range* tmp_range = (struct bet_range*)data;

	gchar* date = NULL;
	gtk_tree_model_get(model, iter, SPP_ESTIMATE_TREE_DATE_COLUMN,
		&date, -1);

	gchar* str = NULL;
	gsb_real null_real = { 0 , 0 };
	gsb_real amount = { 0 , 0 };
	gtk_tree_model_get(model, iter, SPP_ESTIMATE_TREE_CREDIT_COLUMN, &str, -1);
	if (str) {
		amount = gsb_real_get_from_string(str);
	} else {
		gtk_tree_model_get(model, iter, SPP_ESTIMATE_TREE_DEBIT_COLUMN, &str, -1);
		amount = gsb_real_get_from_string(str);
		amount = gsb_real_sub(null_real, amount);
	}


	tmp_range->current_balance = gsb_real_add(tmp_range->current_balance, amount);
	gchar* str_balance = gsb_real_get_string(tmp_range->current_balance);
	gtk_tree_store_set(GTK_TREE_STORE(model), iter,
		SPP_ESTIMATE_TREE_BALANCE_COLUMN, str_balance,
		-1);
	g_free(str_balance);

	if (gsb_real_cmp(tmp_range->min_balance, tmp_range->current_balance) > 0)
		tmp_range->min_balance = tmp_range->current_balance;
	if (gsb_real_cmp(tmp_range->max_balance, tmp_range->current_balance) < 0)
		tmp_range->max_balance = tmp_range->current_balance;

	return FALSE;
}

/*
 * bet_update_graph
 * This function is called for each line of the estimate array and it updates
 * the graph.
 */
static gboolean bet_update_graph (GtkTreeModel *model, GtkTreePath *path, GtkTreeIter *iter, gpointer data)
{
	/* get balance */
	gsb_real balance;
	gchar* balance_str = NULL;
	gtk_tree_model_get(model, iter, SPP_ESTIMATE_TREE_BALANCE_COLUMN, &balance_str, -1);
	balance = gsb_real_get_from_string(balance_str);
	g_free(balance_str);

	/* get date */
	GValue date_value = {0,};
	gtk_tree_model_get_value(GTK_TREE_MODEL(model), iter,
		SPP_ESTIMATE_TREE_SORT_DATE_COLUMN,
		&date_value);
		/*
	GDate* date = g_value_get_boxed(&date_value);
	guint32 dt = g_date_get_julian(date);
	*/
	g_value_unset(&date_value);

	return FALSE;
}


/*
 * bet_estimate_refresh
 * This function clears the estimate array and calculates new estimates.
 * It updates the estimate graph.
 * This function is called when the refresh button is pressed and when
 * the balance estimate tab is selected.
 */
static void bet_estimate_refresh()
{
	/* find the selected account */
	GtkTreeIter iter;
    	GtkWidget *tree_view = g_object_get_data (G_OBJECT(bet_container), "bet_account_treeview");
	GtkTreeModel *tree_model = gtk_tree_view_get_model(GTK_TREE_VIEW(tree_view));
	GtkTreeSelection* tree_selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(tree_view));
	if (!gtk_tree_selection_get_selected(GTK_TREE_SELECTION(tree_selection), &tree_model, &iter))
	{
		GtkWidget* dialog = gtk_message_dialog_new (GTK_WINDOW(window),
			GTK_DIALOG_DESTROY_WITH_PARENT,
			GTK_MESSAGE_ERROR,
			GTK_BUTTONS_CLOSE,
			"Please, select an account in the list !\n");
		gtk_dialog_run(GTK_DIALOG(dialog));
		gtk_widget_destroy(GTK_WIDGET(dialog));
		return;
	}
	gchar* account_name = NULL;
	gtk_tree_model_get (tree_model, &iter, 0, &account_name, -1);
	gint selected_account = gsb_data_account_get_no_account_by_name(account_name);
	g_assert(selected_account != -1);

	/* calculate date_max with user choice */
	gchar* data = g_object_get_data(G_OBJECT(bet_container), "bet_months");
    	gint months = (data) ? GPOINTER_TO_INT(data): 1;
	GDate *date_min = gdate_today();
	GDate *date_max = gdate_today();
	g_date_add_months(date_max, months);

	/* set the graph title and the array title */
	GtkWidget *widget;
	gchar* str_date_min = gsb_format_gdate(date_min);
	gchar* str_date_max = gsb_format_gdate(date_max);
	gchar* title = g_strdup_printf(_("Balance estimate of the account \"%s\" from %s to %s"),
		account_name, str_date_min, str_date_max);
	g_free(str_date_min);
	g_free(str_date_max);
    	widget = GTK_WIDGET(g_object_get_data(G_OBJECT(bet_container), "bet_array_title"));
	gtk_label_set_label(GTK_LABEL(widget), title);
    	widget = GTK_WIDGET(g_object_get_data(G_OBJECT(bet_container), "bet_graph_title"));
	gtk_label_set_label(GTK_LABEL(widget), title);
	g_free(title);
	g_free(account_name);

	/* clear tree view */
    	tree_view = g_object_get_data (G_OBJECT(bet_container), "bet_estimate_treeview");
	tree_model = gtk_tree_view_get_model(GTK_TREE_VIEW(tree_view));
	gtk_tree_store_clear(GTK_TREE_STORE (tree_model));

	/* current balance may be in the future if there are transactions
	 * in the future in the account. So we need to calculate the balance
	 * of today */
	gsb_real current_balance = gsb_data_account_get_current_balance(selected_account);

	/* search transactions of the account which are in the future */
	GSList* tmp_list = gsb_data_transaction_get_complete_transactions_list ();
	while (tmp_list)
	{
		gint transaction_number = gsb_data_transaction_get_transaction_number (tmp_list->data);
		tmp_list = tmp_list -> next;
		gint account_number =  gsb_data_transaction_get_account_number (transaction_number);
		if (account_number != selected_account)
			continue;

		/* ignore transaction which are before date_min (today) */
		GDate *date = gsb_data_transaction_get_date(transaction_number);
		if (g_date_compare(date, date_min) <= 0)
			continue;

		/* update the current_balance.
		 * our target is to calculate the balance of date_min (today) */
		gsb_real amount = gsb_data_transaction_get_amount(transaction_number);
		current_balance = gsb_real_sub(current_balance, amount);

		/* ignore transaction which are after date_max */
		if (g_date_compare(date, date_max) > 0)
			continue;

		gchar* str_value = gsb_real_get_string(amount);
		gchar* str_debit = NULL;
		gchar* str_credit = NULL;
		if (amount.mantissa < 0)
			str_debit = str_value;
		else
			str_credit = str_value;

		/* TODO add something else if description is empty (payee by example) */
		const gchar* str_description = gsb_data_transaction_get_notes(transaction_number);
		gchar* str_date = gsb_format_gdate(date);

		GValue date_value = {0, };
		g_value_init (&date_value, G_TYPE_DATE);
		g_value_set_boxed(&date_value, date);

		/* add a line in the estimate array */
		gtk_tree_store_append (GTK_TREE_STORE(tree_model), &iter, NULL);
		gtk_tree_store_set_value(GTK_TREE_STORE(tree_model), &iter,
			SPP_ESTIMATE_TREE_SORT_DATE_COLUMN,
			&date_value);
		gtk_tree_store_set(GTK_TREE_STORE(tree_model), &iter,
			SPP_ESTIMATE_TREE_DATE_COLUMN, str_date,
			SPP_ESTIMATE_TREE_DESC_COLUMN, str_description,
			SPP_ESTIMATE_TREE_DEBIT_COLUMN, str_debit,
			SPP_ESTIMATE_TREE_CREDIT_COLUMN, str_credit,
			-1);

		g_value_unset(&date_value);
		g_free(str_date);
		g_free(str_value);
	}

	/* for each schedulded operation */
	tmp_list = gsb_data_scheduled_get_scheduled_list();
	while (tmp_list)
	{
		gint scheduled_number = gsb_data_scheduled_get_scheduled_number(tmp_list->data);
		tmp_list = tmp_list->next;

		/* ignore children scheduled operations */
		if (gsb_data_scheduled_get_mother_scheduled_number(scheduled_number))
			continue;

		/* ignore scheduled operations of other account */
		gint account_number = gsb_data_scheduled_get_account_number(scheduled_number);
		gint transfer_account_number =
			gsb_data_scheduled_get_account_number_transfer(scheduled_number);
		if (account_number != selected_account &&
			transfer_account_number != selected_account)
			continue;

		gchar* str_description = gsb_data_scheduled_get_notes(scheduled_number);
		gsb_real amount = gsb_data_scheduled_get_amount(scheduled_number);

		gchar* str_value = gsb_real_get_string(amount);
		gchar* str_debit = NULL;
		gchar* str_credit = NULL;
		if (amount.mantissa < 0)
			str_debit = str_value;
		else
			str_credit = str_value;

		/* with transfer transactions, reverse debit and credit */
		if (transfer_account_number != -1)
		{
			str_value = str_debit;
			str_debit = str_credit;
			str_credit = str_value;
		}

		/* calculate each instance of the scheduled operation
		 * in the range from date_min (today) to date_max */
		GDate *date = gsb_data_scheduled_get_date(scheduled_number);
		while (date != NULL && g_date_valid(date))
		{
			if (g_date_compare(date, date_max) > 0)
				break;
			if (g_date_compare(date, date_min) < 0) {
				date = gsb_scheduler_get_next_date(scheduled_number, date);
				continue;
			}
			g_assert(g_date_valid (date));
			gchar* str_date = gsb_format_gdate(date);

			GValue date_value = {0, };
			g_value_init (&date_value, G_TYPE_DATE);
			g_assert(date != NULL);
			g_value_set_boxed(&date_value, date);

			/* add a line in the estimate array */
			gtk_tree_store_append (GTK_TREE_STORE(tree_model), &iter, NULL);
			gtk_tree_store_set_value(GTK_TREE_STORE(tree_model), &iter,
				SPP_ESTIMATE_TREE_SORT_DATE_COLUMN,
				&date_value);
			gtk_tree_store_set(GTK_TREE_STORE(tree_model), &iter,
				/*SPP_ESTIMATE_TREE_SORT_DATE_COLUMN, &date_value,*/
				SPP_ESTIMATE_TREE_DATE_COLUMN, str_date,
				SPP_ESTIMATE_TREE_DESC_COLUMN, str_description,
				SPP_ESTIMATE_TREE_DEBIT_COLUMN, str_debit,
				SPP_ESTIMATE_TREE_CREDIT_COLUMN, str_credit,
				-1);

			g_value_unset(&date_value);
			g_free(str_date);
			date = gsb_scheduler_get_next_date(scheduled_number, date);
		}
		g_free(str_value);
	}
	g_free(date_min);
	g_free(date_max);

	/* Calculate the balance column */
	struct bet_range tmp_range;
	tmp_range.min_date = *date_min;
	tmp_range.max_date = *date_max;
	gsb_real null_real = { 0 , 0 };
	tmp_range.min_balance = null_real;
	tmp_range.max_balance = null_real;
	tmp_range.current_balance = current_balance;
	gtk_tree_model_foreach(GTK_TREE_MODEL(tree_model),
		bet_update_average_column, &tmp_range);

	/* update graph */
    	widget = g_object_get_data (G_OBJECT(bet_container), "bet_graph_curve");
	gtk_curve_reset(GTK_CURVE(widget));
	gtk_tree_model_foreach(GTK_TREE_MODEL(tree_model),
		bet_update_graph, widget);

}

#endif /* ENABLE_BALANCE_ESTIMATE */
