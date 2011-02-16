/* ************************************************************************** */
/*                                                                            */
/*     Copyright (C)    2000-2009 Cédric Auger (cedric@grisbi.org)            */
/*          2004-2009 Benjamin Drieu (bdrieu@april.org)                       */
/*                      2009 Pierre Biava (grisbi@pierre.biava.name)          */
/*      2009 Thomas Peel (thomas.peel@live.fr)                                */
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
 * \file gsb_account_property.c
 * work with the property page of the accounts
 */


#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "include.h"



/*START_INCLUDE*/
#include "gsb_account_property.h"
#include "dialog.h"
#include "gsb_account.h"
#include "gsb_autofunc.h"
#include "gsb_bank.h"
#include "utils_buttons.h"
#include "gsb_currency.h"
#include "gsb_data_account.h"
#include "gsb_data_bank.h"
#include "gsb_data_currency.h"
#include "gsb_data_transaction.h"
#include "utils_editables.h"
#include "gsb_file.h"
#include "gsb_form.h"
#include "gsb_form_scheduler.h"
#include "navigation.h"
#include "accueil.h"
#include "menu.h"
#include "gsb_scheduler_list.h"
#include "main.h"
#include "traitement_variables.h"
#include "utils_str.h"
#include "utils.h"
#include "tiers_onglet.h"
#include "categories_onglet.h"
#include "imputation_budgetaire.h"
#include "transaction_list.h"
#include "structures.h"
#include "gsb_transactions_list.h"
#include "erreur.h"
/*END_INCLUDE*/

/*START_STATIC*/
static gboolean gsb_account_property_change_currency ( GtkWidget *combobox,
                        gpointer null );
static gboolean gsb_account_property_changed ( GtkWidget *widget,
                        gint *p_origin  );
static gboolean gsb_account_property_changed_bank_label ( GtkWidget *combobox,
                        gpointer null );
static gboolean gsb_account_property_focus_out ( GtkWidget *widget,
                        GdkEventFocus *event,
                        gint *p_origin );
static void gsb_account_property_iban_clear_label_data ( void );
static gint gsb_account_property_iban_control_iban ( gchar *iban );
static void gsb_account_property_iban_delete_text ( GtkEditable *entry,
                        gint start_pos,
                        gint end_pos,
                        GtkWidget *combobox );
static void gsb_account_property_iban_display_bank_data ( gint current_account,
                        gint bank_number );
static gboolean gsb_account_property_iban_focus_in_event ( GtkWidget *entry,
                        GdkEventFocus *ev,
                        gpointer data );
static gboolean gsb_account_property_iban_focus_out_event ( GtkWidget *entry,
                        GdkEventFocus *ev,
                        gpointer data );
static struct iso_13616_iban *gsb_account_property_iban_get_struc ( gchar *pays );
static void gsb_account_property_iban_insert_text ( GtkEditable *entry,
                        gchar *text,
                        gint length,
                        gint *position,
                        GtkWidget *combobox );
static gboolean gsb_account_property_iban_key_press_event ( GtkWidget *entry,
                        GdkEventKey *ev,
                        gpointer data );
static gboolean gsb_account_property_iban_set_bank_from_iban ( gchar *iban );
static void gsb_account_property_iban_set_iban ( const gchar *iban );
static void gsb_account_property_iban_switch_bank_data ( gboolean sensitive );
/*END_STATIC*/

struct iso_13616_iban iso_13616_ibans [] = {
    { "XX", "XXkk XXXX XXXX XXXX XXXX XXXX XXXX XXXX XX", 34 },
    { "AD", "ADkk BBBB SSSS CCCC CCCC CCCC", 24 },
    { "AT", "ATkk BBBB BCCC CCCC CCCC", 20 },
    { "BA", "BAkk BBBS SSCC CCCC CCKK", 20 },
    { "BE", "BEkk BBBC CCCC CCKK", 16 },
    { "BG", "BGkk BBBB SSSS DDCC CCCC CC", 22 },
    { "CH", "CHkk BBBB BCCC CCCC CCCC C", 21 },
    { "CY", "CYkk BBBS SSSS CCCC CCCC CCCC CCCC", 28 },
    { "CZ", "CZkk BBBB CCCC CCCC CCCC CCCC", 24 },
    { "DE", "DEkk BBBB BBBB CCCC CCCC CC", 22 },
    { "DK", "DKkk BBBB CCCC CCCC CC", 18 },
    { "EE", "EEkk BBBB CCCC CCCC CCCK", 20 },
    { "ES", "ESkk BBBB GGGG KKCC CCCC CCCC", 24 },
    { "FI", "FIkk BBBB BBCC CCCC CK", 18 },
    { "FO", "FOkk CCCC CCCC CCCC CC", 18 },
    { "FR", "FRkk BBBB BGGG GGCC CCCC CCCC CKK", 27 },
    { "GB", "GBkk BBBB SSSS SSCC CCCC CC", 22 },
    { "GI", "GIkk BBBB CCCC CCCC CCCC CCC", 23 },
    { "GR", "GRkk BBB BBBB CCCC CCCC CCCC CCCC", 27 },
    { "HR", "HRkk BBBB BBBC CCCC CCCC C", 21 },
    { "IE", "IEkk AAAA BBBB BBCC CCCC CC", 22 },
    { "LU", "LUkk BBBC CCCC CCCC CCCC", 20 },
    { "NL", "NLkk BBBB CCCC CCCC CC", 18 },
    { "PT", "PTkk BBBB BBBB CCCC CCCC CCCK K", 25 },
    { "SE", "SEkk BBBB CCCC CCCC CCCC CCCC", 24 },
    { NULL },
};

static GtkWidget *edit_bank_button = NULL;
GtkWidget *detail_nom_compte = NULL;
static GtkWidget *detail_type_compte = NULL;
static GtkWidget *detail_titulaire_compte = NULL;
static GtkWidget *detail_adresse_titulaire = NULL;
static GtkWidget *bank_list_combobox = NULL;
static GtkWidget *label_code_banque = NULL;
static GtkWidget *detail_guichet = NULL;
static GtkWidget *detail_no_compte = NULL;
static GtkWidget *detail_cle_compte = NULL;
GtkWidget *detail_devise_compte = NULL;
static GtkWidget *detail_compte_cloture = NULL;
static GtkWidget *detail_solde_init = NULL;
static GtkWidget *detail_solde_mini_autorise = NULL;
static GtkWidget *detail_solde_mini_voulu = NULL;
static GtkWidget *detail_commentaire = NULL;
static GtkWidget *button_holder_address = NULL;
static GtkWidget *bouton_icon = NULL;
static GtkWidget *detail_IBAN = NULL;
static GtkWidget *label_code_bic = NULL;
static GtkWidget *label_guichet = NULL;
static GtkWidget *label_no_compte = NULL;
static GtkWidget *label_cle_compte = NULL;


enum origin
{
    PROPERTY_NAME = 0,
    PROPERTY_KIND,
    PROPERTY_CURRENCY,
    PROPERTY_CLOSED_ACCOUNT,
    PROPERTY_INIT_BALANCE,
    PROPERTY_WANTED_BALANCE,
    PROPERTY_HOLDER_NAME,
};

/*START_EXTERN*/
extern gint mise_a_jour_fin_comptes_passifs;
extern gint mise_a_jour_liste_comptes_accueil;
extern gint mise_a_jour_liste_echeances_manuelles_accueil;
extern gint mise_a_jour_soldes_minimaux;
extern gsb_real null_real;
/*END_EXTERN*/


/**
 * create the page of account property
 *
 * \param
 *
 * \return the page
 * */
