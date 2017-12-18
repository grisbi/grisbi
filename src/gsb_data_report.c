/* ************************************************************************** */
/*                                                                            */
/*                                  gsb_data_report                           */
/*                                                                            */
/*     Copyright (C)    2000-2008 Cédric Auger (cedric@grisbi.org)            */
/*          2003-2008 Benjamin Drieu (bdrieu@april.org)                       */
/*          2008-2013 Pierre Biava (grisbi@pierre.biava.name)                 */
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

/**
 * \file gsb_data_report.c
 * work with the report structure, no GUI here
 */


#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "include.h"
#include <string.h>

/*START_INCLUDE*/
#include "gsb_data_report.h"
#include "gsb_data_archive.h"
#include "gsb_data_fyear.h"
#include "gsb_data_report_amout_comparison.h"
#include "gsb_data_report_text_comparison.h"
#include "utils_dates.h"
#include "utils_str.h"
#include "erreur.h"
/*END_INCLUDE*/

/** \ReportStruct
 * describe an report
 * */
typedef struct _ReportStruct	ReportStruct;

struct _ReportStruct {
    /** @name general stuff */
    gint report_number;
    gchar *report_name;
	gboolean compl_name_used;						/* TRUE si utilisation d'un complément au nom du rapport */
	gint compl_name_function;						/* Date ou Date + heure système */
	gint compl_name_position;						/* Devant, derrière ou dessous le nom du rapport */
	gchar *export_pdf_name;							/* Nom du fichier pour l'export pdf */

    /** @name what we show of the transactions */
    gint show_m;                                    /**< 0=all the reports, 1=report not marked R, 2=report marked P,R or T */
    gint show_p;                                    /**< 0=report not marked P, 1=report marked P */
    gint show_r;                                    /**< 0=report not marked R, 1=report marked R */
    gint show_t;                                    /**< 0=report not marked T, 1=report marked T */
    gint show_report_transactions;
    gint show_report_transaction_amount;
    gint show_report_date;
    gint show_report_value_date;
    gint show_report_payee;
    gint show_report_category;
    gint show_report_sub_category;
    gint show_report_budget;
    gint show_report_sub_budget;
    gint show_report_note;
    gint show_report_voucher;
    gint show_report_bank_references;
    gint show_report_transaction_number;
    gint show_report_method_of_payment;
    gint show_report_method_of_payment_content;
    gint show_report_marked;
    gint show_report_financial_year;

    /** @name stuff showed in the report */
    gint sorting_report;                            /**< 0=date, 1=tr number, 2=payee, 3=categ, 4=budget, 5=notes, 6=method payment, 7=method paym content, 8=voucher, 9=bank ref, 10=marked number */

    gint not_detail_split;
    gint split_credit_debit;

    gint currency_general;
    gint column_title_show;
    gint column_title_type;                         /* 0 = botton, 1 = each section */
    gint append_in_payee;                           /* TRUE : the name of the report will be in the payee list */
    gint report_can_click;                          /* TRUE : we can click on the reports */
    gint ignore_archives;                           /* TRUE ignore les opérations archivées choix manuel */


    /** @name period part of the report */
    /** exercices */
    gint use_financial_year;                        /* TRUE : use the financial year, FALSE : use the dates */
    gint financial_year_type;                       /* 0=all, 1=current, 2=last, 3=personnal */
    GSList *financial_year_list;                    /* list of the numbers of financials years used */
    gint financial_year_split;                      /* TRUE : split by financial year */

    /** dates */
    gint date_type;                                 /* 0=all, 1=perso ... */
    gint date_select_value;                         /* 0=date (default), 1=value date */
    GDate *personal_date_start;
    GDate *personal_date_end;

    /** affichage */
    gint period_split;                              /* TRUE : split by period */
    gint period_split_type;                         /* 0=day, 1=week, 2=month, 3=year */
    gint period_split_day;                          /* 0 = monday ... */

    /** à compléter  */
    GSList *sorting_type;                           /* list of numbers : 1=categ,2=sub-categ,3=budget,4=sub-budget,5=account,6=payee */

    /** @name account part of the report */
    gint account_use_chosen;
    GSList *account_numbers;
    gint account_group_reports;
    gint account_show_amount;
    gint account_show_name;

    /** @name transfer part of the report */
    gint transfer_choice;                           /* 0: no transfer / 1: transfers only on liabilities and assets accounts/2:transfer outside the report/3:perso */
    GSList *transfer_account_numbers;
    gint transfer_reports_only;

    /** @name category part of the report */
    gint category_used;
    gint category_detail_used;
    GSList *categ_select_struct;                    /* list of CategBudgetSel containing the selected categories and sub-categories */
    gint category_show_sub_category;
    gint category_show_category_amount;
    gint category_show_sub_category_amount;
    gint category_currency;
    gint category_show_without_category;
    gint category_show_name;

    /** @name budget part of the report */
    gint budget_used;
    gint budget_detail_used;
    GSList *budget_select_struct;                   /* list of CategBudgetSel containing the selected budgets and sub-budgets */
    gint budget_show_sub_budget;
    gint budget_show_budget_amount;
    gint budget_show_sub_budget_amount;
    gint budget_currency;
    gint budget_show_without_budget;
    gint budget_show_name;

    /** @name payee part of the report */
    gint payee_used;
    gint payee_detail_used;
    GSList *payee_numbers;
    gint payee_show_payee_amount;
    gint payee_currency;
    gint payee_show_name;

    /** @name text comparison part of the report */
    gint text_comparison_used;
    GSList *text_comparison_list;

    /** @name amount comparison part of the report */
    gint amount_comparison_used;
    gint amount_comparison_currency;
    GSList *amount_comparison_list;
    gint amount_comparison_only_report_non_null;

    /** @name method of payment part of the report */
    gint method_of_payment_used;
    GSList *method_of_payment_list;
};



/*START_STATIC*/
static void _gsb_data_report_free ( ReportStruct *report );
static GSList *gsb_data_report_copy_categ_budget_struct (GSList *orig_categ_budget_list);
static ReportStruct *gsb_data_report_get_structure ( gint report_number );
/*END_STATIC*/

/*START_EXTERN*/
/*END_EXTERN*/



/** contains a g_slist of ReportStruct */
static GSList *report_list = NULL;

/** a pointers to the last report used (to increase the speed) */
static ReportStruct *report_buffer;




/**
 * set the reports global variables to NULL, usually when we init all the global variables
 *
 * \param none
 *
 * \return FALSE
 * */
gboolean gsb_data_report_init_variables ( void )
{
    /* free memory used by report_list */
    if ( report_list )
    {
        GSList* tmp_list = report_list;
        while ( tmp_list )
        {
	    ReportStruct *report = tmp_list -> data;
	    tmp_list = tmp_list -> next;
	    _gsb_data_report_free ( report );
	}
    }
    g_slist_free ( report_list );
    report_list = NULL;
    return FALSE;
}


/**
 * return a pointer on the g_slist of reports
 * carrefull : it's not a copy, so we must not free or change it
 *
 * \param none
 *
 * \return a g_slist on the reports
 * */
GSList *gsb_data_report_get_report_list ( void )
{
    return report_list;
}




/**
 * return a pointer on the report which the number is in the parameter.
 * that report is stored in the buffer
 *
 * \param report_number
 *
 * \return a pointer to the report, NULL if not found
 * */
ReportStruct *gsb_data_report_get_structure ( gint report_number )
{
    GSList *tmp_list;

    /* check first if the report is in the buffer */

    if ( report_buffer
	 &&
	 report_buffer -> report_number == report_number )
	return (gpointer) report_buffer;

    tmp_list = report_list;

    while ( tmp_list )
    {
	ReportStruct *report;

	report = tmp_list -> data;

	if ( report -> report_number == report_number )
	{
	    report_buffer = report;
	    return report;
	}

	tmp_list = tmp_list -> next;
    }

    /* here, we didn't find any report with that number */

    return NULL;
}



/**
 * get the number of the report and save the pointer in the buffer
 * which will increase the speed later
 *
 * \param report_pointer a pointer to a report
 *
 * \return the number of the report
 * */
gint gsb_data_report_get_report_number ( gpointer report_pointer )
{
    ReportStruct *report;

    report = report_pointer;

    if ( !report )
	return 0;

    /* if we want the report number, usually it's to make other stuff after that
     * so we will save the adr of the report to increase the speed after */

    report_buffer = report;

    return report -> report_number;
}


/** find and return the last number of report
 *
 * \param none
 *
 * \return last number of report
 * */
gint gsb_data_report_max_number ( void )
{
    GSList *tmp;
    gint number_tmp = 0;

    tmp = report_list;

    while ( tmp )
    {
	ReportStruct *report;

	report = tmp -> data;

	if ( report -> report_number > number_tmp )
	    number_tmp = report -> report_number;

	tmp = tmp -> next;
    }
    return number_tmp;
}



/**
 * create a new report, give it a number, append it to the list
 * and return the number
 *
 * \param name the name of the report (can be freed after, it's a copy) or NULL
 *
 * \return the number of the new report
 * */
gint gsb_data_report_new ( gchar *name )
{
    gint report_number;

    /* create the new report with a new number */

    report_number = gsb_data_report_new_with_number ( gsb_data_report_max_number () + 1 );

    /* append the name if necessary */

    if (name)
	gsb_data_report_set_report_name ( report_number,
					  name );

    return report_number;
}


/**
 * create a new report with a number, append it to the list
 * and return the number
 *
 * \param number the number we want to give to that report
 *
 * \return the number of the new report
 * */
gint gsb_data_report_new_with_number ( gint number )
{
    ReportStruct *report;

    report = g_malloc0 ( sizeof ( ReportStruct ));
    report -> report_number = number;

    report_list = g_slist_append ( report_list,
				   report );

    report_buffer = report;

    return report -> report_number;
}

/**
 * This internal function is called to free the memory used by a ReportStruct structure
 */
static void _gsb_data_report_free ( ReportStruct *report )
{
	GSList* list_tmp;

    if ( ! report )
        return ;

    /* remove first the comparison */
    list_tmp = report -> text_comparison_list;
    while ( list_tmp )
    {
	gsb_data_report_text_comparison_remove ( GPOINTER_TO_INT (list_tmp -> data));
	list_tmp = list_tmp -> next;
    }

    list_tmp = report -> amount_comparison_list;
    while ( list_tmp )
    {
	gsb_data_report_amount_comparison_remove ( GPOINTER_TO_INT (list_tmp -> data));
	list_tmp = list_tmp -> next;
    }

    g_slist_free (report -> financial_year_list);
    g_slist_free (report -> sorting_type);
    g_slist_free (report -> account_numbers);
    g_slist_free (report -> transfer_account_numbers);
    gsb_data_report_free_categ_budget_struct_list (report -> categ_select_struct);
    report -> categ_select_struct = NULL;
    gsb_data_report_free_categ_budget_struct_list (report -> budget_select_struct);
    report -> budget_select_struct = NULL;
    g_slist_free (report -> payee_numbers);
    g_slist_free (report -> method_of_payment_list);

    if ( report -> report_name )
        g_free ( report -> report_name );
    if ( report->export_pdf_name)
        g_free ( report->export_pdf_name);
    if ( report -> personal_date_start )
        g_date_free ( report -> personal_date_start );
    if ( report -> personal_date_end )
        g_date_free ( report -> personal_date_end );

    g_free (report);
    if ( report_buffer == report )
	report_buffer = NULL;
}

/**
 * remove a report
 *
 * \param no_report the report we want to remove
 *
 * \return TRUE ok
 * */
gboolean gsb_data_report_remove ( gint no_report )
{
    ReportStruct *report;
    report = gsb_data_report_get_structure ( no_report );

    if (!report)
	return FALSE;

    report_list = g_slist_remove ( report_list,
				   report );

    _gsb_data_report_free ( report );
    return TRUE;
}


/**
 * return the number of the report found by its name
 * that report is stored in the buffer
 *
 * \param name
 *
 * \return the number of the report or 0 if not found
 * */
gint gsb_data_report_get_report_by_name ( const gchar *name )
{
    GSList *tmp_list;

    if (!name)
	return 0;

    /* check first if the report is in the buffer */
    if ( report_buffer
	 &&
	 !strcmp ( report_buffer -> report_name,
		   name ))
	return report_buffer -> report_number;

    tmp_list = report_list;

    while ( tmp_list )
    {
	ReportStruct *report;

	report = tmp_list -> data;

	if ( !strcmp ( report -> report_name,
		       name ))
	{
	    report_buffer = report;
	    return report -> report_number;
	}
	tmp_list = tmp_list -> next;
    }
    /* here, we didn't find any report with that name */
    return 0;
}


/**
 * get the  report_name
 *
 * \param report_number the number of the report
 *
 * \return the report_name  of the report, -1 if problem
 * */
gchar *gsb_data_report_get_report_name ( gint report_number )
{
    ReportStruct *report;

    report = gsb_data_report_get_structure (report_number);

    if ( !report )
	return NULL;

    return report -> report_name;
}

