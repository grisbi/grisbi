/* *******************************************************************************/
/*                                 GRISBI                                        */
/*              Programme de gestion financière personnelle                      */
/*                              license : GPLv2                                  */
/*                                                                               */
/*     Copyright (C)    2000-2008 Cédric Auger (cedric@grisbi.org)               */
/*                      2003-2008 Benjamin Drieu (bdrieu@april.org)              */
/*          2008-2018 Pierre Biava (grisbi@pierre.biava.name)                    */
/*          https://www.grisbi.org/                                              */
/*                                                                               */
/*     This program is free software; you can redistribute it and/or modify      */
/*     it under the terms of the GNU General Public License as published by      */
/*     the Free Software Foundation; either version 2 of the License, or         */
/*     (at your option) any later version.                                       */
/*                                                                               */
/*     This program is distributed in the hope that it will be useful,           */
/*     but WITHOUT ANY WARRANTY; without even the implied warranty of            */
/*     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the             */
/*     GNU General Public License for more details.                              */
/*                                                                               */
/*     You should have received a copy of the GNU General Public License         */
/*     along with this program; if not, write to the Free Software               */
/*     Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA */
/*                                                                               */
/* *******************************************************************************/

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <errno.h>
#include <glib/gstdio.h>
#include <glib/gi18n.h>

/*START_INCLUDE*/
#include "widget_bank_details.h"
#include "gsb_autofunc.h"
#include "gsb_bank.h"
#include "gsb_data_bank.h"
#include "structures.h"
#include "utils.h"
#include "utils_editables.h"
#include "utils_prefs.h"
#include "widget_account_property.h"
#include "erreur.h"

/*END_INCLUDE*/

/*START_EXTERN*/
/*END_EXTERN*/


typedef struct _WidgetBankDetailsPrivate	WidgetBankDetailsPrivate;

struct _WidgetBankDetailsPrivate
{
	GtkWidget *			vbox_bank_details;
	GtkWidget *			grid_bank_details;

	GtkWidget *			button_bank_remove;		/* bouton miroir du bouton remove de l'onglet bank */
	GtkWidget *			entry_bank_name;
	GtkWidget *			entry_bank_code;
	GtkWidget *			entry_bank_BIC;
	GtkWidget *			entry_bank_tel;
	GtkWidget *			entry_bank_mail;
	GtkWidget *			entry_bank_web;
	GtkWidget *			entry_bank_contact_name;
	GtkWidget *			entry_bank_contact_tel;
	GtkWidget *			entry_bank_contact_mail;
	GtkWidget *			entry_bank_contact_fax;
	GtkWidget *			textview_bank_adr;
	GtkWidget *			textview_bank_notes;
};

G_DEFINE_TYPE_WITH_PRIVATE (WidgetBankDetails, widget_bank_details, GTK_TYPE_BOX)

/******************************************************************************/
/* Private functions                                                          */
/******************************************************************************/
/**
 * Update BIC code in account_property view
 * when it's change in the edit_bank form.
 *
 * \param entry
 * \param null
 *
 * \return
 **/
static void widget_bank_details_bic_code_changed (GtkEntry *entry,
												  gpointer null)
{
    gint bank_number;

    bank_number = GPOINTER_TO_INT (g_object_get_data (G_OBJECT (entry), "number_for_func"));
    widget_account_property_update_bank_data (bank_number);
}

/**
 * Update bank code in account_property view
 * when it's change in the edit_bank form.
 *
 * \param entry
 * \param null
 *
 * \return
 **/
static void widget_bank_details_code_changed (GtkEntry *entry,
											  gpointer null)
{
    gint bank_number;

    bank_number = GPOINTER_TO_INT (g_object_get_data (G_OBJECT (entry), "number_for_func"));
    widget_account_property_update_bank_data (bank_number);
}

/**
 * Création du widget des détails d'une banque
 *
 * \param bank_details
 * \param page_bank
 * \param combobox		présent si appel depuis account_property.c
 *
 * \return
 **/