GtkWidget *gsb_account_property_create_page ( void )
{
    GtkWidget *onglet, *vbox, *scrolled_window, *hbox, *vbox2;
    GtkWidget *label, *scrolled_window_text, *paddingbox;
    GtkSizeGroup * size_group;
    GtkWidget *align;

    devel_debug ( NULL );

    /* la fenetre ppale est une vbox avec les détails en haut et appliquer en bas */
    onglet = gtk_vbox_new ( FALSE, 5 );
    gtk_widget_set_name ( onglet, "properties_page" );
    gtk_container_set_border_width ( GTK_CONTAINER ( onglet ), 10 );

    size_group = gtk_size_group_new ( GTK_SIZE_GROUP_HORIZONTAL );

    /* Création du bouton pour modifier l'icône de compte. C'est un moyen de
     * contourner le bug du gtk_viewport */
    align = gtk_alignment_new (0.5, 0.0, 0.0, 0.0);
    bouton_icon = gtk_button_new ( );
    gtk_widget_set_size_request ( bouton_icon, -1, 40 );
    gtk_button_set_relief ( GTK_BUTTON ( bouton_icon ), GTK_RELIEF_NORMAL );
    gtk_container_add ( GTK_CONTAINER ( align ), bouton_icon );
    gtk_box_pack_start ( GTK_BOX ( onglet ), align, FALSE, FALSE, 0);

    /* partie du haut avec les détails du compte */
    scrolled_window = gtk_scrolled_window_new ( FALSE, FALSE );
    gtk_scrolled_window_set_policy ( GTK_SCROLLED_WINDOW ( scrolled_window ),
                        GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC );
    gtk_box_pack_start ( GTK_BOX ( onglet ), scrolled_window, TRUE, TRUE, 0 );

    vbox = gtk_vbox_new ( FALSE, 5 );

    gtk_scrolled_window_add_with_viewport ( GTK_SCROLLED_WINDOW ( scrolled_window ), vbox );
    gtk_viewport_set_shadow_type ( GTK_VIEWPORT ( GTK_BIN ( scrolled_window ) -> child ),
                        GTK_SHADOW_NONE );

    /* création de la ligne des détails du compte */
    paddingbox = new_paddingbox_with_title (vbox, FALSE, _("Account details"));
    
   /* création de la ligne du nom du compte */
    hbox = gtk_hbox_new ( FALSE, 6 );
    gtk_box_pack_start ( GTK_BOX(paddingbox), hbox, FALSE, FALSE, 0 );

    label = gtk_label_new ( _("Account name: ") );
    gtk_misc_set_alignment ( GTK_MISC(label), MISC_LEFT, MISC_VERT_CENTER );
    gtk_size_group_add_widget ( GTK_SIZE_GROUP ( size_group ), label );
    gtk_box_pack_start ( GTK_BOX(hbox), label, FALSE, FALSE, 0);

    detail_nom_compte = gsb_autofunc_entry_new ( NULL,
                        G_CALLBACK ( gsb_account_property_changed ),
                        GINT_TO_POINTER ( PROPERTY_NAME ),
                        G_CALLBACK ( gsb_data_account_set_name ),
                        0 );
    g_signal_connect ( G_OBJECT ( detail_nom_compte ),
			       "focus-out-event",
			       G_CALLBACK ( gsb_account_property_focus_out ),
			       GINT_TO_POINTER ( PROPERTY_NAME ) );

    gtk_box_pack_start ( GTK_BOX ( hbox ), detail_nom_compte, TRUE, TRUE, 0 );

    /* create the box of kind of account */
    hbox = gtk_hbox_new ( FALSE, 6 );
    gtk_box_pack_start ( GTK_BOX ( paddingbox ), hbox, FALSE, FALSE, 0 );

    label = gtk_label_new ( _("Account type: ") );
    gtk_misc_set_alignment ( GTK_MISC(label), MISC_LEFT, MISC_VERT_CENTER );
    gtk_size_group_add_widget ( GTK_SIZE_GROUP ( size_group ), label );
    gtk_box_pack_start ( GTK_BOX(hbox), label, FALSE, FALSE, 0);

    /* create the list of the kind of account combobox */
    detail_type_compte = gsb_autofunc_combobox_new ( gsb_account_property_create_combobox_list (),
                        0,
                        G_CALLBACK (gsb_account_property_changed),
                        GINT_TO_POINTER (PROPERTY_KIND),
                        G_CALLBACK (gsb_data_account_set_kind),
                        0 );
    gtk_box_pack_start ( GTK_BOX(hbox), detail_type_compte, TRUE, TRUE, 0);

    /* create the currency line */
    hbox = gtk_hbox_new ( FALSE, 6 );
    gtk_box_pack_start ( GTK_BOX(paddingbox), hbox, FALSE, FALSE, 0 );

    label = gtk_label_new ( _("Account currency: ") );
    gtk_misc_set_alignment ( GTK_MISC(label), MISC_LEFT, MISC_VERT_CENTER );
    gtk_size_group_add_widget ( GTK_SIZE_GROUP ( size_group ), label );
    gtk_box_pack_start ( GTK_BOX(hbox), label, FALSE, FALSE, 0);

    detail_devise_compte = gsb_autofunc_currency_new (TRUE, 
                        0,
                        G_CALLBACK (gsb_account_property_changed),
                        GINT_TO_POINTER (PROPERTY_CURRENCY),
                        NULL,
                        0 );
    gtk_box_pack_start ( GTK_BOX(hbox), detail_devise_compte, TRUE, TRUE, 0);

    /* create closed account line */
    detail_compte_cloture = gsb_autofunc_checkbutton_new (_("Closed account"),
                        FALSE,
                        G_CALLBACK (gsb_account_property_changed),
                        GINT_TO_POINTER (PROPERTY_CLOSED_ACCOUNT),
                        G_CALLBACK (gsb_data_account_set_closed_account),
                        0 );
    gtk_box_pack_start ( GTK_BOX(paddingbox), detail_compte_cloture, FALSE, FALSE, 0 );

    /* set the callback for the button_icon */
    gtk_button_set_relief ( GTK_BUTTON ( bouton_icon ), GTK_RELIEF_NONE );

    g_signal_connect ( G_OBJECT( bouton_icon ),
                        "clicked",
                        G_CALLBACK(gsb_data_account_change_account_icon),
                        NULL );

    /* création de la ligne du titulaire du compte */
    paddingbox = new_paddingbox_with_title ( vbox, FALSE, _("Account holder"));
    hbox = gtk_hbox_new ( FALSE, 6 );
    gtk_box_pack_start ( GTK_BOX(paddingbox), hbox, FALSE, FALSE, 0 );

    label = gtk_label_new ( _("Holder name: ") );
    gtk_misc_set_alignment ( GTK_MISC(label), MISC_LEFT, MISC_VERT_CENTER );
    gtk_size_group_add_widget ( GTK_SIZE_GROUP ( size_group ), label );
    gtk_box_pack_start ( GTK_BOX(hbox), label, FALSE, FALSE, 0);

    detail_titulaire_compte = gsb_autofunc_entry_new ( NULL,
                        NULL,
                        NULL,
                        G_CALLBACK ( gsb_data_account_set_holder_name ),
                        0);
    g_signal_connect ( G_OBJECT ( detail_titulaire_compte ),
			       "focus-out-event",
			       G_CALLBACK ( gsb_account_property_focus_out ),
			       GINT_TO_POINTER ( PROPERTY_HOLDER_NAME ) );
    gtk_box_pack_start ( GTK_BOX(hbox), detail_titulaire_compte, TRUE, TRUE, 0);

    /* address of the holder line */
    hbox = gtk_hbox_new ( FALSE, 6 );
    gtk_box_pack_start ( GTK_BOX(paddingbox), hbox, FALSE, FALSE, 0 );

    vbox2 = gtk_vbox_new (FALSE, 0);
    gtk_box_pack_start ( GTK_BOX (hbox), vbox2, FALSE, FALSE, 0 );

    /* we need to create first the text_view because used in callbacks */
    detail_adresse_titulaire = gsb_autofunc_textview_new ( NULL,
                        NULL,
                        NULL,
                        G_CALLBACK (gsb_data_account_set_holder_address),
                        0 );

    /* now the checkbutton for different address */
    button_holder_address = gsb_autofunc_checkbutton_new (
                        _("Holder's own address: "), FALSE,
                        G_CALLBACK (gsb_editable_erase_text_view),
                        detail_adresse_titulaire,
                        NULL,
                        0 );
    gtk_button_set_alignment ( GTK_BUTTON (button_holder_address), 0.0, 0.0 );
    gtk_box_pack_start ( GTK_BOX(vbox2), button_holder_address, FALSE, FALSE, 0);

    /* if un-select the holder's button address, we need to erase the tree_view,
     * else issue when come back to that account property */
    g_signal_connect ( G_OBJECT (button_holder_address),
                        "toggled",
                        G_CALLBACK (gsb_button_sensitive_by_checkbutton),
                        detail_adresse_titulaire );

    /* create the text view for holder address */
    scrolled_window_text = gtk_scrolled_window_new ( FALSE, FALSE );
    gtk_scrolled_window_set_policy ( GTK_SCROLLED_WINDOW ( scrolled_window_text ),
                        GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC );
    gtk_scrolled_window_set_shadow_type ( GTK_SCROLLED_WINDOW(scrolled_window_text),
                        GTK_SHADOW_IN );
    gtk_box_pack_start ( GTK_BOX(hbox), scrolled_window_text, TRUE, TRUE, 0);


    /* text view created before */
    gtk_container_add ( GTK_CONTAINER ( scrolled_window_text ), detail_adresse_titulaire );
    gtk_widget_set_sensitive (detail_adresse_titulaire, FALSE);


    /* création de la ligne de l'établissement financier */
    paddingbox = new_paddingbox_with_title ( vbox, FALSE, _("Bank"));
    hbox = gtk_hbox_new ( FALSE, 6 );
    gtk_box_pack_start ( GTK_BOX(paddingbox), hbox, FALSE, FALSE, 0 );

    label = gtk_label_new ( _("Financial institution: ") );
    gtk_misc_set_alignment ( GTK_MISC(label), MISC_LEFT, MISC_VERT_CENTER );
    gtk_size_group_add_widget ( GTK_SIZE_GROUP ( size_group ), label );
    gtk_box_pack_start ( GTK_BOX(hbox), label, FALSE, FALSE, 0);

    bank_list_combobox = gsb_bank_create_combobox ( 0,
                        NULL,
                        NULL,
                        G_CALLBACK (gsb_data_account_set_bank),
                        0 );
    g_signal_connect ( G_OBJECT (bank_list_combobox),
                        "changed",
                        G_CALLBACK (gsb_account_property_changed_bank_label),
                        NULL );
    gtk_box_pack_start ( GTK_BOX(hbox), bank_list_combobox, TRUE, TRUE, 0);

    edit_bank_button = gtk_button_new_from_stock ( GTK_STOCK_EDIT );
    gtk_button_set_relief ( GTK_BUTTON ( edit_bank_button ), GTK_RELIEF_NONE );
    g_signal_connect ( G_OBJECT ( edit_bank_button ),
                        "clicked",
                        G_CALLBACK (gsb_bank_edit_from_button),
                        bank_list_combobox );
    gtk_box_pack_start ( GTK_BOX ( hbox ), edit_bank_button, FALSE, FALSE, 0 );

    /* création du numéro BIC */
    hbox = gtk_hbox_new ( FALSE, 6 );
    gtk_box_pack_start ( GTK_BOX(paddingbox), hbox, FALSE, FALSE, 0 );

    label = gtk_label_new ( _("BIC code: ") );
    gtk_misc_set_alignment ( GTK_MISC(label), MISC_LEFT, MISC_VERT_CENTER );
    gtk_size_group_add_widget ( GTK_SIZE_GROUP ( size_group ), label );
    gtk_box_pack_start ( GTK_BOX(hbox), label, FALSE, FALSE, 0);

    label_code_bic = gtk_label_new ( NULL );
    gtk_misc_set_alignment ( GTK_MISC(label_code_bic), MISC_LEFT, MISC_VERT_CENTER );
    gtk_label_set_justify ( GTK_LABEL(label_code_bic), GTK_JUSTIFY_RIGHT );
    gtk_box_pack_start ( GTK_BOX(hbox), label_code_bic, TRUE, TRUE, 0 );

     /* création de la ligne du numéro IBAN */
    hbox = gtk_hbox_new ( FALSE, 6 );
    gtk_box_pack_start ( GTK_BOX(paddingbox), hbox, FALSE, FALSE, 0 );

    label = gtk_label_new ( _("IBAN number: ") );
    gtk_misc_set_alignment ( GTK_MISC(label), MISC_LEFT, MISC_VERT_CENTER );
    gtk_size_group_add_widget ( GTK_SIZE_GROUP ( size_group ), label );
    gtk_box_pack_start ( GTK_BOX(hbox), label, FALSE, FALSE, 0);

    detail_IBAN = gtk_entry_new ( );
    g_signal_connect ( G_OBJECT (detail_IBAN ), "insert-text",
                        G_CALLBACK (gsb_account_property_iban_insert_text),
                        bank_list_combobox );
    g_signal_connect ( G_OBJECT (detail_IBAN ), "delete-text",
                        G_CALLBACK (gsb_account_property_iban_delete_text),
                        bank_list_combobox );
    g_signal_connect ( G_OBJECT ( detail_IBAN ), "key-press-event",
                        G_CALLBACK ( gsb_account_property_iban_key_press_event ),
                        NULL );
    g_signal_connect ( G_OBJECT ( detail_IBAN ), "focus-in-event",
                        G_CALLBACK ( gsb_account_property_iban_focus_in_event ),
                        NULL );
    g_signal_connect ( G_OBJECT ( detail_IBAN ), "focus-out-event",
                        G_CALLBACK ( gsb_account_property_iban_focus_out_event ),
                        NULL );

    gtk_widget_set_size_request ( detail_IBAN, 350, -1 );
    gtk_box_pack_start ( GTK_BOX ( hbox ), detail_IBAN, FALSE, FALSE, 0 );

    /* create the code of bank */
    hbox = gtk_hbox_new ( FALSE, 6 );
    gtk_box_pack_start ( GTK_BOX(paddingbox), hbox, FALSE, FALSE, 0 );

    label = gtk_label_new ( _("Bank sort code: ") );
    gtk_misc_set_alignment ( GTK_MISC(label), MISC_LEFT, MISC_VERT_CENTER );
    gtk_size_group_add_widget ( GTK_SIZE_GROUP ( size_group ), label );
    gtk_box_pack_start ( GTK_BOX(hbox), label, FALSE, FALSE, 0);

    label_code_banque = gtk_label_new ( NULL );
    gtk_misc_set_alignment ( GTK_MISC(label_code_banque), MISC_LEFT, MISC_VERT_CENTER );
    gtk_label_set_justify ( GTK_LABEL(label_code_banque), GTK_JUSTIFY_RIGHT );
    gtk_box_pack_start ( GTK_BOX(hbox), label_code_banque, TRUE, TRUE, 0 );

    /* création de la ligne du guichet */
    hbox = gtk_hbox_new ( FALSE, 6 );
    gtk_box_pack_start ( GTK_BOX(paddingbox), hbox, FALSE, FALSE, 0 );

    label = gtk_label_new ( _("Bank branch code: ") );
    gtk_misc_set_alignment ( GTK_MISC(label), MISC_LEFT, MISC_VERT_CENTER );
    gtk_size_group_add_widget ( GTK_SIZE_GROUP ( size_group ), label );
    gtk_box_pack_start ( GTK_BOX(hbox), label, FALSE, FALSE, 0);

    label_guichet = gtk_label_new ( NULL );
    gtk_misc_set_alignment ( GTK_MISC(label), MISC_LEFT, MISC_VERT_CENTER );
    gtk_label_set_justify ( GTK_LABEL(label), GTK_JUSTIFY_RIGHT );
    gtk_box_pack_start ( GTK_BOX(hbox), label_guichet, FALSE, FALSE, 0 );

    detail_guichet = gsb_autofunc_entry_new ( NULL,
                        NULL,
                        NULL,
                        G_CALLBACK (gsb_data_account_set_bank_branch_code),
                        0);
    gtk_box_pack_start ( GTK_BOX(hbox), detail_guichet, TRUE, TRUE, 0);

    /* création de la ligne du numéro du compte */
    hbox = gtk_hbox_new ( FALSE, 6 );
    gtk_box_pack_start ( GTK_BOX(paddingbox), hbox, FALSE, FALSE, 0 );

    label = gtk_label_new ( _("Account number / Key: ") );
    gtk_misc_set_alignment ( GTK_MISC(label), MISC_LEFT, MISC_VERT_CENTER );
    gtk_size_group_add_widget ( GTK_SIZE_GROUP ( size_group ), label );
    gtk_box_pack_start ( GTK_BOX(hbox), label, FALSE, FALSE, 0);

    label_no_compte = gtk_label_new ( NULL );
    gtk_misc_set_alignment ( GTK_MISC(label), MISC_LEFT, MISC_VERT_CENTER );
    gtk_label_set_justify ( GTK_LABEL(label), GTK_JUSTIFY_RIGHT );
    gtk_box_pack_start ( GTK_BOX(hbox), label_no_compte, FALSE, FALSE, 0 );

    detail_no_compte = gsb_autofunc_entry_new ( NULL,
                        NULL,
                        NULL,
                        G_CALLBACK (gsb_data_account_set_bank_account_number),
                        0 );
    gtk_box_pack_start ( GTK_BOX ( hbox ), detail_no_compte, TRUE, TRUE, 0 );

    detail_cle_compte = gsb_autofunc_entry_new ( NULL,
                        NULL,
                        NULL,
                        G_CALLBACK (gsb_data_account_set_bank_account_key),
                        0);
    gtk_widget_set_size_request ( detail_cle_compte, 30, -1 );
    gtk_box_pack_start ( GTK_BOX ( hbox ), detail_cle_compte, FALSE, FALSE, 0 );

    label_cle_compte = gtk_label_new ( NULL );
    gtk_misc_set_alignment ( GTK_MISC(label), MISC_LEFT, MISC_VERT_CENTER );
    gtk_label_set_justify ( GTK_LABEL(label), GTK_JUSTIFY_RIGHT );
    gtk_box_pack_start ( GTK_BOX(hbox), label_cle_compte, FALSE, FALSE, 0 );


    /* création de la ligne du solde initial */
    paddingbox = new_paddingbox_with_title ( vbox, FALSE, _("Balances"));

    hbox = gtk_hbox_new ( FALSE, 6 );
    gtk_box_pack_start ( GTK_BOX(paddingbox), hbox, FALSE, FALSE, 0 );

    label = gtk_label_new ( _("Initial balance: ") );
    gtk_misc_set_alignment ( GTK_MISC(label), MISC_LEFT, MISC_VERT_CENTER );
    gtk_size_group_add_widget ( GTK_SIZE_GROUP ( size_group ), label );
    gtk_box_pack_start ( GTK_BOX ( hbox ), label, FALSE, FALSE, 0 );

    detail_solde_init = gsb_autofunc_real_new ( null_real,
                        G_CALLBACK (gsb_account_property_changed), 
                        GINT_TO_POINTER (PROPERTY_INIT_BALANCE),
                        G_CALLBACK (gsb_data_account_set_init_balance), 0);
    gtk_box_pack_start ( GTK_BOX ( hbox ), detail_solde_init, TRUE, TRUE, 0 );


    /* création de la ligne du solde mini autorisé */
    hbox = gtk_hbox_new ( FALSE, 6 );
    gtk_box_pack_start ( GTK_BOX ( paddingbox ), hbox, FALSE, FALSE, 0 );

    label = gtk_label_new ( _("Minimum authorised balance: ") );
    gtk_misc_set_alignment ( GTK_MISC(label), MISC_LEFT, MISC_VERT_CENTER );
    gtk_size_group_add_widget ( GTK_SIZE_GROUP ( size_group ), label );
    gtk_box_pack_start ( GTK_BOX ( hbox ), label, FALSE, FALSE, 0 );

    detail_solde_mini_autorise = gsb_autofunc_real_new ( null_real,
                        G_CALLBACK (gsb_account_property_changed), 
                        GINT_TO_POINTER (PROPERTY_WANTED_BALANCE),
                        G_CALLBACK (gsb_data_account_set_mini_balance_authorized),
                        0);
    gtk_box_pack_start ( GTK_BOX ( hbox ), detail_solde_mini_autorise, TRUE, TRUE, 0 );


    /* création de la ligne du solde mini voulu */
    hbox = gtk_hbox_new ( FALSE, 6 );
    gtk_box_pack_start ( GTK_BOX ( paddingbox ), hbox, FALSE, FALSE, 0 );

    label = gtk_label_new ( _("Minimum desired balance: ") );
    gtk_misc_set_alignment ( GTK_MISC(label), MISC_LEFT, MISC_VERT_CENTER );
    gtk_size_group_add_widget ( GTK_SIZE_GROUP ( size_group ), label );
    gtk_box_pack_start ( GTK_BOX ( hbox ), label, FALSE, FALSE, 0 );

    detail_solde_mini_voulu = gsb_autofunc_real_new (null_real,
                        G_CALLBACK (gsb_account_property_changed), GINT_TO_POINTER (PROPERTY_WANTED_BALANCE),
                        G_CALLBACK (gsb_data_account_set_mini_balance_wanted), 0);
    gtk_box_pack_start ( GTK_BOX ( hbox ), detail_solde_mini_voulu, TRUE, TRUE, 0 );

    /* comments line */
    paddingbox = new_paddingbox_with_title ( vbox, FALSE, _("Comments"));

    scrolled_window_text = gtk_scrolled_window_new ( FALSE, FALSE );
    gtk_scrolled_window_set_policy ( GTK_SCROLLED_WINDOW ( scrolled_window_text ),
                        GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC );
    gtk_scrolled_window_set_shadow_type ( GTK_SCROLLED_WINDOW(scrolled_window_text),
                        GTK_SHADOW_IN );
    gtk_box_pack_start ( GTK_BOX ( paddingbox ), scrolled_window_text, TRUE, TRUE, 5 );

    detail_commentaire = gsb_autofunc_textview_new ( NULL,
                        NULL,
                        NULL,
                        G_CALLBACK (gsb_data_account_set_comment),
                        0);
    gtk_container_add ( GTK_CONTAINER ( scrolled_window_text ), detail_commentaire );

    gtk_widget_show_all ( onglet );

    return ( onglet );
}