/**
 * set the report_name
 *
 * \param report_number number of the report
 * \param report_name
 *
 * \return TRUE if ok
 * */
gboolean gsb_data_report_set_report_name ( gint report_number,
                        const gchar *report_name )
{
    ReportStruct *report;

    report = gsb_data_report_get_structure (report_number);

    if ( !report )
	return FALSE;

    if (report_name)
	report -> report_name = my_strdup (report_name);
    else
	report -> report_name = NULL;

    return TRUE;
}


/**
 * get the  show_report_transactions
 *
 * \param report_number the number of the report
 *
 * \return the show_report_transactions  of the report, -1 if problem
 * */
gint gsb_data_report_get_show_report_transactions ( gint report_number )
{
    ReportStruct *report;

    report = gsb_data_report_get_structure (report_number);

    if ( !report )
	return -1;

    return report -> show_report_transactions;
}

/**
 * set the show_report_transactions
 *
 * \param report_number number of the report
 * \param show_report_transactions
 *
 * \return TRUE if ok
 * */
gboolean gsb_data_report_set_show_report_transactions ( gint report_number,
                        gint show_report_transactions )
{
    ReportStruct *report;

    report = gsb_data_report_get_structure (report_number);

    if ( !report )
	return FALSE;

    report -> show_report_transactions = show_report_transactions;

    return TRUE;
}


/**
 * get the  show_report_transaction_amount
 *
 * \param report_number the number of the report
 *
 * \return the show_report_transaction_amount  of the report, -1 if problem
 * */
gint gsb_data_report_get_show_report_transaction_amount ( gint report_number )
{
    ReportStruct *report;

    report = gsb_data_report_get_structure (report_number);

    if ( !report )
	return -1;

    return report -> show_report_transaction_amount;
}

/**
 * set the show_report_transaction_amount
 *
 * \param report_number number of the report
 * \param show_report_transaction_amount
 *
 * \return TRUE if ok
 * */
gboolean gsb_data_report_set_show_report_transaction_amount ( gint report_number,
                        gint show_report_transaction_amount )
{
    ReportStruct *report;

    report = gsb_data_report_get_structure (report_number);

    if ( !report )
	return FALSE;

    report -> show_report_transaction_amount = show_report_transaction_amount;

    return TRUE;
}


/**
 * get the  show_report_date
 *
 * \param report_number the number of the report
 *
 * \return the show_report_date  of the report, -1 if problem
 * */
gint gsb_data_report_get_show_report_date ( gint report_number )
{
    ReportStruct *report;

    report = gsb_data_report_get_structure (report_number);

    if ( !report )
	return -1;

    return report -> show_report_date;
}

/**
 * set the show_report_date
 *
 * \param report_number number of the report
 * \param show_report_date
 *
 * \return TRUE if ok
 * */
gboolean gsb_data_report_set_show_report_date ( gint report_number,
                        gint show_report_date )
{
    ReportStruct *report;

    report = gsb_data_report_get_structure (report_number);

    if ( !report )
	return FALSE;

    report -> show_report_date = show_report_date;

    return TRUE;
}

/**
 * get the  show_report_value_date
 *
 * \param report_number the number of the report
 *
 * \return the show_report_value_date  of the report, -1 if problem
 * */
gint gsb_data_report_get_show_report_value_date ( gint report_number )
{
    ReportStruct *report;

    report = gsb_data_report_get_structure (report_number);

    if ( !report )
	return -1;

    return report -> show_report_value_date;
}

/**
 * set the show_report_value_date
 *
 * \param report_number number of the report
 * \param show_report_value_date
 *
 * \return TRUE if ok
 * */
gboolean gsb_data_report_set_show_report_value_date ( gint report_number,
                        gint show_report_value_date )
{
    ReportStruct *report;

    report = gsb_data_report_get_structure (report_number);

    if ( !report )
	return FALSE;

    report -> show_report_value_date = show_report_value_date;

    return TRUE;
}


/**
 * get the  show_report_payee
 *
 * \param report_number the number of the report
 *
 * \return the  show_report_payee of the report, -1 if problem
 * */
gint gsb_data_report_get_show_report_payee ( gint report_number )
{
    ReportStruct *report;

    report = gsb_data_report_get_structure (report_number);

    if ( !report )
	return -1;

    return report -> show_report_payee;
}

/**
 * set the show_report_payee
 *
 * \param report_number number of the report
 * \param show_report_payee
 *
 * \return TRUE if ok
 * */
gboolean gsb_data_report_set_show_report_payee ( gint report_number,
                        gint show_report_payee )
{
    ReportStruct *report;

    report = gsb_data_report_get_structure (report_number);

    if ( !report )
	return FALSE;

    report -> show_report_payee = show_report_payee;

    return TRUE;
}


/**
 * get the  show_report_category
 *
 * \param report_number the number of the report
 *
 * \return the show_report_category  of the report, -1 if problem
 * */
gint gsb_data_report_get_show_report_category ( gint report_number )
{
    ReportStruct *report;

    report = gsb_data_report_get_structure (report_number);

    if ( !report )
	return -1;

    return report -> show_report_category;
}

/**
 * set the show_report_category
 *
 * \param report_number number of the report
 * \param show_report_category
 *
 * \return TRUE if ok
 * */
gboolean gsb_data_report_set_show_report_category ( gint report_number,
                        gint show_report_category )
{
    ReportStruct *report;

    report = gsb_data_report_get_structure (report_number);

    if ( !report )
	return FALSE;

    report -> show_report_category = show_report_category;

    return TRUE;
}


/**
 * get the  show_report_sub_category
 *
 * \param report_number the number of the report
 *
 * \return the show_report_sub_category  of the report, -1 if problem
 * */
gint gsb_data_report_get_show_report_sub_category ( gint report_number )
{
    ReportStruct *report;

    report = gsb_data_report_get_structure (report_number);

    if ( !report )
	return -1;

    return report -> show_report_sub_category;
}

/**
 * set the show_report_sub_category
 *
 * \param report_number number of the report
 * \param show_report_sub_category
 *
 * \return TRUE if ok
 * */
gboolean gsb_data_report_set_show_report_sub_category ( gint report_number,
                        gint show_report_sub_category )
{
    ReportStruct *report;

    report = gsb_data_report_get_structure (report_number);

    if ( !report )
	return FALSE;

    report -> show_report_sub_category = show_report_sub_category;

    return TRUE;
}


/**
 * get the  show_report_budget
 *
 * \param report_number the number of the report
 *
 * \return the show_report_budget  of the report, -1 if problem
 * */
gint gsb_data_report_get_show_report_budget ( gint report_number )
{
    ReportStruct *report;

    report = gsb_data_report_get_structure (report_number);

    if ( !report )
	return -1;

    return report -> show_report_budget;
}

/**
 * set the show_report_budget
 *
 * \param report_number number of the report
 * \param show_report_budget
 *
 * \return TRUE if ok
 * */
gboolean gsb_data_report_set_show_report_budget ( gint report_number,
                        gint show_report_budget )
{
    ReportStruct *report;

    report = gsb_data_report_get_structure (report_number);

    if ( !report )
	return FALSE;

    report -> show_report_budget = show_report_budget;

    return TRUE;
}


/**
 * get the  show_report_sub_budget
 *
 * \param report_number the number of the report
 *
 * \return the  show_report_sub_budget of the report, -1 if problem
 * */
gint gsb_data_report_get_show_report_sub_budget ( gint report_number )
{
    ReportStruct *report;

    report = gsb_data_report_get_structure (report_number);

    if ( !report )
	return -1;

    return report -> show_report_sub_budget;
}

/**
 * set the show_report_sub_budget
 *
 * \param report_number number of the report
 * \param show_report_sub_budget
 *
 * \return TRUE if ok
 * */
gboolean gsb_data_report_set_show_report_sub_budget ( gint report_number,
                        gint show_report_sub_budget )
{
    ReportStruct *report;

    report = gsb_data_report_get_structure (report_number);

    if ( !report )
	return FALSE;

    report -> show_report_sub_budget = show_report_sub_budget;

    return TRUE;
}


/**
 * get the  show_report_note
 *
 * \param report_number the number of the report
 *
 * \return the show_report_note  of the report, -1 if problem
 * */
gint gsb_data_report_get_show_report_note ( gint report_number )
{
    ReportStruct *report;

    report = gsb_data_report_get_structure (report_number);

    if ( !report )
	return -1;

    return report -> show_report_note;
}

/**
 * set the show_report_note
 *
 * \param report_number number of the report
 * \param show_report_note
 *
 * \return TRUE if ok
 * */
gboolean gsb_data_report_set_show_report_note ( gint report_number,
                        gint show_report_note )
{
    ReportStruct *report;

    report = gsb_data_report_get_structure (report_number);

    if ( !report )
	return FALSE;

    report -> show_report_note = show_report_note;

    return TRUE;
}


/**
 * get the  show_report_voucher
 *
 * \param report_number the number of the report
 *
 * \return the show_report_voucher  of the report, -1 if problem
 * */
gint gsb_data_report_get_show_report_voucher ( gint report_number )
{
    ReportStruct *report;

    report = gsb_data_report_get_structure (report_number);

    if ( !report )
	return -1;

    return report -> show_report_voucher;
}

/**
 * set the show_report_voucher
 *
 * \param report_number number of the report
 * \param show_report_voucher
 *
 * \return TRUE if ok
 * */
gboolean gsb_data_report_set_show_report_voucher ( gint report_number,
                        gint show_report_voucher )
{
    ReportStruct *report;

    report = gsb_data_report_get_structure (report_number);

    if ( !report )
	return FALSE;

    report -> show_report_voucher = show_report_voucher;

    return TRUE;
}


/**
 * get the  show_report_bank_references
 *
 * \param report_number the number of the report
 *
 * \return the show_report_bank_references  of the report, -1 if problem
 * */
gint gsb_data_report_get_show_report_bank_references ( gint report_number )
{
    ReportStruct *report;

    report = gsb_data_report_get_structure (report_number);

    if ( !report )
	return -1;

    return report -> show_report_bank_references;
}

/**
 * set the show_report_bank_references
 *
 * \param report_number number of the report
 * \param show_report_bank_references
 *
 * \return TRUE if ok
 * */
gboolean gsb_data_report_set_show_report_bank_references ( gint report_number,
                        gint show_report_bank_references )
{
    ReportStruct *report;

    report = gsb_data_report_get_structure (report_number);

    if ( !report )
	return FALSE;

    report -> show_report_bank_references = show_report_bank_references;

    return TRUE;
}


/**
 * get the  show_report_transaction_number
 *
 * \param report_number the number of the report
 *
 * \return the show_report_transaction_number  of the report, -1 if problem
 * */
gint gsb_data_report_get_show_report_transaction_number ( gint report_number )
{
    ReportStruct *report;

    report = gsb_data_report_get_structure (report_number);

    if ( !report )
	return -1;

    return report -> show_report_transaction_number;
}

/**
 * set the show_report_transaction_number
 *
 * \param report_number number of the report
 * \param show_report_transaction_number
 *
 * \return TRUE if ok
 * */
gboolean gsb_data_report_set_show_report_transaction_number ( gint report_number,
                        gint show_report_transaction_number )
{
    ReportStruct *report;

    report = gsb_data_report_get_structure (report_number);

    if ( !report )
	return FALSE;

    report -> show_report_transaction_number = show_report_transaction_number;

    return TRUE;
}


/**
 * get the  show_report_method_of_payment
 *
 * \param report_number the number of the report
 *
 * \return the  show_report_method_of_payment of the report, -1 if problem
 * */
gint gsb_data_report_get_show_report_method_of_payment ( gint report_number )
{
    ReportStruct *report;

    report = gsb_data_report_get_structure (report_number);

    if ( !report )
	return -1;

    return report -> show_report_method_of_payment;
}

/**
 * set the show_report_method_of_payment
 *
 * \param report_number number of the report
 * \param show_report_method_of_payment
 *
 * \return TRUE if ok
 * */
gboolean gsb_data_report_set_show_report_method_of_payment ( gint report_number,
                        gint show_report_method_of_payment )
{
    ReportStruct *report;

    report = gsb_data_report_get_structure (report_number);

    if ( !report )
	return FALSE;

    report -> show_report_method_of_payment = show_report_method_of_payment;

    return TRUE;
}


/**
 * get the  show_report_method_of_payment_content
 *
 * \param report_number the number of the report
 *
 * \return the show_report_method_of_payment_content  of the report, -1 if problem
 * */
gint gsb_data_report_get_show_report_method_of_payment_content ( gint report_number )
{
    ReportStruct *report;

    report = gsb_data_report_get_structure (report_number);

    if ( !report )
	return -1;

    return report -> show_report_method_of_payment_content;
}

/**
 * set the show_report_method_of_payment_content
 *
 * \param report_number number of the report
 * \param show_report_method_of_payment_content
 *
 * \return TRUE if ok
 * */