static void widget_bank_details_setup_widget (WidgetBankDetails *w_bank_details,
											  PrefsPageBank *page_bank,
											  GtkWidget *combobox)
{
    GtkSizeGroup *size_group;
	WidgetBankDetailsPrivate *priv;

	devel_debug (NULL);
	priv = widget_bank_details_get_instance_private (w_bank_details);
    size_group = gtk_size_group_new (GTK_SIZE_GROUP_HORIZONTAL);

	if (page_bank)
		priv->button_bank_remove = prefs_page_bank_get_button_remove (page_bank);
	else
		priv->button_bank_remove = NULL;

	/* set Generalities */
    if (combobox)
	{
        priv->entry_bank_name = gsb_autofunc_entry_new (NULL, NULL, NULL, NULL, 0);
        priv->entry_bank_code = gsb_autofunc_entry_new (NULL, NULL, NULL, NULL, 0);
        priv->entry_bank_BIC = gsb_autofunc_entry_new (NULL, NULL, NULL, NULL, 0);
        priv->textview_bank_adr = gsb_autofunc_textview_new (NULL, NULL, NULL, NULL, 0);
        priv->entry_bank_tel = gsb_autofunc_entry_new (NULL, NULL, NULL, NULL, 0);
        priv->entry_bank_mail = gsb_autofunc_entry_new (NULL, NULL, NULL, NULL, 0);
        priv->entry_bank_web = gsb_autofunc_entry_new (NULL, NULL, NULL, NULL, 0);
	}
    else
	{
        priv->entry_bank_name = gsb_autofunc_entry_new (NULL,
														G_CALLBACK (prefs_page_bank_update_selected_line),
														page_bank,
														G_CALLBACK (gsb_data_bank_set_name),
														0);
        priv->entry_bank_code = gsb_autofunc_entry_new (NULL,
														G_CALLBACK (widget_bank_details_code_changed),
														NULL,
														G_CALLBACK (gsb_data_bank_set_code),
														0);
        priv->entry_bank_BIC = gsb_autofunc_entry_new (NULL,
													   G_CALLBACK (widget_bank_details_bic_code_changed),
													   NULL,
                       							 	   G_CALLBACK (gsb_data_bank_set_bic),
													   0);
        priv->textview_bank_adr = gsb_autofunc_textview_new (NULL,
															 NULL,
															 NULL,
															 G_CALLBACK (gsb_data_bank_set_bank_address),
															 0);
		priv->entry_bank_tel = gsb_autofunc_entry_new (NULL,
													   NULL,
                        							   NULL,
                        							   G_CALLBACK (gsb_data_bank_set_bank_tel),
                        							   0);
        priv->entry_bank_mail = gsb_autofunc_entry_new (NULL,
														NULL,
                        								NULL,
                        								G_CALLBACK (gsb_data_bank_set_bank_mail),
                        								0);
        priv->entry_bank_web = gsb_autofunc_entry_new (NULL,
													   NULL,
                        							   NULL,
                        							   G_CALLBACK (gsb_data_bank_set_bank_web),
                        							   0);
	}

	/* identification de type de nom pour la MAJ de la liste des détails */
	g_object_set_data (G_OBJECT (priv->entry_bank_name), "name-type", GINT_TO_POINTER (0));

	/* attach widgets */
    gtk_size_group_add_widget (size_group, priv->entry_bank_name);
    gtk_grid_attach (GTK_GRID (priv->grid_bank_details), priv->entry_bank_name, 2, 1, 2, 1);

    gtk_size_group_add_widget (size_group, priv->entry_bank_code);
    gtk_grid_attach (GTK_GRID (priv->grid_bank_details), priv->entry_bank_code, 2, 2, 2, 1);

    gtk_size_group_add_widget (size_group, priv->entry_bank_BIC);
    gtk_grid_attach (GTK_GRID (priv->grid_bank_details), priv->entry_bank_BIC, 2, 3, 2, 1);

    gtk_size_group_add_widget (size_group, priv->textview_bank_adr);
    gtk_grid_attach (GTK_GRID (priv->grid_bank_details), priv->textview_bank_adr, 2, 4, 2, 1);

    gtk_size_group_add_widget (size_group, priv->entry_bank_tel);
    gtk_grid_attach (GTK_GRID (priv->grid_bank_details), priv->entry_bank_tel, 2, 5, 2, 1);

    gtk_size_group_add_widget (size_group, priv->entry_bank_mail);
    gtk_grid_attach (GTK_GRID (priv->grid_bank_details), priv->entry_bank_mail, 2, 6, 2, 1);

    gtk_size_group_add_widget (size_group, priv->entry_bank_web);
    gtk_grid_attach (GTK_GRID (priv->grid_bank_details), priv->entry_bank_web, 2, 7, 2, 1);

	/* set width and heigh */
	gtk_widget_set_hexpand (priv->textview_bank_adr, TRUE);
	gtk_widget_set_size_request (priv->textview_bank_adr, -1, 80);

	/* set Contact */
    if (combobox)
	{
        priv->entry_bank_contact_name = gsb_autofunc_entry_new (NULL, NULL, NULL, NULL, 0);
        priv->entry_bank_contact_tel = gsb_autofunc_entry_new (NULL, NULL, NULL, NULL, 0);
        priv->entry_bank_contact_mail = gsb_autofunc_entry_new (NULL, NULL, NULL, NULL, 0);
        priv->entry_bank_contact_fax = gsb_autofunc_entry_new (NULL, NULL, NULL, NULL, 0);
	}
    else
	{
        priv->entry_bank_contact_name = gsb_autofunc_entry_new (NULL,
																G_CALLBACK (prefs_page_bank_update_selected_line),
                        										page_bank,
                        										G_CALLBACK (gsb_data_bank_set_correspondent_name),
                        										0);
        priv->entry_bank_contact_tel = gsb_autofunc_entry_new (NULL,
															   NULL,
                        									   NULL,
                        									   G_CALLBACK (gsb_data_bank_set_correspondent_tel),
                        									   0);
        priv->entry_bank_contact_mail = gsb_autofunc_entry_new (NULL,
																NULL,
                        										NULL,
                        										G_CALLBACK (gsb_data_bank_set_correspondent_mail),
                        										0);
        priv->entry_bank_contact_fax = gsb_autofunc_entry_new (NULL,
															   NULL,
                        									   NULL,
                        									   G_CALLBACK (gsb_data_bank_set_correspondent_fax),
                        									   0);
	}

	/* identification de type de nom pour la MAJ de la liste des détails */
	g_object_set_data (G_OBJECT (priv->entry_bank_contact_name), "name-type", GINT_TO_POINTER (1));

	/* attach widgets */
	gtk_size_group_add_widget (size_group, priv->entry_bank_contact_name);
    gtk_grid_attach (GTK_GRID (priv->grid_bank_details), priv->entry_bank_contact_name, 2, 9, 2, 1);

    gtk_size_group_add_widget (size_group, priv->entry_bank_contact_tel);
    gtk_grid_attach (GTK_GRID (priv->grid_bank_details), priv->entry_bank_contact_tel, 2, 10, 2, 1);

    gtk_size_group_add_widget (size_group, priv->entry_bank_contact_mail);
    gtk_grid_attach (GTK_GRID (priv->grid_bank_details), priv->entry_bank_contact_mail, 2, 11, 2, 1);

    gtk_size_group_add_widget (size_group, priv->entry_bank_contact_fax);
    gtk_grid_attach (GTK_GRID (priv->grid_bank_details), priv->entry_bank_contact_fax, 2, 12, 2, 1);

	/* set Notes */
    if (combobox)
    {
        priv->textview_bank_notes = gsb_autofunc_textview_new (NULL, NULL, NULL, NULL, 0);
    }
    else
    {
        priv->textview_bank_notes = gsb_autofunc_textview_new (NULL,
                        NULL,
                        NULL,
                        G_CALLBACK (gsb_data_bank_set_bank_note),
                        0);
    }

	/* attach widget */
	gtk_size_group_add_widget (size_group, priv->textview_bank_notes);
    gtk_grid_attach (GTK_GRID (priv->grid_bank_details), priv->textview_bank_notes, 1, 14, 3, 1);

	/* set width and heigh */
	gtk_widget_set_size_request (priv->textview_bank_notes, -1, SW_MIN_HEIGHT);
}