/**
 * fill the content of the property of the selected account
 *
 * \param
 *
 * \return
 * */
void gsb_account_property_fill_page ( void )
{
    gint bank_number;
    gint current_account;
    GtkWidget *image;

    devel_debug (NULL);

    current_account = gsb_gui_navigation_get_current_account ();

    gsb_autofunc_entry_set_value (detail_nom_compte,
                        gsb_data_account_get_name (current_account), current_account);

    gsb_autofunc_combobox_set_index (detail_type_compte,
                        gsb_data_account_get_kind (current_account), current_account);

    /* modification pour mettre à jour l'icône du sélecteur d'icône du compte */
    image = gsb_data_account_get_account_icon_image ( current_account );
    gtk_button_set_image ( GTK_BUTTON ( bouton_icon ), image );

    gsb_autofunc_currency_set_currency_number (detail_devise_compte,
                        gsb_data_account_get_currency (current_account), current_account);

    gsb_autofunc_checkbutton_set_value (detail_compte_cloture,
                        gsb_data_account_get_closed_account (current_account), current_account);

    gsb_autofunc_entry_set_value ( detail_titulaire_compte,
                        gsb_data_account_get_holder_name (current_account), current_account );

    gsb_autofunc_checkbutton_set_value ( button_holder_address,
                        gsb_data_account_get_holder_address (current_account) != NULL, 0 );

    gsb_autofunc_textview_set_value ( detail_adresse_titulaire,
                        gsb_data_account_get_holder_address (current_account), current_account );

    /* fill bank information */
    bank_number = gsb_data_account_get_bank (current_account);
    gsb_account_property_set_label_code_bic ( bank_number );

    gsb_account_property_iban_set_iban (
                        gsb_data_account_get_bank_account_iban (current_account) );

    if ( gsb_account_property_iban_set_bank_from_iban (
                        gsb_data_account_get_bank_account_iban (current_account)) )
        gsb_account_property_iban_switch_bank_data ( FALSE );
    else
        gsb_account_property_iban_switch_bank_data ( TRUE );

    gsb_autofunc_real_set ( detail_solde_init,
                        gsb_data_account_get_init_balance (current_account,
                        gsb_data_currency_get_floating_point (
                        gsb_data_account_get_currency ( current_account))), current_account);
    gsb_autofunc_real_set (detail_solde_mini_autorise,
                        gsb_data_account_get_mini_balance_authorized (current_account),
                        current_account);
    gsb_autofunc_real_set (detail_solde_mini_voulu,
                        gsb_data_account_get_mini_balance_wanted (current_account),
                        current_account);

    gsb_autofunc_textview_set_value ( detail_commentaire,
                        gsb_data_account_get_comment (current_account), current_account);
}