gboolean gsb_data_report_set_show_report_method_of_payment_content ( gint report_number,
                        gint show_report_method_of_payment_content )
{
    ReportStruct *report;

    report = gsb_data_report_get_structure (report_number);

    if ( !report )
	return FALSE;

    report -> show_report_method_of_payment_content = show_report_method_of_payment_content;

    return TRUE;
}


/**
 * get the  show_report_marked
 *
 * \param report_number the number of the report
 *
 * \return the show_report_marked  of the report, -1 if problem
 * */
gint gsb_data_report_get_show_report_marked ( gint report_number )
{
    ReportStruct *report;

    report = gsb_data_report_get_structure (report_number);

    if ( !report )
	return -1;

    return report -> show_report_marked;
}

/**
 * set the show_report_marked
 *
 * \param report_number number of the report
 * \param show_report_marked
 *
 * \return TRUE if ok
 * */
gboolean gsb_data_report_set_show_report_marked ( gint report_number,
                        gint show_report_marked )
{
    ReportStruct *report;

    report = gsb_data_report_get_structure (report_number);

    if ( !report )
	return FALSE;

    report -> show_report_marked = show_report_marked;

    return TRUE;
}


/**
 * get the  show_report_financial_year
 *
 * \param report_number the number of the report
 *
 * \return the show_report_financial_year  of the report, -1 if problem
 * */
gint gsb_data_report_get_show_report_financial_year ( gint report_number )
{
    ReportStruct *report;

    report = gsb_data_report_get_structure (report_number);

    if ( !report )
	return -1;

    return report -> show_report_financial_year;
}

/**
 * set the show_report_financial_year
 *
 * \param report_number number of the report
 * \param show_report_financial_year
 *
 * \return TRUE if ok
 * */
gboolean gsb_data_report_set_show_report_financial_year ( gint report_number,
                        gint show_report_financial_year )
{
    ReportStruct *report;

    report = gsb_data_report_get_structure (report_number);

    if ( !report )
	return FALSE;

    report -> show_report_financial_year = show_report_financial_year;

    return TRUE;
}


/**
 * get the  sorting_report
 *
 * \param report_number the number of the report
 *
 * \return the sorting_report  of the report, -1 if problem
 * */
gint gsb_data_report_get_sorting_report ( gint report_number )
{
    ReportStruct *report;

    report = gsb_data_report_get_structure (report_number);

    if ( !report )
	return -1;

    return report -> sorting_report;
}

/**
 * set the sorting_report
 *
 * \param report_number number of the report
 * \param sorting_report
 *
 * \return TRUE if ok
 * */
gboolean gsb_data_report_set_sorting_report ( gint report_number,
                        gint sorting_report )
{
    ReportStruct *report;

    report = gsb_data_report_get_structure (report_number);

    if ( !report )
	return FALSE;

    report -> sorting_report = sorting_report;

    return TRUE;
}


/**
 * get the  not_detail_split
 *
 * \param report_number the number of the report
 *
 * \return the not_detail_split  of the report, -1 if problem
 * */
gint gsb_data_report_get_not_detail_split ( gint report_number )
{
    ReportStruct *report;

    report = gsb_data_report_get_structure (report_number);

    if ( !report )
	return -1;

    return report -> not_detail_split;
}

/**
 * set the not_detail_split
 *
 * \param report_number number of the report
 * \param not_detail_split
 *
 * \return TRUE if ok
 * */
gboolean gsb_data_report_set_not_detail_split ( gint report_number,
                        gint not_detail_split )
{
    ReportStruct *report;

    report = gsb_data_report_get_structure (report_number);

    if ( !report )
	return FALSE;

    report -> not_detail_split = not_detail_split;

    return TRUE;
}


/**
 * get the  split_credit_debit
 *
 * \param report_number the number of the report
 *
 * \return the split_credit_debit  of the report, -1 if problem
 * */
gint gsb_data_report_get_split_credit_debit ( gint report_number )
{
    ReportStruct *report;

    report = gsb_data_report_get_structure (report_number);

    if ( !report )
	return -1;

    return report -> split_credit_debit;
}

/**
 * set the split_credit_debit
 *
 * \param report_number number of the report
 * \param split_credit_debit
 *
 * \return TRUE if ok
 * */
gboolean gsb_data_report_set_split_credit_debit ( gint report_number,
                        gint split_credit_debit )
{
    ReportStruct *report;

    report = gsb_data_report_get_structure (report_number);

    if ( !report )
	return FALSE;

    report -> split_credit_debit = split_credit_debit;

    return TRUE;
}


/**
 * get the  currency_general
 *
 * \param report_number the number of the report
 *
 * \return the  currency_general of the report, -1 if problem
 * */
gint gsb_data_report_get_currency_general ( gint report_number )
{
    ReportStruct *report;

    report = gsb_data_report_get_structure (report_number);

    if ( !report )
	return -1;

    return report -> currency_general;
}

/**
 * set the currency_general
 *
 * \param report_number number of the report
 * \param currency_general
 *
 * \return TRUE if ok
 * */
gboolean gsb_data_report_set_currency_general ( gint report_number,
                        gint currency_general )
{
    ReportStruct *report;

    report = gsb_data_report_get_structure (report_number);

    if ( !report )
	return FALSE;

    report -> currency_general = currency_general;

    return TRUE;
}


/**
 * get the  column_title_show
 *
 * \param report_number the number of the report
 *
 * \return the column_title_show  of the report, -1 if problem
 * */
gint gsb_data_report_get_column_title_show ( gint report_number )
{
    ReportStruct *report;

    report = gsb_data_report_get_structure (report_number);

    if ( !report )
	return -1;

    return report -> column_title_show;
}

/**
 * set the column_title_show
 *
 * \param report_number number of the report
 * \param column_title_show
 *
 * \return TRUE if ok
 * */
gboolean gsb_data_report_set_column_title_show ( gint report_number,
                        gint column_title_show )
{
    ReportStruct *report;

    report = gsb_data_report_get_structure (report_number);

    if ( !report )
	return FALSE;

    report -> column_title_show = column_title_show;

    return TRUE;
}


/**
 * get the  column_title_type
 *
 * \param report_number the number of the report
 *
 * \return the column_title_type  of the report, -1 if problem
 * */
gint gsb_data_report_get_column_title_type ( gint report_number )
{
    ReportStruct *report;

    report = gsb_data_report_get_structure (report_number);

    if ( !report )
	return -1;

    return report -> column_title_type;
}

/**
 * set the column_title_type
 *
 * \param report_number number of the report
 * \param column_title_type
 *
 * \return TRUE if ok
 * */
gboolean gsb_data_report_set_column_title_type ( gint report_number,
                        gint column_title_type )
{
    ReportStruct *report;

    report = gsb_data_report_get_structure (report_number);

    if ( !report )
	return FALSE;

    report -> column_title_type = column_title_type;

    return TRUE;
}


/**
 * get the  append_in_payee
 *
 * \param report_number the number of the report
 *
 * \return the  append_in_payee of the report, -1 if problem
 * */
gint gsb_data_report_get_append_in_payee ( gint report_number )
{
    ReportStruct *report;

    report = gsb_data_report_get_structure (report_number);

    if ( !report )
	return -1;

    return report -> append_in_payee;
}

/**
 * set the append_in_payee
 *
 * \param report_number number of the report
 * \param append_in_payee
 *
 * \return TRUE if ok
 * */
gboolean gsb_data_report_set_append_in_payee ( gint report_number,
                        gint append_in_payee )
{
    ReportStruct *report;

    report = gsb_data_report_get_structure (report_number);

    if ( !report )
	return FALSE;

    report -> append_in_payee = append_in_payee;

    return TRUE;
}


/**
 * get the  report_can_click
 *
 * \param report_number the number of the report
 *
 * \return the report_can_click  of the report, -1 if problem
 * */
gint gsb_data_report_get_report_can_click ( gint report_number )
{
    ReportStruct *report;

    report = gsb_data_report_get_structure (report_number);

    if ( !report )
	return -1;

    return report -> report_can_click;
}

/**
 * set the report_can_click
 *
 * \param report_number number of the report
 * \param report_can_click
 *
 * \return TRUE if ok
 * */
gboolean gsb_data_report_set_report_can_click ( gint report_number,
                        gint report_can_click )
{
    ReportStruct *report;

    report = gsb_data_report_get_structure (report_number);

    if ( !report )
	return FALSE;

    report -> report_can_click = report_can_click;

    return TRUE;
}


/**
 * get the  use_financial_year
 *
 * \param report_number the number of the report
 *
 * \return the use_financial_year  of the report, -1 if problem
 * */
gint gsb_data_report_get_use_financial_year ( gint report_number )
{
    ReportStruct *report;

    report = gsb_data_report_get_structure (report_number);

    if ( !report )
	return -1;

    return report -> use_financial_year;
}

/**
 * set the use_financial_year
 *
 * \param report_number number of the report
 * \param use_financial_year
 *
 * \return TRUE if ok
 * */
gboolean gsb_data_report_set_use_financial_year ( gint report_number,
                        gint use_financial_year )
{
    ReportStruct *report;

    report = gsb_data_report_get_structure (report_number);

    if ( !report )
	return FALSE;

    report -> use_financial_year = use_financial_year;

    return TRUE;
}


/**
 * get the  financial_year_type
 *
 * \param report_number the number of the report
 *
 * \return the financial_year_type  of the report, -1 if problem
 * */
gint gsb_data_report_get_financial_year_type ( gint report_number )
{
    ReportStruct *report;

    report = gsb_data_report_get_structure (report_number);

    if ( !report )
	return -1;

    return report -> financial_year_type;
}

/**
 * set the financial_year_type
 *
 * \param report_number number of the report
 * \param financial_year_type
 *
 * \return TRUE if ok
 * */
gboolean gsb_data_report_set_financial_year_type ( gint report_number,
                        gint financial_year_type )
{
    ReportStruct *report;

    report = gsb_data_report_get_structure (report_number);

    if ( !report )
	return FALSE;

    report -> financial_year_type = financial_year_type;

    return TRUE;
}


/**
 * get the  financial_year_split
 *
 * \param report_number the number of the report
 *
 * \return the financial_year_split  of the report, -1 if problem
 * */
gint gsb_data_report_get_financial_year_split ( gint report_number )
{
    ReportStruct *report;

    report = gsb_data_report_get_structure (report_number);

    if ( !report )
	return -1;

    return report -> financial_year_split;
}

/**
 * set the financial_year_split
 *
 * \param report_number number of the report
 * \param financial_year_split
 *
 * \return TRUE if ok
 * */
gboolean gsb_data_report_set_financial_year_split ( gint report_number,
                        gint financial_year_split )
{
    ReportStruct *report;

    report = gsb_data_report_get_structure (report_number);

    if ( !report )
	return FALSE;

    report -> financial_year_split = financial_year_split;

    return TRUE;
}


/**
 * get the  date_type
 *
 * \param report_number the number of the report
 *
 * \return the date_type  of the report, -1 if problem
 * */
gint gsb_data_report_get_date_type ( gint report_number )
{
    ReportStruct *report;

    report = gsb_data_report_get_structure (report_number);

    if ( !report )
	return -1;

    return report -> date_type;
}

/**
 * set the date_type
 *
 * \param report_number number of the report
 * \param date_type
 *
 * \return TRUE if ok
 * */
gboolean gsb_data_report_set_date_type ( gint report_number,
                        gint date_type )
{
    ReportStruct *report;

    report = gsb_data_report_get_structure (report_number);

    if ( !report )
	return FALSE;

    report -> date_type = date_type;

    return TRUE;
}


/**
 * get the  personal_date_start
 *
 * \param report_number the number of the report
 *
 * \return the personal_date_start  of the report, -1 if problem
 * */
GDate *gsb_data_report_get_personal_date_start ( gint report_number )
{
    ReportStruct *report;

    report = gsb_data_report_get_structure (report_number);

    if ( !report )
	return NULL;

    return report -> personal_date_start;
}

/**
 * set the personal_date_start
 *
 * \param report_number number of the report
 * \param personal_date_start
 *
 * \return TRUE if ok
 * */
gboolean gsb_data_report_set_personal_date_start ( gint report_number,
                        GDate *personal_date_start )
{
    ReportStruct *report;

    report = gsb_data_report_get_structure (report_number);

    if ( !report )
	return FALSE;

    report -> personal_date_start = personal_date_start;

    return TRUE;
}

/**
 * get the  personal_date_end
 *
 * \param report_number the number of the report
 *
 * \return the personal_date_end  of the report, -1 if problem
 * */
GDate *gsb_data_report_get_personal_date_end ( gint report_number )
{
    ReportStruct *report;

    report = gsb_data_report_get_structure (report_number);

    if ( !report )
	return NULL;

    return report -> personal_date_end;
}

/**
 * set the personal_date_end
 *
 * \param report_number number of the report
 * \param personal_date_end
 *
 * \return TRUE if ok
 * */