/******************************************************************************/
/* Fonctions propres à l'initialisation des fenêtres                          */
/******************************************************************************/
/**
 *
 *
 * \param
 *
 * \return
 **/
static void widget_bank_details_init (WidgetBankDetails *w_bank_details)
{
	gtk_widget_init_template (GTK_WIDGET (w_bank_details));
}

/**
 *
 *
 * \param
 *
 * \return
 **/
static void widget_bank_details_dispose (GObject *object)
{
	G_OBJECT_CLASS (widget_bank_details_parent_class)->dispose (object);
}

/**
 *
 *
 * \param
 *
 * \return
 **/
static void widget_bank_details_class_init (WidgetBankDetailsClass *klass)
{
	G_OBJECT_CLASS (klass)->dispose = widget_bank_details_dispose;

	gtk_widget_class_set_template_from_resource (GTK_WIDGET_CLASS (klass),
												 "/org/gtk/grisbi/widgets/widget_bank_details.ui");

	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), WidgetBankDetails, vbox_bank_details);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), WidgetBankDetails, grid_bank_details);
}

/******************************************************************************/
/* Public functions                                                           */
/******************************************************************************/
/**
 *
 *
 * \param
 * \param combobox		présent si appel depuis account_property.c
 *
 * \return
 **/
WidgetBankDetails *widget_bank_details_new (PrefsPageBank *page_bank,
											GtkWidget *combobox)
{
	WidgetBankDetails *w_bank_details;

	w_bank_details = g_object_new (WIDGET_BANK_DETAILS_TYPE, NULL);
	widget_bank_details_setup_widget (w_bank_details, page_bank, combobox);

	return w_bank_details;
}