/**
 * callback called when change the bank for the account
 * change the bank code label under the combobox
 *
 * \param combobox the combobox containing the banks
 * \param null
 *
 * \return FALSE
 * */
gboolean gsb_account_property_changed_bank_label ( GtkWidget *combobox,
                        gpointer null )
{
    gint bank_number;

    if (!combobox)
        return FALSE;

    bank_number = gsb_bank_list_get_bank_number (combobox);

    gsb_account_property_set_label_code_banque ( bank_number );
    gsb_account_property_set_label_code_bic ( bank_number );

    return FALSE;
}


/**
 * create a list of kind of account to use in a gsb_combo_box or gsb_autofunc_combobox
 * this is an alternance of text (kind of account) and number
 *
 * \param
 *
 * \return a newly allocated GSList
 * */
GSList *gsb_account_property_create_combobox_list ( void )
{
    gchar *text [] = { _("Bank account"), _("Cash account"), _("Liabilities account"), _("Assets account"),NULL };
    gint i = 0;
    GSList *list = NULL;

    while (text[i])
    {
    list = g_slist_append (list, text[i]);
    list = g_slist_append (list, GINT_TO_POINTER (i));
    i++;
    }
    return list;
}



/**
 * called when something changed and need to modify other things
 *
 * \param widget various widget according of the origin
 * \param p_origin the origin as a pointer
 *
 * \return FALSE;
 * */