gboolean gsb_data_report_set_personal_date_end ( gint report_number,
                        GDate *personal_date_end )
{
    ReportStruct *report;

    report = gsb_data_report_get_structure (report_number);

    if ( !report )
	return FALSE;

    report -> personal_date_end = personal_date_end;

    return TRUE;
}


/**
 * get the  period_split
 *
 * \param report_number the number of the report
 *
 * \return the period_split  of the report, -1 if problem
 * */
gint gsb_data_report_get_period_split ( gint report_number )
{
    ReportStruct *report;

    report = gsb_data_report_get_structure (report_number);

    if ( !report )
	return -1;

    return report -> period_split;
}

/**
 * set the period_split
 *
 * \param report_number number of the report
 * \param period_split
 *
 * \return TRUE if ok
 * */
gboolean gsb_data_report_set_period_split ( gint report_number,
                        gint period_split )
{
    ReportStruct *report;

    report = gsb_data_report_get_structure (report_number);

    if ( !report )
	return FALSE;

    report -> period_split = period_split;

    return TRUE;
}


/**
 * get the  period_split_type
 *
 * \param report_number the number of the report
 *
 * \return the period_split_type  of the report, -1 if problem
 * */
gint gsb_data_report_get_period_split_type ( gint report_number )
{
    ReportStruct *report;

    report = gsb_data_report_get_structure (report_number);

    if ( !report )
	return -1;

    return report -> period_split_type;
}

/**
 * set the period_split_type
 *
 * \param report_number number of the report
 * \param period_split_type
 *
 * \return TRUE if ok
 * */
gboolean gsb_data_report_set_period_split_type ( gint report_number,
                        gint period_split_type )
{
    ReportStruct *report;

    report = gsb_data_report_get_structure (report_number);

    if ( !report )
	return FALSE;

    report -> period_split_type = period_split_type;

    return TRUE;
}


/**
 * get the  period_split_day
 *
 * \param report_number the number of the report
 *
 * \return the period_split_day  of the report, -1 if problem
 * */
gint gsb_data_report_get_period_split_day ( gint report_number )
{
    ReportStruct *report;

    report = gsb_data_report_get_structure (report_number);

    if ( !report )
	return -1;

    return report -> period_split_day;
}

/**
 * set the period_split_day
 *
 * \param report_number number of the report
 * \param period_split_day
 *
 * \return TRUE if ok
 * */
gboolean gsb_data_report_set_period_split_day ( gint report_number,
                        gint period_split_day )
{
    ReportStruct *report;

    report = gsb_data_report_get_structure (report_number);

    if ( !report )
	return FALSE;

    report -> period_split_day = period_split_day;

    return TRUE;
}


/**
 * get the  account_use_chosen
 *
 * \param report_number the number of the report
 *
 * \return the account_use_chosen  of the report, -1 if problem
 * */
gint gsb_data_report_get_account_use_chosen ( gint report_number )
{
    ReportStruct *report;

    report = gsb_data_report_get_structure (report_number);

    if ( !report )
	return -1;

    return report -> account_use_chosen;
}

/**
 * set the account_use_chosen
 *
 * \param report_number number of the report
 * \param account_use_chosen
 *
 * \return TRUE if ok
 * */
gboolean gsb_data_report_set_account_use_chosen ( gint report_number,
                        gint account_use_chosen )
{
    ReportStruct *report;

    report = gsb_data_report_get_structure (report_number);

    if ( !report )
	return FALSE;

    report -> account_use_chosen = account_use_chosen;

    return TRUE;
}


/**
 * get the  account_group_reports
 *
 * \param report_number the number of the report
 *
 * \return the account_group_reports  of the report, -1 if problem
 * */
gint gsb_data_report_get_account_group_reports ( gint report_number )
{
    ReportStruct *report;

    report = gsb_data_report_get_structure (report_number);

    if ( !report )
	return -1;

    return report -> account_group_reports;
}

/**
 * set the account_group_reports
 *
 * \param report_number number of the report
 * \param account_group_reports
 *
 * \return TRUE if ok
 * */
gboolean gsb_data_report_set_account_group_reports ( gint report_number,
                        gint account_group_reports )
{
    ReportStruct *report;

    report = gsb_data_report_get_structure (report_number);

    if ( !report )
	return FALSE;

    report -> account_group_reports = account_group_reports;

    return TRUE;
}


/**
 * get the  account_show_amount
 *
 * \param report_number the number of the report
 *
 * \return the account_show_amount  of the report, -1 if problem
 * */
gint gsb_data_report_get_account_show_amount ( gint report_number )
{
    ReportStruct *report;

    report = gsb_data_report_get_structure (report_number);

    if ( !report )
	return -1;

    return report -> account_show_amount;
}

/**
 * set the account_show_amount
 *
 * \param report_number number of the report
 * \param account_show_amount
 *
 * \return TRUE if ok
 * */
gboolean gsb_data_report_set_account_show_amount ( gint report_number,
                        gint account_show_amount )
{
    ReportStruct *report;

    report = gsb_data_report_get_structure (report_number);

    if ( !report )
	return FALSE;

    report -> account_show_amount = account_show_amount;

    return TRUE;
}


/**
 * get the  account_show_name
 *
 * \param report_number the number of the report
 *
 * \return the account_show_name  of the report, -1 if problem
 * */
gint gsb_data_report_get_account_show_name ( gint report_number )
{
    ReportStruct *report;

    report = gsb_data_report_get_structure (report_number);

    if ( !report )
	return -1;

    return report -> account_show_name;
}

/**
 * set the account_show_name
 *
 * \param report_number number of the report
 * \param account_show_name
 *
 * \return TRUE if ok
 * */
gboolean gsb_data_report_set_account_show_name ( gint report_number,
                        gint account_show_name )
{
    ReportStruct *report;

    report = gsb_data_report_get_structure (report_number);

    if ( !report )
	return FALSE;

    report -> account_show_name = account_show_name;

    return TRUE;
}


/**
 * get the  transfer_choice
 *
 * \param report_number the number of the report
 *
 * \return the transfer_choice  of the report, -1 if problem
 * */
gint gsb_data_report_get_transfer_choice ( gint report_number )
{
    ReportStruct *report;

    report = gsb_data_report_get_structure (report_number);

    if ( !report )
	return -1;

    return report -> transfer_choice;
}

/**
 * set the transfer_choice
 *
 * \param report_number number of the report
 * \param transfer_choice
 *
 * \return TRUE if ok
 * */
gboolean gsb_data_report_set_transfer_choice ( gint report_number,
                        gint transfer_choice )
{
    ReportStruct *report;

    report = gsb_data_report_get_structure (report_number);

    if ( !report )
	return FALSE;

    report -> transfer_choice = transfer_choice;

    return TRUE;
}


/**
 * get the  transfer_reports_only
 *
 * \param report_number the number of the report
 *
 * \return the transfer_reports_only  of the report, -1 if problem
 * */
gint gsb_data_report_get_transfer_reports_only ( gint report_number )
{
    ReportStruct *report;

    report = gsb_data_report_get_structure (report_number);

    if ( !report )
	return -1;

    return report -> transfer_reports_only;
}

/**
 * set the transfer_reports_only
 *
 * \param report_number number of the report
 * \param transfer_reports_only
 *
 * \return TRUE if ok
 * */
gboolean gsb_data_report_set_transfer_reports_only ( gint report_number,
                        gint transfer_reports_only )
{
    ReportStruct *report;

    report = gsb_data_report_get_structure (report_number);

    if ( !report )
	return FALSE;

    report -> transfer_reports_only = transfer_reports_only;

    return TRUE;
}


/**
 * get the  category_used
 *
 * \param report_number the number of the report
 *
 * \return the category_used  of the report, -1 if problem
 * */
gint gsb_data_report_get_category_used ( gint report_number )
{
    ReportStruct *report;

    report = gsb_data_report_get_structure (report_number);

    if ( !report )
	return -1;

    return report -> category_used;
}

/**
 * set the category_used
 *
 * \param report_number number of the report
 * \param category_used
 *
 * \return TRUE if ok
 * */
gboolean gsb_data_report_set_category_used ( gint report_number,
                        gint category_used )
{
    ReportStruct *report;

    report = gsb_data_report_get_structure (report_number);

    if ( !report )
	return FALSE;

    report -> category_used = category_used;

    return TRUE;
}


/**
 * get the category_detail_used
 *
 * \param report_number the number of the report
 *
 * \return the category_detail_used  of the report, -1 if problem
 * */
gint gsb_data_report_get_category_detail_used ( gint report_number )
{
    ReportStruct *report;

    report = gsb_data_report_get_structure (report_number);

    if ( !report )
	return -1;

    return report -> category_detail_used;
}

/**
 * set the category_detail_used
 *
 * \param report_number number of the report
 * \param category_detail_used
 *
 * \return TRUE if ok
 * */
gboolean gsb_data_report_set_category_detail_used ( gint report_number,
                        gint category_detail_used )
{
    ReportStruct *report;

    report = gsb_data_report_get_structure (report_number);

    if ( !report )
	return FALSE;

    report -> category_detail_used = category_detail_used;

    return TRUE;
}


/**
 * get the  category_show_sub_category
 *
 * \param report_number the number of the report
 *
 * \return the category_show_sub_category  of the report, -1 if problem
 * */
gint gsb_data_report_get_category_show_sub_category ( gint report_number )
{
    ReportStruct *report;

    report = gsb_data_report_get_structure (report_number);

    if ( !report )
	return -1;

    return report -> category_show_sub_category;
}

/**
 * set the category_show_sub_category
 *
 * \param report_number number of the report
 * \param category_show_sub_category
 *
 * \return TRUE if ok
 * */
gboolean gsb_data_report_set_category_show_sub_category ( gint report_number,
                        gint category_show_sub_category )
{
    ReportStruct *report;

    report = gsb_data_report_get_structure (report_number);

    if ( !report )
	return FALSE;

    report -> category_show_sub_category = category_show_sub_category;

    return TRUE;
}


/**
 * get the  category_show_category_amount
 *
 * \param report_number the number of the report
 *
 * \return the  category_show_category_amount of the report, -1 if problem
 * */
gint gsb_data_report_get_category_show_category_amount ( gint report_number )
{
    ReportStruct *report;

    report = gsb_data_report_get_structure (report_number);

    if ( !report )
	return -1;

    return report -> category_show_category_amount;
}

/**
 * set the category_show_category_amount
 *
 * \param report_number number of the report
 * \param category_show_category_amount
 *
 * \return TRUE if ok
 * */
gboolean gsb_data_report_set_category_show_category_amount ( gint report_number,
                        gint category_show_category_amount )
{
    ReportStruct *report;

    report = gsb_data_report_get_structure (report_number);

    if ( !report )
	return FALSE;

    report -> category_show_category_amount = category_show_category_amount;

    return TRUE;
}


/**
 * get the  category_show_sub_category_amount
 *
 * \param report_number the number of the report
 *
 * \return the category_show_sub_category_amount  of the report, -1 if problem
 * */
gint gsb_data_report_get_category_show_sub_category_amount ( gint report_number )
{
    ReportStruct *report;

    report = gsb_data_report_get_structure (report_number);

    if ( !report )
	return -1;

    return report -> category_show_sub_category_amount;
}

/**
 * set the category_show_sub_category_amount
 *
 * \param report_number number of the report
 * \param category_show_sub_category_amount
 *
 * \return TRUE if ok
 * */
gboolean gsb_data_report_set_category_show_sub_category_amount ( gint report_number,
                        gint category_show_sub_category_amount )
{
    ReportStruct *report;

    report = gsb_data_report_get_structure (report_number);

    if ( !report )
	return FALSE;

    report -> category_show_sub_category_amount = category_show_sub_category_amount;

    return TRUE;
}



/**
 * get the  category_currency
 *
 * \param report_number the number of the report
 *
 * \return the category_currency  of the report, -1 if problem
 * */
gint gsb_data_report_get_category_currency ( gint report_number )
{
    ReportStruct *report;

    report = gsb_data_report_get_structure (report_number);

    if ( !report )
	return -1;

    return report -> category_currency;
}

/**
 * set the category_currency
 *
 * \param report_number number of the report
 * \param category_currency
 *
 * \return TRUE if ok
 * */
gboolean gsb_data_report_set_category_currency ( gint report_number,
                        gint category_currency )
{
    ReportStruct *report;

    report = gsb_data_report_get_structure (report_number);

    if ( !report )
	return FALSE;

    report -> category_currency = category_currency;

    return TRUE;
}


/**
 * get the  category_show_without_category
 *
 * \param report_number the number of the report
 *
 * \return the category_show_without_category  of the report, -1 if problem
 * */
gint gsb_data_report_get_category_show_without_category ( gint report_number )
{
    ReportStruct *report;

    report = gsb_data_report_get_structure (report_number);

    if ( !report )
	return -1;

    return report -> category_show_without_category;
}

/**
 * set the category_show_without_category
 *
 * \param report_number number of the report
 * \param category_show_without_category
 *
 * \return TRUE if ok
 * */