/**
 * create or update bank data
 * when it's change in the edit_bank form.
 *
 * \param entry
 * \param w_bank_details
 *
 * \return
 **/
void widget_bank_details_update_bank_data (gint bank_number,
										   GtkWidget *w_bank_details)
{
    GtkTextBuffer *buffer;
	WidgetBankDetailsPrivate *priv;

	priv = widget_bank_details_get_instance_private (WIDGET_BANK_DETAILS (w_bank_details));

    /* set bank_name */
    if (gtk_entry_get_text_length (GTK_ENTRY (priv->entry_bank_name)) > 0)
        gsb_data_bank_set_name (bank_number, gtk_entry_get_text (GTK_ENTRY (priv->entry_bank_name)));
    else
        gsb_data_bank_set_name (bank_number, _("New bank"));

    /* set bank_code */
    if (gtk_entry_get_text_length (GTK_ENTRY (priv->entry_bank_code)) > 0)
        gsb_data_bank_set_code (bank_number, gtk_entry_get_text (GTK_ENTRY (priv->entry_bank_code)));
    else
        gsb_data_bank_set_code (bank_number, "");

    /* set bank_BIC */
    if (gtk_entry_get_text_length (GTK_ENTRY (priv->entry_bank_BIC)) > 0)
        gsb_data_bank_set_bic (bank_number, gtk_entry_get_text (GTK_ENTRY (priv->entry_bank_BIC)));
    else
        gsb_data_bank_set_bic (bank_number, "");

    /* set bank_adr */
    buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (priv->textview_bank_adr));
    if (gtk_text_buffer_get_char_count (buffer) > 0)
        gsb_data_bank_set_bank_address (bank_number,
										gsb_editable_text_view_get_content (priv->textview_bank_adr));
    else
        gsb_data_bank_set_bank_address (bank_number, "");

    /* set bank_tel */
    if (gtk_entry_get_text_length (GTK_ENTRY (priv->entry_bank_tel)) > 0)
        gsb_data_bank_set_bank_tel (bank_number, gtk_entry_get_text (GTK_ENTRY (priv->entry_bank_tel)));
    else
        gsb_data_bank_set_bank_tel (bank_number, "");

    /* set bank_mail */
    if (gtk_entry_get_text_length (GTK_ENTRY (priv->entry_bank_mail)) > 0)
        gsb_data_bank_set_bank_mail (bank_number, gtk_entry_get_text (GTK_ENTRY (priv->entry_bank_mail)));
    else
        gsb_data_bank_set_bank_mail (bank_number, "");

    /* set bank_web */
    if (gtk_entry_get_text_length (GTK_ENTRY (priv->entry_bank_web)) > 0)
        gsb_data_bank_set_bank_web (bank_number, gtk_entry_get_text (GTK_ENTRY (priv->entry_bank_web)));
    else
        gsb_data_bank_set_bank_web (bank_number, "");

    /* set contact_name */
    if (gtk_entry_get_text_length (GTK_ENTRY (priv->entry_bank_contact_name)) > 0)
        gsb_data_bank_set_correspondent_name (bank_number, gtk_entry_get_text (GTK_ENTRY (priv->entry_bank_contact_name)));
    else
        gsb_data_bank_set_correspondent_name (bank_number, "");

    /* set contact_tel */
    if (gtk_entry_get_text_length (GTK_ENTRY (priv->entry_bank_contact_tel)) > 0)
        gsb_data_bank_set_correspondent_tel (bank_number, gtk_entry_get_text (GTK_ENTRY (priv->entry_bank_contact_tel)));
    else
        gsb_data_bank_set_correspondent_tel (bank_number, "");

    /* set contact_mail */
    if (gtk_entry_get_text_length (GTK_ENTRY (priv->entry_bank_contact_mail)) > 0)
        gsb_data_bank_set_correspondent_mail (bank_number, gtk_entry_get_text (GTK_ENTRY (priv->entry_bank_contact_mail)));
    else
        gsb_data_bank_set_correspondent_mail (bank_number, "");

    /* set contact_fax */
    if (gtk_entry_get_text_length (GTK_ENTRY (priv->entry_bank_contact_fax)) > 0)
        gsb_data_bank_set_correspondent_fax (bank_number, gtk_entry_get_text (GTK_ENTRY (priv->entry_bank_contact_fax)));
    else
        gsb_data_bank_set_correspondent_fax (bank_number, "");

    /* set bank_notes */
    buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (priv->textview_bank_adr));
    if (gtk_text_buffer_get_char_count (buffer) > 0)
        gsb_data_bank_set_bank_note (bank_number,
									 gsb_editable_text_view_get_content (priv->textview_bank_notes));
    else
        gsb_data_bank_set_bank_note (bank_number, "");
}