gboolean gsb_account_property_changed ( GtkWidget *widget,
                        gint *p_origin  )
{
    gint origin = GPOINTER_TO_INT (p_origin);
    gint account_number;
    GtkWidget *image;

    account_number = gsb_gui_navigation_get_current_account ();
    if ( account_number == -1)
        return FALSE;

    switch (origin)
    {
    case PROPERTY_NAME:
        gsb_gui_navigation_update_account ( account_number );
        break;
    case PROPERTY_CLOSED_ACCOUNT:
        gsb_gui_navigation_update_account ( account_number );
        gsb_menu_update_accounts_in_menus ();

        /* update the name of accounts in form */
        gsb_account_update_combo_list ( gsb_form_scheduler_get_element_widget (
                        SCHEDULED_FORM_ACCOUNT), FALSE );

        /* Replace trees contents. */
        remplit_arbre_categ ();
        remplit_arbre_imputation ();
        payee_fill_tree ();
        break;

    case PROPERTY_KIND:
        gsb_gui_navigation_update_account ( account_number );
        image = gsb_data_account_get_account_icon_image ( account_number );
        gtk_button_set_image ( GTK_BUTTON ( bouton_icon ), image );
        gsb_form_clean(gsb_form_get_account_number ());
        break;

    case PROPERTY_CURRENCY:
        gsb_account_property_change_currency (widget, NULL);
        break;

    case PROPERTY_INIT_BALANCE:
        /* as we changed the initial balance, we need to recalculate the amount
         * of each line in the list */
        transaction_list_set_balances ();

        break;

    case PROPERTY_WANTED_BALANCE:
        break;
    }

    /* update main page */
    mise_a_jour_liste_comptes_accueil = 1;
    mise_a_jour_liste_echeances_manuelles_accueil = 1;
    mise_a_jour_soldes_minimaux = 1;
    mise_a_jour_fin_comptes_passifs = 1;

    return FALSE;
}



/**
 * called when change the currency of the account,
 * propose to change the currency of all the transactions, or let them
 *
 * \param combobox the combobox of currencies
 * \param null a pointer not used
 *
 * \return FALSE
 * */
gboolean gsb_account_property_change_currency ( GtkWidget *combobox,
                        gpointer null )
{
    gint account_number;
    gint new_currency_number;
    gint account_currency_number;
    gint result;
    GSList *list_tmp;
	gchar* tmpstr;

    account_number = gsb_gui_navigation_get_current_account ();

    account_currency_number = gsb_data_account_get_currency (account_number);

    new_currency_number = gsb_currency_get_currency_from_combobox (detail_devise_compte);

    if ( account_currency_number == new_currency_number )
        return FALSE;

    /* set the new currency, must set here and no in the autofunc directly  */
    gsb_data_account_set_currency ( account_number,
				    new_currency_number );

    /* ask for the currency of the transactions */
    tmpstr = g_strdup_printf ( _("You are changing the currency of the account, do you want to change the currency of the transactions too ?\n(yes will change all the transactions currency from %s to %s, all the transactions with another currency will stay the same).\n\nArchived and reconcilied transactions will be left unmodified."),
				      gsb_data_currency_get_name (account_currency_number),
				      gsb_data_currency_get_name (new_currency_number));
    result = question_yes_no_hint ( _("Change the transactions currency"),
				    tmpstr,
				    GTK_RESPONSE_NO );
    g_free ( tmpstr );

    if (result)
    {
	/* we have to change the currency of the transactions,
	 * we never want to change the archives, neither the marked R transactions
	 * so change only the non archived and non marked R transactions */
	list_tmp = gsb_data_transaction_get_transactions_list ();

	while ( list_tmp )
	{
	    gint transaction_number;
	    transaction_number = gsb_data_transaction_get_transaction_number (list_tmp -> data);

	    if ( gsb_data_transaction_get_account_number (transaction_number) == account_number
		 &&
		 gsb_data_transaction_get_currency_number (transaction_number) == account_currency_number
		 &&
		 gsb_data_transaction_get_marked_transaction (transaction_number) != OPERATION_RAPPROCHEE)
		gsb_data_transaction_set_currency_number ( transaction_number,
							   new_currency_number );
	    list_tmp = list_tmp -> next;
	}
    }

    transaction_list_update_element (ELEMENT_CREDIT);
    transaction_list_update_element (ELEMENT_DEBIT);
    transaction_list_update_element (ELEMENT_AMOUNT);

    /* in each cases, we had to update the account balance */
    transaction_list_set_balances ();

    /* update the headings balance */
    gsb_data_account_colorize_current_balance ( account_number );

    return FALSE;
}