gboolean gsb_data_report_set_category_show_without_category ( gint report_number,
                        gint category_show_without_category )
{
    ReportStruct *report;

    report = gsb_data_report_get_structure (report_number);

    if ( !report )
	return FALSE;

    report -> category_show_without_category = category_show_without_category;

    return TRUE;
}


/**
 * get the  category_show_name
 *
 * \param report_number the number of the report
 *
 * \return the category_show_name  of the report, -1 if problem
 * */
gint gsb_data_report_get_category_show_name ( gint report_number )
{
    ReportStruct *report;

    report = gsb_data_report_get_structure (report_number);

    if ( !report )
	return -1;

    return report -> category_show_name;
}

/**
 * set the category_show_name
 *
 * \param report_number number of the report
 * \param category_show_name
 *
 * \return TRUE if ok
 * */
gboolean gsb_data_report_set_category_show_name ( gint report_number,
                        gint category_show_name )
{
    ReportStruct *report;

    report = gsb_data_report_get_structure (report_number);

    if ( !report )
	return FALSE;

    report -> category_show_name = category_show_name;

    return TRUE;
}


/**
 * get the  budget_used
 *
 * \param report_number the number of the report
 *
 * \return the budget_used  of the report, -1 if problem
 * */
gint gsb_data_report_get_budget_used ( gint report_number )
{
    ReportStruct *report;

    report = gsb_data_report_get_structure (report_number);

    if ( !report )
	return -1;

    return report -> budget_used;
}

/**
 * set the budget_used
 *
 * \param report_number number of the report
 * \param budget_used
 *
 * \return TRUE if ok
 * */
gboolean gsb_data_report_set_budget_used ( gint report_number,
                        gint budget_used )
{
    ReportStruct *report;

    report = gsb_data_report_get_structure (report_number);

    if ( !report )
	return FALSE;

    report -> budget_used = budget_used;

    return TRUE;
}


/**
 * get the  budget_detail_used
 *
 * \param report_number the number of the report
 *
 * \return the  budget_detail_used of the report, -1 if problem
 * */
gint gsb_data_report_get_budget_detail_used ( gint report_number )
{
    ReportStruct *report;

    report = gsb_data_report_get_structure (report_number);

    if ( !report )
	return -1;

    return report -> budget_detail_used;
}

/**
 * set the budget_detail_used
 *
 * \param report_number number of the report
 * \param budget_detail_used
 *
 * \return TRUE if ok
 * */
gboolean gsb_data_report_set_budget_detail_used ( gint report_number,
                        gint budget_detail_used )
{
    ReportStruct *report;

    report = gsb_data_report_get_structure (report_number);

    if ( !report )
	return FALSE;

    report -> budget_detail_used = budget_detail_used;

    return TRUE;
}


/**
 * get the  budget_show_sub_budget
 *
 * \param report_number the number of the report
 *
 * \return the budget_show_sub_budget  of the report, -1 if problem
 * */
gint gsb_data_report_get_budget_show_sub_budget ( gint report_number )
{
    ReportStruct *report;

    report = gsb_data_report_get_structure (report_number);

    if ( !report )
	return -1;

    return report -> budget_show_sub_budget;
}

/**
 * set the budget_show_sub_budget
 *
 * \param report_number number of the report
 * \param budget_show_sub_budget
 *
 * \return TRUE if ok
 * */
gboolean gsb_data_report_set_budget_show_sub_budget ( gint report_number,
                        gint budget_show_sub_budget )
{
    ReportStruct *report;

    report = gsb_data_report_get_structure (report_number);

    if ( !report )
	return FALSE;

    report -> budget_show_sub_budget = budget_show_sub_budget;

    return TRUE;
}


/**
 * get the  budget_show_budget_amount
 *
 * \param report_number the number of the report
 *
 * \return the budget_show_budget_amount  of the report, -1 if problem
 * */
gint gsb_data_report_get_budget_show_budget_amount ( gint report_number )
{
    ReportStruct *report;

    report = gsb_data_report_get_structure (report_number);

    if ( !report )
	return -1;

    return report -> budget_show_budget_amount;
}

/**
 * set the budget_show_budget_amount
 *
 * \param report_number number of the report
 * \param budget_show_budget_amount
 *
 * \return TRUE if ok
 * */
gboolean gsb_data_report_set_budget_show_budget_amount ( gint report_number,
                        gint budget_show_budget_amount )
{
    ReportStruct *report;

    report = gsb_data_report_get_structure (report_number);

    if ( !report )
	return FALSE;

    report -> budget_show_budget_amount = budget_show_budget_amount;

    return TRUE;
}

/**
 * get the  budget_show_sub_budget_amount
 *
 * \param report_number the number of the report
 *
 * \return the budget_show_sub_budget_amount  of the report, -1 if problem
 * */
gint gsb_data_report_get_budget_show_sub_budget_amount ( gint report_number )
{
    ReportStruct *report;

    report = gsb_data_report_get_structure (report_number);

    if ( !report )
	return -1;

    return report -> budget_show_sub_budget_amount;
}

/**
 * set the budget_show_sub_budget_amount
 *
 * \param report_number number of the report
 * \param budget_show_sub_budget_amount
 *
 * \return TRUE if ok
 * */
gboolean gsb_data_report_set_budget_show_sub_budget_amount ( gint report_number,
                        gint budget_show_sub_budget_amount )
{
    ReportStruct *report;

    report = gsb_data_report_get_structure (report_number);

    if ( !report )
	return FALSE;

    report -> budget_show_sub_budget_amount = budget_show_sub_budget_amount;

    return TRUE;
}




/**
 * get the  budget_currency
 *
 * \param report_number the number of the report
 *
 * \return the budget_currency  of the report, -1 if problem
 * */
gint gsb_data_report_get_budget_currency ( gint report_number )
{
    ReportStruct *report;

    report = gsb_data_report_get_structure (report_number);

    if ( !report )
	return -1;

    return report -> budget_currency;
}

/**
 * set the budget_currency
 *
 * \param report_number number of the report
 * \param budget_currency
 *
 * \return TRUE if ok
 * */
gboolean gsb_data_report_set_budget_currency ( gint report_number,
                        gint budget_currency )
{
    ReportStruct *report;

    report = gsb_data_report_get_structure (report_number);

    if ( !report )
	return FALSE;

    report -> budget_currency = budget_currency;

    return TRUE;
}


/**
 * get the  budget_show_without_budget
 *
 * \param report_number the number of the report
 *
 * \return the budget_show_without_budget  of the report, -1 if problem
 * */
gint gsb_data_report_get_budget_show_without_budget ( gint report_number )
{
    ReportStruct *report;

    report = gsb_data_report_get_structure (report_number);

    if ( !report )
	return -1;

    return report -> budget_show_without_budget;
}

/**
 * set the budget_show_without_budget
 *
 * \param report_number number of the report
 * \param budget_show_without_budget
 *
 * \return TRUE if ok
 * */
gboolean gsb_data_report_set_budget_show_without_budget ( gint report_number,
                        gint budget_show_without_budget )
{
    ReportStruct *report;

    report = gsb_data_report_get_structure (report_number);

    if ( !report )
	return FALSE;

    report -> budget_show_without_budget = budget_show_without_budget;

    return TRUE;
}


/**
 * get the  budget_show_name
 *
 * \param report_number the number of the report
 *
 * \return the budget_show_name  of the report, -1 if problem
 * */
gint gsb_data_report_get_budget_show_name ( gint report_number )
{
    ReportStruct *report;

    report = gsb_data_report_get_structure (report_number);

    if ( !report )
	return -1;

    return report -> budget_show_name;
}

/**
 * set the budget_show_name
 *
 * \param report_number number of the report
 * \param budget_show_name
 *
 * \return TRUE if ok
 * */
gboolean gsb_data_report_set_budget_show_name ( gint report_number,
                        gint budget_show_name )
{
    ReportStruct *report;

    report = gsb_data_report_get_structure (report_number);

    if ( !report )
	return FALSE;

    report -> budget_show_name = budget_show_name;

    return TRUE;
}


/**
 * get the  payee_used
 *
 * \param report_number the number of the report
 *
 * \return the  payee_used of the report, -1 if problem
 * */
gint gsb_data_report_get_payee_used ( gint report_number )
{
    ReportStruct *report;

    report = gsb_data_report_get_structure (report_number);

    if ( !report )
	return -1;

    return report -> payee_used;
}

/**
 * set the payee_used
 *
 * \param report_number number of the report
 * \param payee_used
 *
 * \return TRUE if ok
 * */
gboolean gsb_data_report_set_payee_used ( gint report_number,
                        gint payee_used )
{
    ReportStruct *report;

    report = gsb_data_report_get_structure (report_number);

    if ( !report )
	return FALSE;

    report -> payee_used = payee_used;

    return TRUE;
}


/**
 * get the  payee_detail_used
 *
 * \param report_number the number of the report
 *
 * \return the  payee_detail_used of the report, -1 if problem
 * */
gint gsb_data_report_get_payee_detail_used ( gint report_number )
{
    ReportStruct *report;

    report = gsb_data_report_get_structure (report_number);

    if ( !report )
	return -1;

    return report -> payee_detail_used;
}

/**
 * set the payee_detail_used
 *
 * \param report_number number of the report
 * \param payee_detail_used
 *
 * \return TRUE if ok
 * */
gboolean gsb_data_report_set_payee_detail_used ( gint report_number,
                        gint payee_detail_used )
{
    ReportStruct *report;

    report = gsb_data_report_get_structure (report_number);

    if ( !report )
	return FALSE;

    report -> payee_detail_used = payee_detail_used;

    return TRUE;
}


/**
 * get the  payee_show_payee_amount
 *
 * \param report_number the number of the report
 *
 * \return the payee_show_payee_amount  of the report, -1 if problem
 * */
gint gsb_data_report_get_payee_show_payee_amount ( gint report_number )
{
    ReportStruct *report;

    report = gsb_data_report_get_structure (report_number);

    if ( !report )
	return -1;

    return report -> payee_show_payee_amount;
}

/**
 * set the payee_show_payee_amount
 *
 * \param report_number number of the report
 * \param payee_show_payee_amount
 *
 * \return TRUE if ok
 * */
gboolean gsb_data_report_set_payee_show_payee_amount ( gint report_number,
                        gint payee_show_payee_amount )
{
    ReportStruct *report;

    report = gsb_data_report_get_structure (report_number);

    if ( !report )
	return FALSE;

    report -> payee_show_payee_amount = payee_show_payee_amount;

    return TRUE;
}


/**
 * get the  payee_currency
 *
 * \param report_number the number of the report
 *
 * \return the payee_currency  of the report, -1 if problem
 * */
gint gsb_data_report_get_payee_currency ( gint report_number )
{
    ReportStruct *report;

    report = gsb_data_report_get_structure (report_number);

    if ( !report )
	return -1;

    return report -> payee_currency;
}

/**
 * set the payee_currency
 *
 * \param report_number number of the report
 * \param payee_currency
 *
 * \return TRUE if ok
 * */
gboolean gsb_data_report_set_payee_currency ( gint report_number,
                        gint payee_currency )
{
    ReportStruct *report;

    report = gsb_data_report_get_structure (report_number);

    if ( !report )
	return FALSE;

    report -> payee_currency = payee_currency;

    return TRUE;
}


/**
 * get the  payee_show_name
 *
 * \param report_number the number of the report
 *
 * \return the payee_show_name  of the report, -1 if problem
 * */
gint gsb_data_report_get_payee_show_name ( gint report_number )
{
    ReportStruct *report;

    report = gsb_data_report_get_structure (report_number);

    if ( !report )
	return -1;

    return report -> payee_show_name;
}

/**
 * set the payee_show_name
 *
 * \param report_number number of the report
 * \param payee_show_name
 *
 * \return TRUE if ok
 * */
gboolean gsb_data_report_set_payee_show_name ( gint report_number,
                        gint payee_show_name )
{
    ReportStruct *report;

    report = gsb_data_report_get_structure (report_number);

    if ( !report )
	return FALSE;

    report -> payee_show_name = payee_show_name;

    return TRUE;
}


/**
 * get the  text_comparison_used
 *
 * \param report_number the number of the report
 *
 * \return the text_comparison_used  of the report, -1 if problem
 * */
gint gsb_data_report_get_text_comparison_used ( gint report_number )
{
    ReportStruct *report;

    report = gsb_data_report_get_structure (report_number);

    if ( !report )
	return -1;

    return report -> text_comparison_used;
}

/**
 * set the text_comparison_used
 *
 * \param report_number number of the report
 * \param text_comparison_used
 *
 * \return TRUE if ok
 * */
gboolean gsb_data_report_set_text_comparison_used ( gint report_number,
                        gint text_comparison_used )
{
    ReportStruct *report;

    report = gsb_data_report_get_structure (report_number);

    if ( !report )
	return FALSE;

    report -> text_comparison_used = text_comparison_used;

    return TRUE;
}


/**
 * get the  amount_comparison_used
 *
 * \param report_number the number of the report
 *
 * \return the  amount_comparison_used of the report, -1 if problem
 * */