/**
 * fill the form containing the information of the bank
 *
 * \param bank_number the number to fill
 * \param w_bank_details
 *
 * \return
 **/
void widget_bank_details_update_form (gint bank_number,
									  GtkWidget *w_bank_details)
{
	WidgetBankDetailsPrivate *priv;

	priv = widget_bank_details_get_instance_private (WIDGET_BANK_DETAILS (w_bank_details));

	/* we can let the defaults func even for -1 because with a bank_number of -1, all the gsb_data_bank_get_...
     * will return NULL, and that's we want... */
    gsb_autofunc_entry_set_value (priv->entry_bank_name, gsb_data_bank_get_name (bank_number), bank_number);
    gsb_autofunc_entry_set_value (priv->entry_bank_code, gsb_data_bank_get_code (bank_number), bank_number);
    gsb_autofunc_entry_set_value (priv->entry_bank_BIC, gsb_data_bank_get_bic (bank_number), bank_number);
    gsb_autofunc_textview_set_value (priv->textview_bank_adr, gsb_data_bank_get_bank_address (bank_number), bank_number);

    gsb_autofunc_entry_set_value (priv->entry_bank_tel, gsb_data_bank_get_bank_tel (bank_number), bank_number);
    gsb_autofunc_entry_set_value (priv->entry_bank_mail, gsb_data_bank_get_bank_mail (bank_number), bank_number);
    gsb_autofunc_entry_set_value (priv->entry_bank_web, gsb_data_bank_get_bank_web (bank_number), bank_number);

    gsb_autofunc_entry_set_value (priv->entry_bank_contact_name, gsb_data_bank_get_correspondent_name (bank_number), bank_number);
    gsb_autofunc_entry_set_value (priv->entry_bank_contact_tel, gsb_data_bank_get_correspondent_tel (bank_number), bank_number);
    gsb_autofunc_entry_set_value (priv->entry_bank_contact_mail, gsb_data_bank_get_correspondent_mail (bank_number), bank_number);
    gsb_autofunc_entry_set_value (priv->entry_bank_contact_fax, gsb_data_bank_get_correspondent_fax (bank_number), bank_number);

    gsb_autofunc_textview_set_value (priv->textview_bank_notes, gsb_data_bank_get_bank_note (bank_number), bank_number);
}

/**
 *
 *
 * \param
 *
 * \return
 **/
void widget_bank_details_select_name_entry (gint bank_number,
											GtkWidget *w_bank_details)
{
	WidgetBankDetailsPrivate *priv;

	priv = widget_bank_details_get_instance_private (WIDGET_BANK_DETAILS (w_bank_details));

	/* sert quand on crée une banque à partir des préférences du compte */
    if (bank_number == -1)
    {
        gchar *tmp_str;

        tmp_str = g_strdup (_("New bank"));
        gtk_entry_set_text (GTK_ENTRY (priv->entry_bank_name), tmp_str);
        g_free (tmp_str);
    }
	gtk_editable_select_region (GTK_EDITABLE (priv->entry_bank_name), 0, -1);
	gtk_widget_grab_focus (priv->entry_bank_name);
}

/* Local Variables: */
/* c-basic-offset: 4 */
/* End: */