/**
 * positionne le code bank
 *
 * \param bank_number
 *
 * */
void gsb_account_property_set_label_code_banque ( gint bank_number )
{
    if ( gsb_data_bank_get_code (bank_number) )
    {
    gtk_label_set_text ( GTK_LABEL (label_code_banque), gsb_data_bank_get_code (
                        bank_number) );
    gtk_widget_show ( GTK_WIDGET (label_code_banque) );
    }
    else
    {
    gtk_label_set_text ( GTK_LABEL(label_code_banque ), "" );
    gtk_widget_hide ( GTK_WIDGET(label_code_banque) );
    }
}

/**
 * positionne le code BIC
 *
 * \param bank_number
 *
 **/
void gsb_account_property_set_label_code_bic ( gint bank_number )
{
    if ( gsb_data_bank_get_bic ( bank_number ) )
    {
        gtk_label_set_text ( GTK_LABEL ( label_code_bic ), gsb_data_bank_get_bic (
                        bank_number) );
        gtk_widget_show ( GTK_WIDGET ( label_code_bic ) );
    }
    else
    {
        gtk_label_set_text ( GTK_LABEL ( label_code_bic ), "" );
        gtk_widget_hide ( GTK_WIDGET ( label_code_bic ) );
    }
}


/**
 *
 *
 *
 *
 **/
void gsb_account_property_iban_insert_text ( GtkEditable *entry,
                        gchar *text,
                        gint length,
                        gint *position,
                        GtkWidget *combobox )
{
    static struct iso_13616_iban *s_iban = iso_13616_ibans;
    gchar *iban;
    gint nbre_char;

    iban = g_utf8_strup (text, length);

    /* on bloque l'appel de la fonction */
    g_signal_handlers_block_by_func ( G_OBJECT (entry),
                        G_CALLBACK (gsb_account_property_iban_insert_text),
                        bank_list_combobox );

    /* on met en majuscule l'entrée */
    gtk_editable_insert_text (entry, iban, length, position);
    g_free ( iban );

    /* on fait les traitements complémentaires */
    iban = g_utf8_strup ( gtk_editable_get_chars ( entry, 0, -1 ), -1 );

    /* on autorise ou pas la saisie des données banquaires */
    if ( g_utf8_strlen (iban, -1) == 0 )
        gsb_account_property_iban_switch_bank_data ( TRUE );
    else if ( GTK_WIDGET_IS_SENSITIVE ( bank_list_combobox ) )
        gsb_account_property_iban_switch_bank_data ( FALSE );

    /* on contrôle l'existence d'un modèle pour le numéro IBAN */
    if ( g_utf8_strlen (iban, -1) >= 2 )
    {
        if ( g_utf8_collate ( s_iban -> locale, g_strndup (iban, 2)) != 0)
        {
            s_iban = gsb_account_property_iban_get_struc ( iban );

            if ( s_iban -> nbre_char % 4 == 0 )
                nbre_char = s_iban -> nbre_char + (s_iban -> nbre_char / 4) - 1;
            else
                nbre_char = s_iban -> nbre_char + (s_iban -> nbre_char / 4);
            gtk_entry_set_max_length ( GTK_ENTRY (entry), nbre_char );
        }
    }

    g_free ( iban );
    g_signal_handlers_unblock_by_func ( G_OBJECT (entry),
                        G_CALLBACK (gsb_account_property_iban_insert_text),
                        bank_list_combobox );
    g_signal_stop_emission_by_name ( entry, "insert_text" );
}


/**
 *
 *
 *
 *
 * */
void gsb_account_property_iban_delete_text ( GtkEditable *entry,
                        gint start_pos,
                        gint end_pos,
                        GtkWidget *combobox )
{
    /* on autorise ou pas la saisie des données banquaires */
    if ( start_pos == 0 )
        gsb_account_property_iban_switch_bank_data ( TRUE );
    else if ( GTK_WIDGET_IS_SENSITIVE ( bank_list_combobox ) )
        gsb_account_property_iban_switch_bank_data ( FALSE );
}


/**
 * retourne la structure du compte IBAN pour le pays concerné
 *
 * \param le code du pays concerné
 *
 * \return une structure modèle (XX si pays non défini)
 * */
struct iso_13616_iban *gsb_account_property_iban_get_struc ( gchar *pays )
{
    struct iso_13616_iban *s_iban = iso_13616_ibans;

    while (s_iban -> iban )
    {
        if ( g_strstr_len (s_iban -> locale, 2, g_strndup ( pays, 2)) )
            break;
        s_iban ++;
    }
    if ( s_iban -> iban == NULL )
        s_iban = iso_13616_ibans;

    return s_iban;
}


/**
 *
 *
 *
 *
 * */
gboolean gsb_account_property_iban_key_press_event ( GtkWidget *entry,
                        GdkEventKey *ev,
                        gpointer data )
{
    gchar *iban;
    gint current_account;

    switch ( ev -> keyval )
    {
	case GDK_Escape :

        return TRUE;
	    break;

	case GDK_KP_Enter :
	case GDK_Return :
        iban = gtk_editable_get_chars ( GTK_EDITABLE (entry), 0, -1 );
        if ( gsb_account_property_iban_control_iban (iban) == 0 )
        {
            gint position = 0;

            gchar *tmpstr = g_strdup_printf (
                        _("Your IBAN number is not correct. Please check your entry.") );
            dialogue_warning ( tmpstr );
            g_free ( tmpstr );
            gtk_editable_delete_text ( GTK_EDITABLE (detail_IBAN), 0, -1 );
            gtk_editable_insert_text ( GTK_EDITABLE (detail_IBAN),
                            iban, -1, &position );
        }
        else
        {
            current_account = gsb_gui_navigation_get_current_account ();
            gsb_data_account_set_bank_account_iban ( current_account, iban );
            gsb_account_property_iban_set_bank_from_iban ( iban );
            if ( etat.modification_fichier == 0 )
                modification_fichier ( TRUE );
        }

        if ( iban && strlen (iban) > 0 )
            g_free ( iban );
		return TRUE;
	    break;

    }
    return FALSE;
}


gboolean gsb_account_property_iban_focus_in_event ( GtkWidget *entry,
                        GdkEventFocus *ev,
                        gpointer data )
{
    gint current_account;

    /* on sauvegarde le numéro de compte initiateur pour éviter de conserver
     * les données de l'ancien compte lorsque l'on change de compte */

    current_account = gsb_gui_navigation_get_current_account ();
    g_object_set_data ( G_OBJECT (entry), "old_account",
                        GINT_TO_POINTER (current_account) );

    return FALSE;
}

/**
 * If the IBAN is correct it is saved, otherwise we return the
 * previous version.
 *
 * */
gboolean gsb_account_property_iban_focus_out_event ( GtkWidget *entry,
                        GdkEventFocus *ev,
                        gpointer data )
{
    gchar *iban;
    gint current_account;
    gint old_account;

    old_account = GPOINTER_TO_INT ( g_object_get_data (G_OBJECT (entry),
                        "old_account") );
    current_account = gsb_gui_navigation_get_current_account ();
    if ( old_account != current_account )
        return FALSE;

    iban = gtk_editable_get_chars ( GTK_EDITABLE (entry), 0, -1 );

    if ( gsb_account_property_iban_control_iban (iban) == 0 )
        gsb_account_property_iban_set_iban (
                    gsb_data_account_get_bank_account_iban (current_account) );
    else
    {
        gsb_data_account_set_bank_account_iban ( current_account, iban );
        gsb_account_property_iban_set_bank_from_iban ( iban );
        if ( etat.modification_fichier == 0 )
            modification_fichier ( TRUE );
    }

    if ( iban && strlen (iban) > 0 )
        g_free ( iban );

    return FALSE;
}