gint gsb_data_report_get_amount_comparison_used ( gint report_number )
{
    ReportStruct *report;

    report = gsb_data_report_get_structure (report_number);

    if ( !report )
	return -1;

    return report -> amount_comparison_used;
}

/**
 * set the amount_comparison_used
 *
 * \param report_number number of the report
 * \param amount_comparison_used
 *
 * \return TRUE if ok
 * */
gboolean gsb_data_report_set_amount_comparison_used ( gint report_number,
                        gint amount_comparison_used )
{
    ReportStruct *report;

    report = gsb_data_report_get_structure (report_number);

    if ( !report )
	return FALSE;

    report -> amount_comparison_used = amount_comparison_used;

    return TRUE;
}


/**
 * get the  amount_comparison_currency
 *
 * \param report_number the number of the report
 *
 * \return the amount_comparison_currency  of the report, -1 if problem
 * */
gint gsb_data_report_get_amount_comparison_currency ( gint report_number )
{
    ReportStruct *report;

    report = gsb_data_report_get_structure (report_number);

    if ( !report )
	return -1;

    return report -> amount_comparison_currency;
}

/**
 * set the amount_comparison_currency
 *
 * \param report_number number of the report
 * \param amount_comparison_currency
 *
 * \return TRUE if ok
 * */
gboolean gsb_data_report_set_amount_comparison_currency ( gint report_number,
                        gint amount_comparison_currency )
{
    ReportStruct *report;

    report = gsb_data_report_get_structure (report_number);

    if ( !report )
	return FALSE;

    report -> amount_comparison_currency = amount_comparison_currency;

    return TRUE;
}


/**
 * get the  amount_comparison_only_report_non_null
 *
 * \param report_number the number of the report
 *
 * \return the amount_comparison_only_report_non_null  of the report, -1 if problem
 * */
gint gsb_data_report_get_amount_comparison_only_report_non_null ( gint report_number )
{
    ReportStruct *report;

    report = gsb_data_report_get_structure (report_number);

    if ( !report )
	return -1;

    return report -> amount_comparison_only_report_non_null;
}

/**
 * set the amount_comparison_only_report_non_null
 *
 * \param report_number number of the report
 * \param amount_comparison_only_report_non_null
 *
 * \return TRUE if ok
 * */
gboolean gsb_data_report_set_amount_comparison_only_report_non_null ( gint report_number,
                        gint amount_comparison_only_report_non_null )
{
    ReportStruct *report;

    report = gsb_data_report_get_structure (report_number);

    if ( !report )
	return FALSE;

    report -> amount_comparison_only_report_non_null = amount_comparison_only_report_non_null;

    return TRUE;
}

/**
 * get the  method_of_payment_used
 *
 * \param report_number the number of the report
 *
 * \return the method_of_payment_used  of the report, -1 if problem
 * */
gint gsb_data_report_get_method_of_payment_used ( gint report_number )
{
    ReportStruct *report;

    report = gsb_data_report_get_structure (report_number);

    if ( !report )
	return -1;

    return report -> method_of_payment_used;
}

/**
 * set the method_of_payment_used
 *
 * \param report_number number of the report
 * \param method_of_payment_used
 *
 * \return TRUE if ok
 * */
gboolean gsb_data_report_set_method_of_payment_used ( gint report_number,
                        gint method_of_payment_used )
{
    ReportStruct *report;

    report = gsb_data_report_get_structure (report_number);

    if ( !report )
	return FALSE;

    report -> method_of_payment_used = method_of_payment_used;

    return TRUE;
}


/**
 * free the financial_year_list
 *
 * \param report_number number of the report
 *
 * \return
 * */
void gsb_data_report_free_financial_year_list ( gint report_number )
{
    ReportStruct *report;

    report = gsb_data_report_get_structure ( report_number );

    if ( !report )
        return;
    if ( report -> financial_year_list )
        g_slist_free ( report -> financial_year_list );
    report -> financial_year_list = NULL;
}


/**
 * get the  financial_year_list
 *
 * \param report_number the number of the report
 *
 * \return the financial_year_list  of the report, -1 if problem
 * */
GSList *gsb_data_report_get_financial_year_list ( gint report_number )
{
    ReportStruct *report;

    report = gsb_data_report_get_structure (report_number);

    if ( !report )
	return NULL;

    return report -> financial_year_list;
}

/**
 * set the financial_year_list
 *
 * \param report_number number of the report
 * \param financial_year_list
 *
 * \return TRUE if ok
 * */
gboolean gsb_data_report_set_financial_year_list ( gint report_number,
                        GSList *financial_year_list )
{
    ReportStruct *report;

    report = gsb_data_report_get_structure (report_number);

    if ( !report )
        return FALSE;

    report -> financial_year_list = financial_year_list;

    return TRUE;
}


/**
 * get the  sorting_type
 *
 * \param report_number the number of the report
 *
 * \return the sorting_type  of the report, -1 if problem
 * */
GSList *gsb_data_report_get_sorting_type_list ( gint report_number )
{
    ReportStruct *report;

    report = gsb_data_report_get_structure (report_number);

    if ( !report )
	return NULL;

    return report -> sorting_type;
}

/**
 * set the sorting_type
 *
 * \param report_number number of the report
 * \param sorting_type
 *
 * \return TRUE if ok
 * */
gboolean gsb_data_report_set_sorting_type_list ( gint report_number,
                        GSList *sorting_type )
{
    ReportStruct *report;

    report = gsb_data_report_get_structure (report_number);

    if ( !report )
	return FALSE;

    report -> sorting_type = sorting_type;

    return TRUE;
}

/**
 * get the  account_numbers
 *
 * \param report_number the number of the report
 *
 * \return the account_numbers  of the report, -1 if problem
 * */
GSList *gsb_data_report_get_account_numbers_list ( gint report_number )
{
    ReportStruct *report;

    report = gsb_data_report_get_structure (report_number);

    if ( !report )
	return NULL;

    return report -> account_numbers;
}

/**
 * set the account_numbers
 *
 * \param report_number number of the report
 * \param account_numbers
 *
 * \return TRUE if ok
 * */
gboolean gsb_data_report_set_account_numbers_list ( gint report_number,
                        GSList *account_numbers )
{
    ReportStruct *report;

    report = gsb_data_report_get_structure (report_number);

    if ( !report )
	return FALSE;

    report -> account_numbers = account_numbers;

    return TRUE;
}

/**
 * get the  transfer_account_numbers
 *
 * \param report_number the number of the report
 *
 * \return the transfer_account_numbers  of the report, -1 if problem
 * */
GSList *gsb_data_report_get_transfer_account_numbers_list ( gint report_number )
{
    ReportStruct *report;

    report = gsb_data_report_get_structure (report_number);

    if ( !report )
	return NULL;

    return report -> transfer_account_numbers;
}

/**
 * set the transfer_account_numbers
 *
 * \param report_number number of the report
 * \param transfer_account_numbers
 *
 * \return TRUE if ok
 * */
gboolean gsb_data_report_set_transfer_account_numbers_list ( gint report_number,
                        GSList *transfer_account_numbers )
{
    ReportStruct *report;

    report = gsb_data_report_get_structure (report_number);

    if ( !report )
	return FALSE;

    report -> transfer_account_numbers = transfer_account_numbers;

    return TRUE;
}


/**
 * return the list of CategBudgetSel
 * containing the selected categories and sub-categories
 *
 * \param report_number the number of the report
 *
 * \return the categ_select_struct  of the report, -1 if problem
 * */
GSList *gsb_data_report_get_category_struct_list ( gint report_number )
{
    ReportStruct *report;

    report = gsb_data_report_get_structure (report_number);

    if ( !report )
	return NULL;

    return report -> categ_select_struct;
}

/**
 * set the list of CategBudgetSel
 * this is a list of CategBudgetSel
 * if there were a previous category struct list, we free it before
 *
 * \param report_number number of the report
 * \param categ_select_struct
 *
 * \return TRUE if ok
 * */
gboolean gsb_data_report_set_category_struct_list ( gint report_number,
                        GSList *categ_select_struct )
{
    ReportStruct *report;

    report = gsb_data_report_get_structure (report_number);

    if ( !report )
	return FALSE;

    if (report -> categ_select_struct)
	gsb_data_report_free_categ_budget_struct_list (report -> categ_select_struct);

    report -> categ_select_struct = categ_select_struct;

    return TRUE;
}

/**
 * free the list of categories/budgets struct
 *
 * \param categ_budget_sel_list	the list of CategBudgetSel to free
 *
 * \return TRUE if ok
 * */
void gsb_data_report_free_categ_budget_struct_list (GSList *categ_budget_sel_list)
{
    GSList *tmp_list;

    tmp_list = categ_budget_sel_list;
    while (tmp_list)
    {
        CategBudgetSel *categ_budget_struct;

        categ_budget_struct = tmp_list -> data;
        if (categ_budget_struct -> sub_div_numbers)
            g_slist_free (categ_budget_struct -> sub_div_numbers);
        g_free (categ_budget_struct);
        tmp_list = tmp_list -> next;
    }
    g_slist_free (categ_budget_sel_list);
}

/**
 * coppy the list of categories/budgets struct
 *
 * \param orig_categ_budget_list the CategBudgetSel list to copy
 *
 * \return a new GSList of CategBudgetSel
 * */
GSList *gsb_data_report_copy_categ_budget_struct (GSList *orig_categ_budget_list)
{
    GSList *tmp_list;
    GSList *new_list = NULL;

    tmp_list = orig_categ_budget_list;
    while (tmp_list)
    {
	CategBudgetSel *categ_budget_struct;
	CategBudgetSel *new_categ_budget_struct;

	categ_budget_struct = tmp_list -> data;
	new_categ_budget_struct = g_malloc0 (sizeof (CategBudgetSel));
	new_list = g_slist_append (new_list, new_categ_budget_struct);

	new_categ_budget_struct -> div_number = categ_budget_struct -> div_number;

	if (categ_budget_struct -> sub_div_numbers)
	    new_categ_budget_struct -> sub_div_numbers = g_slist_copy (categ_budget_struct -> sub_div_numbers);

	tmp_list = tmp_list -> next;
    }
    return new_list;
}

/**
 * return the list of CategBudgetSel
 * containing the selected budgets and sub-budgets
 *
 * \param report_number the number of the report
 *
 * \return the categ_select_struct  of the report, -1 if problem
 * */
GSList *gsb_data_report_get_budget_struct_list ( gint report_number )
{
    ReportStruct *report;

    report = gsb_data_report_get_structure (report_number);

    if ( !report )
	return NULL;

    return report -> budget_select_struct;
}

/**
 * set the list of budgets struct
 * this is a list of CategBudgetSel
 * if there were a previous budget struct list, we free it before
 *
 * \param report_number number of the report
 * \param categ_select_struct
 *
 * \return TRUE if ok
 * */
gboolean gsb_data_report_set_budget_struct_list ( gint report_number,
                        GSList *budget_select_struct )
{
    ReportStruct *report;

    report = gsb_data_report_get_structure (report_number);

    if ( !report )
	return FALSE;

    if (report -> budget_select_struct)
	gsb_data_report_free_categ_budget_struct_list (report -> budget_select_struct);

    report -> budget_select_struct = budget_select_struct;

    return TRUE;
}


/**
 * free the payee_numbers_list
 *
 * \param report_number number of the report
 *
 * \return
 * */
void gsb_data_report_free_payee_numbers_list ( gint report_number )
{
    ReportStruct *report;

    report = gsb_data_report_get_structure ( report_number );

    if ( !report )
        return;
    if ( report -> payee_numbers )
        g_slist_free ( report -> payee_numbers );

    report -> payee_numbers = NULL;
}


/**
 * get the  payee_numbers
 *
 * \param report_number the number of the report
 *
 * \return the payee_numbers  of the report, -1 if problem
 * */
GSList *gsb_data_report_get_payee_numbers_list ( gint report_number )
{
    ReportStruct *report;

    report = gsb_data_report_get_structure (report_number);

    if ( !report )
	return NULL;

    return report -> payee_numbers;
}

/**
 * set the payee_numbers
 *
 * \param report_number number of the report
 * \param payee_numbers
 *
 * \return TRUE if ok
 * */
gboolean gsb_data_report_set_payee_numbers_list ( gint report_number,
                        GSList *payee_numbers )
{
    ReportStruct *report;

    report = gsb_data_report_get_structure (report_number);

    if ( !report )
	return FALSE;

    report -> payee_numbers = payee_numbers;

    return TRUE;
}

/**
 * get the  text_comparison_list
 *
 * \param report_number the number of the report
 *
 * \return the text_comparison_list  of the report, -1 if problem
 * */
GSList *gsb_data_report_get_text_comparison_list ( gint report_number )
{
    ReportStruct *report;

    report = gsb_data_report_get_structure (report_number);

    if ( !report )
	return NULL;

    return report -> text_comparison_list;
}

