/* ************************************************************************** */
/* Ce fichier s'occupe des astuces                                            */
/*                                                                            */
/*     Copyright (C)    2004-2008 Benjamin Drieu (bdrieu@april.org)           */
/*          2009 Pierre Biava (grisbi@pierre.biava.name)                      */
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


#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "include.h"

#define START_INCLUDE
#include "tip.h"
#include "dialog.h"
#include "gsb_automem.h"
#include "structures.h"
#define END_INCLUDE

static const gchar* tips[] =
{
    N_("You can reconcile transactions to match your real financial state as "
    "seen on your bank receipts.  If you just spend 15mn each month reconciling "
    "Grisbi transactions with your bank receipts, you are guaranteed that your "
    "accounts are in perfect shape."),
    N_("Grisbi automatically saves a copy of your work if it crashes.\n\nThus "
    "it is unlikely that you loose any data in case Grisbi encounters a "
    "programming error."),
    N_("Grisbi can import Gnucash files natively, then you don't loose your work "
    "when you switch from Gnucash to Grisbi."),
    N_("You can organize transactions into budgetary lines, which are budgetary "
    "items (why money is spent) that can help you classify your spendings.\n\n"
    "Combined with categories (how money is spend), budgetary lines are a "
    "powerful way to analyse budget and to make reports."),
    N_("You can create categories or budgetary lines directly by typing them in "
    "transaction form.\n\nIf you write a category or a budgetary line which name "
    "contains a ':', a sub-category or sub-budgetary line will be created as "
    "well."),
    N_("You can encrypt your Grisbi files to improve your privacy.  Just select "
    "the 'Encrypt Grisbi file' in preferences window and enter a password next "
    "time you save your file.\n\nEncryption is irreversible, if you loose your "
    "password, there is NO WAY to restore your Grisbi accounts.  Use with "
    "caution !"),
    N_("You can efficiently manage your professional expenses with Grisbi.\n\n"
    "Create a <i>Professional expenses</i> budgetary line and put all "
    "professional expenses there.  When you are paid, split your salary as a "
    "split of transaction, with both a <i>Salary</i> part and a <i>Profesionnal "
    "expenses</i> part.\n\nThen, you can create a new report that sums up "
    "transactions from the <i>Profesionnal expenses</i> budgetary line and see "
    "the balance."),
    N_("You can use the import facility to automatically reconcile transactions "
    "against a QIF or OFX file from your online bank."),
    N_("You can configure the contents of the transaction list.\n\nRight-click "
    "on a cell of the transaction list and select the <i>Change cell content</i> "
    "option.  Then select any content you want and configure transaction list to "
    "your needs."),
    N_("You can configure fields displayed in the transaction form.\n\nGo to the "
    "<i>Preferences</i> dialog and select the <i>Form content</i> option.  Then "
    "you can click on fields to be displayed and drag them through the form to "
    "adjust their position."),
    N_("You can hide annoying dialogs like this one, by clicking the <i>\'Do not "
    "show this message again\'</i> checkbox in this dialog.\n\nYou can then "
    "activate them again by going to the <i>Preferences</i> window and then to "
    "the <i>Dialog and messages</i> option."),
    N_("By double cliking on a date field, you can pop up a calendar to select "
    "a date."),
    N_("There are shortcuts in date fields, to avoid typing date or selecting "
    "it with mouse.\n\n<b>+</b> and <b>-</b> increment and decrement date of one "
    "day, <i>Page Up</i> and <i>Page Down</i> increment and decrement date of "
    "one month ... and there are more, check out the manual."),
    N_("You can archive previous financial years to speed up Grisbi and "
    "possibily put old transactions into separate files for archival.  Use "
    "'File/Archive transaction' menu to achieve this."),
    N_("If unused payees have accumulated in the course of time due to bank "
    "imports, you can remove all of them in one click!  Just go to the "
    "<i>Payees</i> page and click on the 'Remove unused payees' button."),
    N_("You can sort the transaction list according to any criteria.  Click on "
    "the column titles to sort transactions differently.  For example, you can "
    "sort transactions by payee or category name."),
    N_("You can reorder accounts in the navigation list using drag &amp; drop."),
    N_("You can have future scheduled transactions 'executed' before their date, "),
    N_("No more tip available !  Please send any tips you know to "
    "grisbi-devel@lists.sourceforge.net so that we can include them in next "
    "version for all users.")
};