/**
 * Affiche les données bancaires à partir du numero IBAN. Si le N° IBAN n'existe pas
 * on affiche les données bancaires classiques
 *
 * \param le numéro IBAN
 *
 * \return TRUE si OK FALSE si affichage des donnés bancaires classiques
 * */
gboolean gsb_account_property_iban_set_bank_from_iban ( gchar *iban )
{
    struct iso_13616_iban *s_iban;
    gchar *model;
    gchar *tmpstr;
    gchar *ptr_1;
    gchar *ptr_2;
    gchar *code;
    gint pos_char_1;
    gint pos_char_2;
    gint bank_number;
    gint current_account;
    gunichar c;
    gboolean set_label = FALSE;

    current_account = gsb_gui_navigation_get_current_account ();
    bank_number = gsb_data_account_get_bank (current_account);

    /* set bank à revoir avec gestion des iban */
    g_signal_handlers_block_by_func ( G_OBJECT ( bank_list_combobox ),
                        G_CALLBACK ( gsb_account_property_changed_bank_label ),
                        NULL );
    gsb_bank_list_set_bank ( bank_list_combobox,
                        bank_number,
                        current_account );
    g_signal_handlers_unblock_by_func ( G_OBJECT ( bank_list_combobox ),
                        G_CALLBACK ( gsb_account_property_changed_bank_label ),
                        NULL );

    if ( iban == NULL || strlen (iban) == 0 )
    {
        gsb_account_property_iban_display_bank_data ( current_account,
                        bank_number );
        return FALSE;
    }

    s_iban = gsb_account_property_iban_get_struc ( g_strndup (iban, 2) );

    /* on affiche les données banquaires pour un IBAN sans modèle */
    if ( g_strcmp0 (s_iban -> locale, "XX") == 0 )
    {
        gsb_account_property_iban_display_bank_data ( current_account,
                        bank_number );
        gsb_account_property_iban_switch_bank_data ( TRUE );
        return FALSE;
    }

    model = my_strdelimit ( s_iban -> iban, " -", "" );
    tmpstr = my_strdelimit ( iban, " -", "" );
    if ( g_utf8_strlen (model, -1) != g_utf8_strlen (tmpstr, -1) )
        return FALSE;

    code = g_malloc0 ( 36 * sizeof (gunichar) );

    /* set label_code_banque */
    c = 'A';
    ptr_1 = g_utf8_strchr ( model, -1, c );
    if ( ptr_1 )
    {
        pos_char_1 = g_utf8_pointer_to_offset ( model, ptr_1 );
        ptr_2 = g_utf8_strrchr ( model, -1, c );
        pos_char_2 = g_utf8_pointer_to_offset ( model, ptr_2 );
        if ( (pos_char_2 - pos_char_1) > 0 )
        {
            code = g_utf8_strncpy ( code, ptr_1, (pos_char_2 - pos_char_1) + 1 );
            gtk_label_set_text ( GTK_LABEL (label_code_banque), code );
            gtk_widget_show ( label_code_banque );
            set_label = TRUE;
        }
    }
    c = 'B';
    ptr_1 = g_utf8_strchr ( model, -1, c );
    if ( set_label == FALSE && ptr_1 )
    {
        pos_char_1 = g_utf8_pointer_to_offset ( model, ptr_1 );
        ptr_1 = g_utf8_offset_to_pointer ( tmpstr, pos_char_1 );

        ptr_2 = g_utf8_strrchr ( model, -1, c );
        pos_char_2 = g_utf8_pointer_to_offset ( model, ptr_2 );

        code = g_utf8_strncpy ( code, ptr_1, (pos_char_2 - pos_char_1) + 1 );
        gtk_label_set_text ( GTK_LABEL (label_code_banque), code );
        gtk_widget_show ( label_code_banque );
    }

    /* set bank_branch_code */
    if ( c == 'A' )
        c = 'B';
    else
        c = 'G';
    ptr_1 = g_utf8_strchr ( model, -1, c );
    if ( ptr_1 == NULL )
        c = 'S';
    if ( ptr_1 )
    {
        pos_char_1 = g_utf8_pointer_to_offset ( model, ptr_1 );
        ptr_1 = g_utf8_offset_to_pointer ( tmpstr, pos_char_1 );

        ptr_2 = g_utf8_strrchr ( model, -1, c );
        pos_char_2 = g_utf8_pointer_to_offset ( model, ptr_2 );

        code = g_utf8_strncpy ( code, ptr_1, (pos_char_2 - pos_char_1) + 1 );
    }
    else
        code = g_strdup ( "" );
    gtk_label_set_text ( GTK_LABEL (label_guichet), code );
    gtk_widget_show ( label_guichet );

    /* set bank_account_number */
    c = 'C';
    ptr_1 = g_utf8_strchr ( model, -1, c );
    if ( ptr_1 )
    {
        pos_char_1 = g_utf8_pointer_to_offset ( model, ptr_1 );
        ptr_1 = g_utf8_offset_to_pointer ( tmpstr, pos_char_1 );

        ptr_2 = g_utf8_strrchr ( model, -1, c );
        pos_char_2 = g_utf8_pointer_to_offset ( model, ptr_2 );

        code = g_utf8_strncpy ( code, ptr_1, (pos_char_2 - pos_char_1) + 1 );
        gtk_label_set_text ( GTK_LABEL (label_no_compte), code );
        gtk_widget_show ( label_no_compte );
    }

    /* set bank_account_key */
    c = 'K';
    ptr_1 = g_utf8_strchr ( model, -1, c );
    if ( ptr_1 )
    {
        pos_char_1 = g_utf8_pointer_to_offset ( model, ptr_1 );
        ptr_1 = g_utf8_offset_to_pointer ( tmpstr, pos_char_1 );

        ptr_2 = g_utf8_strrchr ( model, -1, c );
        pos_char_2 = g_utf8_pointer_to_offset ( model, ptr_2 );

        code = g_utf8_strncpy ( code, ptr_1, (pos_char_2 - pos_char_1) + 1 );
    }
    else
        code = g_strdup ( "" );
    gtk_label_set_text ( GTK_LABEL (label_cle_compte), code );
    gtk_widget_show ( label_cle_compte );

    g_free ( model );
    g_free ( tmpstr );
    g_free ( code );

    return TRUE;
}


/**
 *
 *
 *
 *
 * */
void gsb_account_property_iban_set_iban ( const gchar *iban )
{
    gint position = 0;

    gtk_editable_delete_text ( GTK_EDITABLE (detail_IBAN), 0, -1 );
    if ( iban && strlen (iban) > 0 )
        gtk_editable_insert_text ( GTK_EDITABLE (detail_IBAN),
                            iban, -1, &position );
}

/**
 * Bascule l'affichage des caractéristiques bancaires soit vers les données
 *  classiques soit vers les données crées à partir de l'IBAN
 *
 \param sensitive Si TRUE affiche les données classiques si FALSE les données IBAN
 *
 * */
void gsb_account_property_iban_switch_bank_data ( gboolean sensitive )
{
    if ( sensitive )
    {
        gtk_widget_set_sensitive ( GTK_WIDGET (bank_list_combobox), sensitive );
        gsb_account_property_iban_clear_label_data ( );
        gtk_widget_show ( GTK_WIDGET (detail_guichet) );
        gtk_widget_hide ( GTK_WIDGET (label_guichet) );
        gtk_widget_show ( GTK_WIDGET (detail_no_compte) );
        gtk_widget_hide ( GTK_WIDGET (label_no_compte) );
        gtk_widget_show ( GTK_WIDGET (detail_cle_compte) );
        gtk_widget_hide ( GTK_WIDGET (label_cle_compte) );
    }
    else
    {
        gtk_widget_set_sensitive ( GTK_WIDGET (bank_list_combobox), sensitive );
        gtk_widget_hide ( GTK_WIDGET (detail_guichet) );
        gtk_widget_show ( GTK_WIDGET (label_guichet) );
        gtk_widget_hide ( GTK_WIDGET (detail_no_compte) );
        gtk_widget_show ( GTK_WIDGET (label_no_compte) );
        gtk_widget_hide ( GTK_WIDGET (detail_cle_compte) );
        gtk_widget_show ( GTK_WIDGET (label_cle_compte) );
    }
}