/**
 * set the text_comparison_list
 *
 * \param report_number number of the report
 * \param text_comparison_list
 *
 * \return TRUE if ok
 * */
gboolean gsb_data_report_set_text_comparison_list ( gint report_number,
                        GSList *text_comparison_list )
{
    ReportStruct *report;

    report = gsb_data_report_get_structure (report_number);

    if ( !report )
	return FALSE;

    report -> text_comparison_list = text_comparison_list;

    return TRUE;
}

/**
 * get the  amount_comparison_list
 *
 * \param report_number the number of the report
 *
 * \return the amount_comparison_list  of the report, -1 if problem
 * */
GSList *gsb_data_report_get_amount_comparison_list ( gint report_number )
{
    ReportStruct *report;

    report = gsb_data_report_get_structure (report_number);

    if ( !report )
	return NULL;

    return report -> amount_comparison_list;
}

/**
 * set the amount_comparison_list
 *
 * \param report_number number of the report
 * \param amount_comparison_list
 *
 * \return TRUE if ok
 * */
gboolean gsb_data_report_set_amount_comparison_list ( gint report_number,
                        GSList *amount_comparison_list )
{
    ReportStruct *report;

    report = gsb_data_report_get_structure (report_number);

    if ( !report )
	return FALSE;

    report -> amount_comparison_list = amount_comparison_list;

    return TRUE;
}

/**
 * get the  method_of_payment_list
 * this is a list of string of method of payment we want to see in the report
 *
 * \param report_number the number of the report
 *
 * \return the method_of_payment_list  of the report, -1 if problem
 * */
GSList *gsb_data_report_get_method_of_payment_list ( gint report_number )
{
    ReportStruct *report;

    report = gsb_data_report_get_structure (report_number);

    if ( !report )
	return NULL;

    return report -> method_of_payment_list;
}

/**
 * set the method_of_payment_list
 *
 * \param report_number number of the report
 * \param method_of_payment_list
 *
 * \return TRUE if ok
 * */
gboolean gsb_data_report_set_method_of_payment_list ( gint report_number,
                        GSList *method_of_payment_list )
{
    ReportStruct *report;

    report = gsb_data_report_get_structure (report_number);

    if ( !report )
        return FALSE;

    report -> method_of_payment_list = method_of_payment_list;

    return TRUE;
}



/**
 * dupplicate the report
 *
 * \param report_number
 *
 * \return number of the new report
 * */
gint gsb_data_report_dup ( gint report_number )
{
    gint new_report_number;
    ReportStruct *report;
    ReportStruct *new_report;
    GSList *list_tmp;

    new_report_number = gsb_data_report_new ( NULL );

    report = gsb_data_report_get_structure (report_number);

    if ( !report )
	return 0;

    new_report = gsb_data_report_get_structure (new_report_number);

    if ( !new_report )
	return 0;

    memcpy ( new_report,
	     report,
	     sizeof ( ReportStruct ));

    /* We have to overwrite report number to be sure this won't
     * duplicate old report number. */
    new_report -> report_number = new_report_number;

    /* we had to duplicate the strings, dates and lists */

    if( report -> report_name )
	new_report -> report_name = my_strdup ( report -> report_name );
    new_report -> financial_year_list = g_slist_copy ( report -> financial_year_list );
    new_report -> sorting_type = g_slist_copy ( report -> sorting_type );
    new_report -> account_numbers = g_slist_copy ( report -> account_numbers );
    new_report -> transfer_account_numbers = g_slist_copy ( report -> transfer_account_numbers );
    new_report -> categ_select_struct = gsb_data_report_copy_categ_budget_struct ( report -> categ_select_struct );
    new_report -> budget_select_struct = gsb_data_report_copy_categ_budget_struct ( report -> budget_select_struct );
    new_report -> payee_numbers = g_slist_copy ( report -> payee_numbers );
    new_report -> method_of_payment_list = g_slist_copy ( report -> method_of_payment_list );

    new_report -> personal_date_start = gsb_date_copy ( report -> personal_date_start );
    new_report -> personal_date_end = gsb_date_copy ( report -> personal_date_end );

    list_tmp = report -> text_comparison_list;
    new_report -> text_comparison_list = NULL;

    while ( list_tmp )
    {
	gint text_comparison_number;

	text_comparison_number = gsb_data_report_text_comparison_dup ( GPOINTER_TO_INT (list_tmp -> data));
	gsb_data_report_text_comparison_set_report_number ( text_comparison_number,
							    new_report_number );
	new_report -> text_comparison_list = g_slist_append ( new_report -> text_comparison_list,
							      GINT_TO_POINTER (text_comparison_number));
	list_tmp = list_tmp -> next;
    }

    list_tmp = report -> amount_comparison_list;
    new_report -> amount_comparison_list = NULL;

    while ( list_tmp )
    {
	gint amount_comparison_number;

	amount_comparison_number = gsb_data_report_amount_comparison_dup ( GPOINTER_TO_INT (list_tmp -> data));
	gsb_data_report_amount_comparison_set_report_number ( amount_comparison_number,
							      new_report_number );
	new_report -> amount_comparison_list = g_slist_append ( new_report -> amount_comparison_list,
								GINT_TO_POINTER (amount_comparison_number));
	list_tmp = list_tmp -> next;
    }

    return new_report_number;
}



/**
 * check the position of the 2 reports in the list and
 * return -1 if first report before second (and +1 else)
 *
 * \param report_number_1
 * \param report_number_2
 *
 * \return -1 if report_number_1 before, report_number_2, and +1 else, 0 if one of report doesn't exist
 * */
gint gsb_data_report_compare_position ( gint report_number_1,
                        gint report_number_2 )
{
    gint pos_1, pos_2;
    ReportStruct *report_1;
    ReportStruct *report_2;

    report_1 = gsb_data_report_get_structure ( report_number_1 );
    report_2 = gsb_data_report_get_structure ( report_number_2 );

    if (!report_1
	||
	!report_2 )
	return 0;

    pos_1 = g_slist_index (report_list, report_1);
    pos_2 = g_slist_index (report_list, report_2);
    if (pos_1 < pos_2)
	return -1;
    else
	return 1;
}


/**
 * change the position of an report in the list of reports
 *
 * \param report_number		the report we want to move
 * \param dest_report_number	the report before we want to move, or -1 to set at the end of list
 *
 * \return FALSE
 * */
gboolean gsb_data_report_move_report ( gint report_number,
                        gint dest_report_number )
{
    ReportStruct *report;

    report = gsb_data_report_get_structure ( report_number );

    if (!report )
	return FALSE;

    /* first, remove the report from the list */
    report_list = g_slist_remove ( report_list,
				   report );

    if (dest_report_number != -1)
    {
	GSList *tmp_list;

	tmp_list = report_list;
	while (tmp_list)
	{
	    ReportStruct *report_tmp;

	    report_tmp = tmp_list -> data;

	    if ( report_tmp && report_tmp -> report_number == dest_report_number)
	    {
		report_list = g_slist_insert_before ( report_list,
						      tmp_list,
						      report );
		return FALSE;
	    }
	    tmp_list = tmp_list -> next;
	}
    }

    /* set the report at the end of list */
    report_list = g_slist_append ( report_list,
				   report );
    return FALSE;
}


/**
 * get a category/budget and sub-category/budget number and check if they are selected
 * in the report
 *
 * \param list_struct_report	a GSList of CategBudgetSel (either categ, either budget)
 * \param div_number	categ or budget number
 * \param sub_div_number	sub-categ or sub-budget number
 *
 * \return TRUE : the couple categ/sub-categ or budget/sub-budget exist, FALSE : it is not in that report
 * */
gboolean gsb_data_report_check_categ_budget_in_report ( GSList *list_struct_report,
                        gint div_number,
                        gint sub_div_number )
{
    GSList *tmp_list;

    tmp_list = list_struct_report;
    while (tmp_list)
    {
	CategBudgetSel *categ_budget_struct = tmp_list -> data;

	if (categ_budget_struct -> div_number == div_number)
	{
	    /* found the categ, now check sub-categ */
	    GSList *sub_categ_budget_list = categ_budget_struct -> sub_div_numbers;

	    if (g_slist_find (sub_categ_budget_list, GINT_TO_POINTER (sub_div_number)))
		return TRUE;
	    else
		/* can return FALSE, needn't to check other categories */
		return FALSE;
	}
	tmp_list = tmp_list -> next;
    }
    return FALSE;
}


/**
 * get the date_select_value
 *
 * \param report_number the number of the report
 *
 * \return the date_select_value, -1 if problem
 * */
gint gsb_data_report_get_date_select_value ( gint report_number )
{
    ReportStruct *report;

    report = gsb_data_report_get_structure ( report_number );

    if ( !report )
        return -1;

    return report -> date_select_value;
}

/**
 * set the date_select_value
 *
 * \param report_number number of the report
 * \param date_select_value
 *
 * \return TRUE if ok
 * */
gboolean gsb_data_report_set_date_select_value ( gint report_number,
                        gint date_select_value )
{
    ReportStruct *report;

    report = gsb_data_report_get_structure ( report_number );

    if ( !report )
        return FALSE;

    report -> date_select_value = date_select_value;

    return TRUE;
}


/**
 * get the  show_m
 *
 * \param report_number the number of the report
 *
 * \return the show_m  of the report, -1 if problem
 * */
gint gsb_data_report_get_show_m ( gint report_number )
{
    ReportStruct *report;

    report = gsb_data_report_get_structure ( report_number );

    if ( !report )
        return -1;

    return report -> show_m;
}


/**
 * set the show_m
 *
 * \param report_number number of the report
 * \param show_m
 *
 * \return TRUE if ok
 * */
gboolean gsb_data_report_set_show_m ( gint report_number,
                        gint show_m )
{
    ReportStruct *report;

    report = gsb_data_report_get_structure ( report_number );

    if ( !report )
        return FALSE;

    report -> show_m = show_m;

    return TRUE;
}


/**
 * get the  show_p
 *
 * \param report_number the number of the report
 *
 * \return the show_p  of the report, -1 if problem
 * */
gint gsb_data_report_get_show_p ( gint report_number )
{
    ReportStruct *report;

    report = gsb_data_report_get_structure ( report_number );

    if ( !report )
        return -1;

    return report -> show_p;
}

/**
 * set the show_p
 *
 * \param report_number number of the report
 * \param show_p
 *
 * \return TRUE if ok
 * */
gboolean gsb_data_report_set_show_p ( gint report_number,
                        gint show_p )
{
    ReportStruct *report;

    report = gsb_data_report_get_structure ( report_number );

    if ( !report )
        return FALSE;

    report -> show_p = show_p;

    return TRUE;
}


/**
 * get the  show_r
 *
 * \param report_number the number of the report
 *
 * \return the show_r  of the report, -1 if problem
 * */
gint gsb_data_report_get_show_r ( gint report_number )
{
    ReportStruct *report;

    report = gsb_data_report_get_structure ( report_number );

    if ( !report )
        return -1;

    return report -> show_r;
}

/**
 * set the show_r
 *
 * \param report_number number of the report
 * \param show_r
 *
 * \return TRUE if ok
 * */
gboolean gsb_data_report_set_show_r ( gint report_number,
                        gint show_r )
{
    ReportStruct *report;

    report = gsb_data_report_get_structure ( report_number );

    if ( !report )
        return FALSE;

    report -> show_r = show_r;

    return TRUE;
}


/**
 * get the  show_t
 *
 * \param report_number the number of the report
 *
 * \return the show_t  of the report, -1 if problem
 * */
gint gsb_data_report_get_show_t ( gint report_number )
{
    ReportStruct *report;

    report = gsb_data_report_get_structure ( report_number );

    if ( !report )
        return -1;

    return report -> show_t;
}


/**
 * set the show_t
 *
 * \param report_number number of the report
 * \param show_t
 *
 * \return TRUE if ok
 * */
gboolean gsb_data_report_set_show_t ( gint report_number,
                        gint show_t )
{
    ReportStruct *report;

    report = gsb_data_report_get_structure ( report_number );

    if ( !report )
        return FALSE;

    report -> show_t = show_t;

    return TRUE;
}


/**
 * free the account_numbers_list
 *
 * \param report_number number of the report
 *
 * \return
 * */
void gsb_data_report_free_account_numbers_list ( gint report_number )
{
    ReportStruct *report;

    report = gsb_data_report_get_structure ( report_number );

    if ( !report )
        return;
    if ( report -> account_numbers )
        g_slist_free ( report -> account_numbers );
    report -> account_numbers = NULL;
}


/**
 * free the transfert_account_numbers_list
 *
 * \param report_number number of the report
 *
 * \return
 * */
void gsb_data_report_free_transfer_account_numbers_list ( gint report_number )
{
    ReportStruct *report;

    report = gsb_data_report_get_structure ( report_number );

    if ( !report )
        return;
    if ( report -> account_numbers )
        g_slist_free ( report -> transfer_account_numbers );
    report -> transfer_account_numbers = NULL;
}


/**
 * free the method_of_payment_list
 *
 * \param report_number number of the report
 *
 * \return
 * */