/** 
 * Display a tip forcefully, even if show_tip option has been disabled.
 */
void force_display_tip (  )
{
    display_tip ( TRUE );
}



/**
 * Display a tip.
 *
 * \param force  Forcefully display the tip even if show_tip option
 *		 has been disabled.
 */
void display_tip ( gboolean force )
{
    GtkWidget * checkbox;
    GtkWidget * dialog = NULL;
    GtkWidget *btn_back, *btn_forward, *btn_close;
    gchar *tmpstr;

    if ( !force && !etat.show_tip )
        return;

    etat.last_tip = CLAMP (etat.last_tip+1, 0, sizeof(tips)/sizeof(gpointer)-1);

    dialog = dialogue_special_no_run ( GTK_MESSAGE_INFO, GTK_BUTTONS_NONE,
                        make_hint ( _("Did you know that..."),
                        /* We use the grisbi-tips catalog */
                        g_dgettext(NULL, tips[etat.last_tip]) ) );
    gtk_window_set_modal ( GTK_WINDOW ( dialog ), FALSE );

    checkbox = gsb_automem_checkbutton_new ( _("Display tips at next start"), 
                        &(etat.show_tip), NULL, NULL );
    gtk_box_pack_start ( GTK_BOX ( GTK_DIALOG(dialog) -> vbox ), checkbox, FALSE, FALSE, 6 );
    gtk_widget_show ( checkbox );

    btn_back =    gtk_dialog_add_button (GTK_DIALOG(dialog), GTK_STOCK_GO_BACK, 1);
    btn_forward = gtk_dialog_add_button (GTK_DIALOG(dialog), GTK_STOCK_GO_FORWARD, 2);
    btn_close =   gtk_dialog_add_button (GTK_DIALOG(dialog), GTK_STOCK_CLOSE, 3);
 
    /* gtk_widget_set_size_request ( dialog, 450, -1 ); */
    /* We iterate as user can select several tips. */
    while ( TRUE )
    {
    if (etat.last_tip == sizeof(tips)/sizeof(gpointer)-1)
        gtk_widget_set_sensitive (btn_forward, FALSE);
    if (etat.last_tip == 0)
        gtk_widget_set_sensitive (btn_back, FALSE);

    switch ( gtk_dialog_run ( GTK_DIALOG(dialog) ) )
    {
        case 1:
        if ( etat.last_tip > 0 )
            etat.last_tip--;
        gtk_widget_set_sensitive (btn_forward, TRUE); 
        tmpstr = g_strconcat ( make_pango_attribut (
                        "size=\"larger\" weight=\"bold\"", _("Did you know that...") ),
                        "\n\n",
                        g_dgettext (NULL, tips[etat.last_tip] ),
                        NULL );

        gtk_label_set_markup ( GTK_LABEL ( GTK_MESSAGE_DIALOG(dialog) -> label ),
                        tmpstr );
        g_free ( tmpstr );
        break;

        case 2:
        if (etat.last_tip < sizeof(tips)/sizeof(gpointer)-1)
            etat.last_tip++;
        tmpstr = g_strconcat ( make_pango_attribut (
                        "size=\"larger\" weight=\"bold\"", _("Did you know that...") ),
                        "\n\n",
                        g_dgettext (NULL, tips[etat.last_tip] ),
                        NULL );

        gtk_label_set_markup ( GTK_LABEL ( GTK_MESSAGE_DIALOG(dialog) -> label ),
                        tmpstr );
        g_free ( tmpstr );
        gtk_widget_set_sensitive (btn_back, TRUE);
        break;

        default:
        gtk_widget_destroy ( dialog );
        return;
    }
    }
}


/* Local Variables: */
/* c-basic-offset: 4 */
/* End: */