/**
 * Affiche les données classiques du compte, données que l'on peut modifier
 * en ligne.
 *
 * \param current_account
 * \param bank_number
 *
 * */
void gsb_account_property_iban_display_bank_data ( gint current_account,
                        gint bank_number )
{
    gsb_account_property_set_label_code_banque ( bank_number );
    gsb_account_property_set_label_code_bic ( bank_number );
    gsb_autofunc_entry_set_value (detail_guichet,
                        gsb_data_account_get_bank_branch_code (
                        current_account), current_account);
    gsb_autofunc_entry_set_value (detail_no_compte,
                        gsb_data_account_get_bank_account_number (
                        current_account), current_account );
    gsb_autofunc_entry_set_value (detail_cle_compte,
                        gsb_data_account_get_bank_account_key (
                        current_account), current_account );
}


/**
 *
 *
 *
 *
 * */
void gsb_account_property_iban_clear_label_data ( void )
{
    gtk_label_set_text ( GTK_LABEL (label_guichet), "" );
    gtk_label_set_text ( GTK_LABEL (label_no_compte), "" );
    gtk_label_set_text ( GTK_LABEL (label_cle_compte), "" );
}


/**
 * Contrôle la validité du numéro IBAN (non opérationnel pour la partie IBAN)
 *
 * \param le numéro IBAN
 *
 * \return 1 si OK 0 si NON OK -1 si longueur IBAN = 0
 * */
gint gsb_account_property_iban_control_iban ( gchar *iban )
{
    struct iso_13616_iban *s_iban;
    gchar *model;
    gchar *tmp_str = NULL;
    gchar *substr;
    gchar *ptr = NULL;
    gchar *buffer = NULL;
    GString *gstring;
    gint i = 0;
    gulong lnum;
    gint reste = 0;
    gint code_controle;
    gint result = 0;

    if ( iban == NULL )
        return 0;
    else if ( strlen ( iban ) == 0 )
        return -1;

    s_iban = gsb_account_property_iban_get_struc ( g_strndup (iban, 2) );

    model = my_strdelimit ( s_iban -> iban, " -", "" );
    tmp_str = my_strdelimit ( iban, " -", "" );

    /* on ne contrôle pas la longueur d'un IBAN sans modèle */
    if ( g_strcmp0 ( s_iban -> locale, "XX" ) != 0 &&
                    g_utf8_strlen ( model, -1 ) != g_utf8_strlen ( tmp_str, -1 ) )
    {
        g_free ( model );
        g_free ( tmp_str );
        return 0;
    }

    /* mise en forme de l'IBAN avant contrôle
     * on place les 4 premiers caractères en fin de chaine et on remplace
     * les 2 chiffres du code de contrôle par des 0.
     * Ensuite on remplace les lettres par des chiffres selon un
     * codage particulier A = 10 B = 11 etc...
     */

    if ( strlen (tmp_str) > 4 )
        tmp_str = g_strconcat ( tmp_str + 4, g_strndup (tmp_str, 2), "00", NULL );
    else
        return 0;

    ptr = tmp_str;
    while ( ptr[i]  )
	{
        if ( g_ascii_isdigit ( ptr[i] ) )
        {
            if ( buffer == NULL )
                buffer = g_strdup_printf ( "%c", ptr[i] );
            else
                buffer = g_strconcat ( buffer,
                        g_strdup_printf ("%c", ptr[i]), NULL );
        }
        else
        {
            if ( buffer == NULL )
                buffer = g_strdup_printf ( "%d", ptr[i] - 55 );
            else
                buffer = g_strconcat ( buffer,
                        g_strdup_printf ("%d", ptr[i] - 55), NULL );
        }
        i++;
    }

    g_free ( tmp_str );

    /* vérification du calcul de l'IBAN
     *
     * methode : on procède par étapes
     * on prend les 9 premiers chiffres et on les divise par 97
     * on ajoute au reste les 7 ou 8 chiffres suivants que l'on divise par 97
     * on procède ainsi jusqu'a la fin de la chaine. Ensuite on soustrait le
     * dernier reste à 98 ce qui donne le code de contrôle
     */
    gstring = g_string_new ( buffer );

    while ( gstring -> len > 0 )
    {
        substr = g_strndup ( gstring -> str, 9 );
        lnum = ( gulong ) utils_str_safe_strtod ( substr, NULL );
        reste = lnum % 97;

        g_free ( substr );

        if ( gstring -> len >= 9 )
        {
            gstring = g_string_erase ( gstring, 0, 9 );
            gstring = g_string_prepend ( gstring, g_strdup_printf ( "%d", reste ) );
        }
        else
            break;
    }

    g_string_free ( gstring, TRUE );

    reste = 98 - reste;

    tmp_str = g_strndup ( iban + 2, 2 );
    code_controle =  utils_str_atoi ( tmp_str );
    
    if ( code_controle - reste == 0 )
        result = 1;
    else
        result = 0;

    g_free ( tmp_str );

    return result;
}


/**
 * required to avoid warnings change file.
 *
 *
 *
 * */
void gsb_account_property_clear_config ( void )
{
    edit_bank_button = NULL;
    detail_nom_compte = NULL;
    detail_type_compte = NULL;
    detail_titulaire_compte = NULL;
    detail_adresse_titulaire = NULL;
    bank_list_combobox = NULL;
    label_code_banque = NULL;
    detail_guichet = NULL;
    detail_no_compte = NULL;
    detail_cle_compte = NULL;
    detail_devise_compte = NULL;
    detail_compte_cloture = NULL;
    detail_solde_init = NULL;
    detail_solde_mini_autorise = NULL;
    detail_solde_mini_voulu = NULL;
    detail_commentaire = NULL;
    button_holder_address = NULL;
    bouton_icon = NULL;
    detail_IBAN = NULL;
    label_code_bic = NULL;
    label_guichet = NULL;
    label_no_compte = NULL;
    label_cle_compte = NULL;
}

gboolean gsb_account_property_focus_out ( GtkWidget *widget,
                        GdkEventFocus *event,
                        gint *p_origin )
{
    gint origin = GPOINTER_TO_INT (p_origin);
    gint account_number;

    account_number = gsb_gui_navigation_get_current_account ();
    if ( account_number == -1)
        return FALSE;

    switch (origin)
    {
    case PROPERTY_NAME:
        /* update the scheduler list */
        gsb_scheduler_list_fill_list (gsb_scheduler_list_get_tree_view ());

        /*update the the view menu */
        gsb_navigation_update_account_label (account_number);
        gsb_menu_update_accounts_in_menus ();

        /* update the name of accounts in form */
        gsb_account_update_combo_list ( gsb_form_scheduler_get_element_widget (
                        SCHEDULED_FORM_ACCOUNT), FALSE );

        /* Replace trees contents. */
        remplit_arbre_categ ();
        remplit_arbre_imputation ();
        payee_fill_tree ();
        break;
    case PROPERTY_HOLDER_NAME:
        gsb_main_set_grisbi_title ( account_number );
        break;
    }

    /* update main page */
    mise_a_jour_liste_comptes_accueil = 1;
    mise_a_jour_liste_echeances_manuelles_accueil = 1;
    mise_a_jour_soldes_minimaux = 1;
    mise_a_jour_fin_comptes_passifs = 1;

    return FALSE;
}
/* Local Variables: */
/* c-basic-offset: 4 */
/* End: */