void gsb_data_report_free_method_of_payment_list ( gint report_number )
{
    ReportStruct *report;

    report = gsb_data_report_get_structure ( report_number );

    if ( !report )
        return;

    if ( report->method_of_payment_list )
        g_slist_free ( report->method_of_payment_list );

    report->method_of_payment_list = NULL;
}


/**
 * free the sorting_type_list
 *
 * \param report_number number of the report
 *
 * \return
 * */
void gsb_data_report_free_sorting_type_list ( gint report_number )
{
    ReportStruct *report;

    report = gsb_data_report_get_structure ( report_number );

    if ( !report )
        return;

    if ( report->sorting_type )
        g_slist_free ( report->sorting_type );

    report->sorting_type = NULL;
}


/**
 * get the  ignore_archive 0 by default
 *
 * \param report_number the number of the report
 *
 * \return the ignore_archive  of the report, -1 if problem
 * */
gint gsb_data_report_get_ignore_archives ( gint report_number )
{
    ReportStruct *report;

    report = gsb_data_report_get_structure ( report_number );

    if ( !report )
        return -1;

    return report -> ignore_archives;
}


/**
 * set the ignore_archive
 *
 * \param report_number number of the report
 * \param ignore_archive
 *
 * \return TRUE if ok
 * */
gboolean gsb_data_report_set_ignore_archives ( gint report_number,
                        gint ignore_archives )
{
    ReportStruct *report;

    report = gsb_data_report_get_structure ( report_number );

    if ( !report )
        return FALSE;

    report -> ignore_archives = ignore_archives;

    return TRUE;
}


/**
 * cherche la valeur d'ignore_archives en fonction de l'état
 *
 * \param report_number de l'état dont on veut déterminer ignore_archives
 *
 * \return ignore_archives
 * */
gint gsb_data_report_test_ignore_archives ( gint report_number )
{
    gint ignore_archives = 0;       /* valeur par défaut */

    if ( gsb_data_report_get_use_financial_year ( report_number ) )
    {
        gint fyear_number;
        gint previous_fyear_number;
        GSList *tmp_list;

        /* get the current financial year */
        fyear_number = gsb_data_fyear_get_from_date ( gdate_today ( ) );

        switch ( gsb_data_report_get_financial_year_type ( report_number ) )
        {
        case 1:
            /* want the current financial year */
            if ( fyear_number )
                ignore_archives = !gsb_data_archive_get_from_fyear ( fyear_number );
            break;
        case 2:
            /* want the previous financial year */
            previous_fyear_number = gsb_data_fyear_get_previous_financial_year ( fyear_number );

            /* here, last_fyear_number is on the last financial year */
            if ( previous_fyear_number )
                ignore_archives = !gsb_data_archive_get_from_fyear ( previous_fyear_number );
            break;
        case 3:
            tmp_list = gsb_data_report_get_financial_year_list ( report_number );

            while (tmp_list )
            {
                fyear_number = GPOINTER_TO_INT ( tmp_list->data );

                if ( gsb_data_archive_get_from_fyear ( fyear_number ) )
                    return 0;

                tmp_list = tmp_list->next;
            }
            ignore_archives = 1;
            break;
        }
    }
    else
    {
        GDate *tmp_date;
        GDate *tmp_date2;

        switch ( gsb_data_report_get_date_type (report_number))
        {
        case 0:
            /* toutes dates */
            ignore_archives = 0;
            break;
        case 1:
            /* plage perso */
            tmp_date = gsb_data_report_get_personal_date_start ( report_number );
            if ( gsb_data_archive_get_from_date ( tmp_date ) )
            {
                ignore_archives = 0;
                break;
            }

            tmp_date = gsb_data_report_get_personal_date_end ( report_number );
            if ( gsb_data_archive_get_from_date ( tmp_date ) )
            {
                ignore_archives = 0;
                break;
            }
            ignore_archives = 1;
            break;
        case 2:
            /* cumul à ce jour */
            ignore_archives = 0;
            break;
        case 3:
            /* mois en cours */
            tmp_date2 = gdate_today ();
            g_date_set_day ( tmp_date2, 1 );
            if ( gsb_data_archive_get_from_date ( tmp_date2 ) )
            {
                ignore_archives = 0;
                g_date_free ( tmp_date2 );
                break;
            }
            tmp_date = gsb_date_get_last_day_of_month ( tmp_date2 );
            if ( gsb_data_archive_get_from_date ( tmp_date ) )
            {
                ignore_archives = 0;
                g_date_free ( tmp_date );
                g_date_free ( tmp_date2 );
                break;
            }
            ignore_archives = 1;

            g_date_free ( tmp_date );
            g_date_free ( tmp_date2 );
            break;
        case 4:
            /* année en cours */
            tmp_date = gdate_today ();
            g_date_set_day ( tmp_date, 1 );
            g_date_set_month ( tmp_date, 1 );
            if ( gsb_data_archive_get_from_date ( tmp_date ) )
            {
                ignore_archives = 0;
                g_date_free ( tmp_date );
                break;
            }
            g_date_set_month ( tmp_date, 12 );
            g_date_set_day ( tmp_date, 31 );
            if ( gsb_data_archive_get_from_date ( tmp_date ) )
            {
                ignore_archives = 0;
                g_date_free ( tmp_date );
                break;
            }
            ignore_archives = 1;

            g_date_free ( tmp_date );
            break;
        case 5:
            /* cumul mensuel */
            tmp_date = gdate_today ();
            if ( gsb_data_archive_get_from_date ( tmp_date ) )
            {
                ignore_archives = 0;
                g_date_free ( tmp_date );
                break;
            }
            g_date_set_day ( tmp_date, 1 );
            if ( gsb_data_archive_get_from_date ( tmp_date ) )
            {
                ignore_archives = 0;
                g_date_free ( tmp_date );
                break;
            }
            ignore_archives = 1;

            g_date_free ( tmp_date );
            break;
        case 6:
            /* cumul annuel */
            tmp_date = gdate_today ();
            if ( gsb_data_archive_get_from_date ( tmp_date ) )
            {
                ignore_archives = 0;
                g_date_free ( tmp_date );
                break;
            }
            g_date_set_day ( tmp_date, 1 );
            g_date_set_month ( tmp_date, 1 );
            if ( gsb_data_archive_get_from_date ( tmp_date ) )
            {
                ignore_archives = 0;
                g_date_free ( tmp_date );
                break;
            }
            ignore_archives = 1;

            g_date_free ( tmp_date );
            break;

        case 7:
            /* mois précédent */
            tmp_date2 = gdate_today ();
            g_date_subtract_months ( tmp_date2, 1 );
            g_date_set_day ( tmp_date2, 1 );
            if ( gsb_data_archive_get_from_date ( tmp_date2 ) )
            {
                ignore_archives = 0;
                g_date_free ( tmp_date2 );
                break;
            }
            tmp_date = gsb_date_get_last_day_of_month ( tmp_date2 );
            if ( gsb_data_archive_get_from_date ( tmp_date ) )
            {
                ignore_archives = 0;
                g_date_free ( tmp_date );
                g_date_free ( tmp_date2 );
                break;
            }
            ignore_archives = 1;

            g_date_free ( tmp_date );
            g_date_free ( tmp_date2 );
            break;
        case 8:
            /* année précédente */
            tmp_date = gdate_today ();
            g_date_subtract_years ( tmp_date, 1 );
            g_date_set_day ( tmp_date, 1 );
            g_date_set_month ( tmp_date, 1 );
            if ( gsb_data_archive_get_from_date ( tmp_date ) )
            {
                ignore_archives = 0;
                g_date_free ( tmp_date );
                break;
            }
            g_date_set_month ( tmp_date, 12 );
            g_date_set_day ( tmp_date, 31 );
            if ( gsb_data_archive_get_from_date ( tmp_date ) )
            {
                ignore_archives = 0;
                g_date_free ( tmp_date );
                break;
            }
            ignore_archives = 1;

            g_date_free ( tmp_date );
            break;
        case 9:
            /* 30 derniers jours */
            tmp_date = gdate_today ();
            if ( gsb_data_archive_get_from_date ( tmp_date ) )
            {
                ignore_archives = 0;
                g_date_free ( tmp_date );
                break;
            }
            g_date_subtract_days ( tmp_date, 30 );
            if ( gsb_data_archive_get_from_date ( tmp_date ) )
            {
                ignore_archives = 0;
                g_date_free ( tmp_date );
                break;
            }
            ignore_archives = 1;

            g_date_free ( tmp_date );
            break;
        case 10:
            /* 3 derniers mois */
            tmp_date = gdate_today ();
            if ( gsb_data_archive_get_from_date ( tmp_date ) )
            {
                ignore_archives = 0;
                g_date_free ( tmp_date );
                break;
            }
            g_date_subtract_months ( tmp_date, 3 );
            if ( gsb_data_archive_get_from_date ( tmp_date ) )
            {
                ignore_archives = 0;
                g_date_free ( tmp_date );
                break;
            }
            ignore_archives = 1;

            g_date_free ( tmp_date );
            break;
        case 11:
            /* 6 derniers mois */
            tmp_date = gdate_today ();
            if ( gsb_data_archive_get_from_date ( tmp_date ) )
            {
                ignore_archives = 0;
                g_date_free ( tmp_date );
                break;
            }
            g_date_subtract_months ( tmp_date, 6 );
            if ( gsb_data_archive_get_from_date ( tmp_date ) )
            {
                ignore_archives = 0;
                g_date_free ( tmp_date );
                break;
            }
            ignore_archives = 1;

            g_date_free ( tmp_date );
            break;
        case 12:
            /* 12 derniers mois */
            tmp_date = gdate_today ();
            if ( gsb_data_archive_get_from_date ( tmp_date ) )
            {
                ignore_archives = 0;
                g_date_free ( tmp_date );
                break;
            }
            g_date_subtract_months ( tmp_date, 12 );
            if ( gsb_data_archive_get_from_date ( tmp_date ) )
            {
                ignore_archives = 0;
                g_date_free ( tmp_date );
                break;
            }
            ignore_archives = 1;

            g_date_free ( tmp_date );
            break;
        }
    }

    return ignore_archives;
}

/**
 *
 *
 * \param
 *
 * \return
 **/
gint gsb_data_report_get_compl_name_function (gint report_number)
{
    ReportStruct *report;

    report = gsb_data_report_get_structure (report_number);

    if (!report)
        return 0;

    return report->compl_name_function;
}

/**
 *
 *
 * \param
 *
 * \return
 **/
gboolean gsb_data_report_set_compl_name_function (gint report_number,
												  gint compl_name_function)
{
    ReportStruct *report;

    report = gsb_data_report_get_structure (report_number);

    if (!report)
        return FALSE;

    report->compl_name_function = compl_name_function;

    return TRUE;
}

/**
 *
 *
 * \param
 *
 * \return
 **/
gint gsb_data_report_get_compl_name_position (gint report_number)
{
    ReportStruct *report;

    report = gsb_data_report_get_structure (report_number);

    if (!report)
        return 0;

    return report->compl_name_position;
}

/**
 *
 *
 * \param
 *
 * \return
 **/
gboolean gsb_data_report_set_compl_name_position (gint report_number,
												  gint compl_name_position)
{
    ReportStruct *report;

    report = gsb_data_report_get_structure (report_number);

    if (!report)
        return FALSE;

    report->compl_name_position = compl_name_position;

    return TRUE;
}

/**
 *
 *
 * \param
 *
 * \return
 **/
gboolean gsb_data_report_get_compl_name_used (gint report_number)
{
    ReportStruct *report;

    report = gsb_data_report_get_structure (report_number);

    if (!report)
        return FALSE;

    return report->compl_name_used;
}

/**
 *
 *
 * \param
 *
 * \return
 **/
gboolean gsb_data_report_set_compl_name_used (gint report_number,
											  gint compl_name_used)
{
    ReportStruct *report;

    report = gsb_data_report_get_structure (report_number);

    if (!report)
        return FALSE;

    report->compl_name_used = compl_name_used;

    return TRUE;
}

/**
 * retourne le nom de fichier pour l'export pdf
 *
 * \param
 *
 * \return
 **/
gchar *gsb_data_report_get_export_pdf_name (gint report_number)
{
    ReportStruct *report;

    report = gsb_data_report_get_structure (report_number);

    if (!report)
		return NULL;

    return report->export_pdf_name;
}

/**
 *  fixe le nom de fichier pour l'export pdf
 *
 * \param
 * \param
 *
 * \return
 **/
gboolean gsb_data_report_set_export_pdf_name (gint report_number,
											  gchar *export_pdf_name)
{
    ReportStruct *report;

    report = gsb_data_report_get_structure (report_number);

    if (!report)
        return FALSE;

    report->export_pdf_name = g_strdup (export_pdf_name);

    return TRUE;
}

/**
 *
 *
 * \param
 *
 * \return
 **/
/* Local Variables: */
/* c-basic-offset: 4 */
/* End: */


