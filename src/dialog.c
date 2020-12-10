/* ************************************************************************** */
/*                                                                            */
/*     Copyright (C)    2000-2008 Cédric Auger (cedric@grisbi.org)            */
/*          2003-2008 Benjamin Drieu (bdrieu@april.org)                       */
/*                      2008-2020 Pierre Biava (grisbi@pierre.biava.name)     */
/*          https://www.grisbi.org/                                           */
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
#include "config.h"
#endif

#include "include.h"
#include <glib/gi18n.h>

/*START_INCLUDE*/
#include "dialog.h"
#include "grisbi_app.h"
#include "parametres.h"
#include "structures.h"
#include "erreur.h"
/*END_INCLUDE*/

/*START_STATIC*/
/*END_STATIC*/

/*START_EXTERN*/
/*END_EXTERN*/

/** All delete messages */
ConditionalMsg tab_delete_msg[NBRE_MSG_DELETE] =
{
    { "delete-child-transaction", N_("Delete a child transaction."),
      NULL,
      FALSE, FALSE, },

    { "delete-transaction",  N_("Delete a transaction."),
      NULL,
      FALSE, FALSE, },

    { "delete-child-scheduled", N_("Delete a child of scheduled transaction."),
      NULL,
      FALSE, FALSE, },

    { "delete-scheduled", N_("Delete a scheduled transaction."),
      NULL,
      FALSE, FALSE, },

    { "delete-scheduled-occurrences", N_("Delete one or all occurrences of scheduled "
      "transaction."),
      NULL,
      FALSE, FALSE, },

    { "delete-rule",  N_("Delete a rule file import."),
      NULL,
      FALSE, FALSE, },

    { NULL, NULL, NULL, FALSE, FALSE },
};

/** All messages */
ConditionalMsg tab_warning_msg[NBRE_MSG_WARNINGS] =
{
    { "account-already-opened", N_("File \"%s\" is already opened"),
      N_("Either this file is already opened by another user or it wasn't closed correctly "
      "(maybe Grisbi crashed?).\nGrisbi can't save the file unless you activate the "
      "\"Force saving locked files\" option in setup."),
      FALSE, FALSE, },

    { "account-file-readable",  N_("Account file is world readable."),
      N_("Your account file should not be readable by anybody else, but it is. You should "
      "change its permissions."),
      FALSE, FALSE, },

    { "development-version", N_("You are running Grisbi version %s"),
      N_("Warning, please be aware that the version you run is a DEVELOPMENT version. "
      "Never use your original file Grisbi: you could make it unusable.\n"
      "Make a copy now."),
      FALSE, FALSE },

    { "encryption-is-irreversible", N_("Encryption is irreversible."),
      N_("Grisbi encrypts files in a very secure way that does not allow recovery without "
      "original password. It means that if you forget your password, you will lose all "
      "your data. Use with caution.\n\nI repeat: if you ever forget your password, there "
      "is no coming back, we cannot help you."),
      FALSE, FALSE, },

    { "minimum-balance-alert", N_("Account under desired balance"),
      N_("Grisbi detected that an account is under a desired balance: %s"),
      FALSE, FALSE, },

    { "reconcile-transaction", N_("Confirmation of manual reconciliation"),
      N_("You are trying to reconcile a transaction manually, "
	  "which is not a recommended action.\n"
      "Are you really sure you know what you are doing?"),
      FALSE, FALSE, },

    { "unreconcile-transaction", N_("Confirmation of manual unreconciliation"),
      N_("You are trying to unreconcile a transaction manually, "
	  "which is not a recommended action.\n"
      "Are you really sure you know what you are doing?"),
      FALSE, FALSE, },

    { "remove-backup-files", N_("Removing backups from the account file"),
      N_("You will delete %d backups from your account file that are older than %s."),
      FALSE, FALSE, },

    { NULL, NULL, NULL, FALSE, FALSE },
};

/******************************************************************************/
/* Private functions                                                          */
/******************************************************************************/
/**
 * Concatenate two strings and use pango layout to produce a string
 * that will contain an hint and a text.
 *
 * \param hint 	Text to use as hint
 * \param text 	Text to use as secondary text
 *
 * \return		a pango formated string with the two arguments concatenated.
 * 				It returns a newly allocated string which must be freed when no more used.
 **/
static gchar *dialogue_make_hint (const gchar *hint,
						 const gchar *text)
{
    gchar *tmp_str = NULL;
    gchar *tmp_markup_str;

    tmp_markup_str = g_markup_printf_escaped ("<span size=\"larger\" weight=\"bold\">%s</span>\n\n", hint);

    if (text && strlen (text))
        tmp_str = g_strconcat (tmp_markup_str, text, NULL);
    else
        tmp_str = g_strdup (tmp_markup_str);

    g_free (tmp_markup_str);

    return tmp_str;
}

/**
 * Display a dialog window with arbitrary icon.
 * WARNING you may need to escape text with g_markup_escape_text()
 * or g_markup_printf_escaped():
 *
 * \param param 	Type of Window to display
 * \param text 		Text to display in window
 * \param hint 		Text to display in window as hint (bold, larger), may be NULL
 *
 * \return
 **/
static void dialogue_special (GtkMessageType param,
							  const gchar *text,
							  const gchar *hint)
{
    GtkWidget *dialog;
    const gchar *primary_text;

    primary_text = hint ? hint : text;

    if (NULL == grisbi_app_get_active_window (NULL))
        return;

    dialog = gtk_message_dialog_new (GTK_WINDOW (grisbi_app_get_active_window (NULL)),
									 GTK_DIALOG_DESTROY_WITH_PARENT,
									 param, GTK_BUTTONS_CLOSE,
									 NULL);

    if (hint)
    {
        gtk_message_dialog_set_markup (GTK_MESSAGE_DIALOG (dialog), dialogue_make_hint (primary_text, NULL));
        gtk_message_dialog_format_secondary_text (GTK_MESSAGE_DIALOG (dialog), "%s", text);
    }
    else
         gtk_message_dialog_set_markup (GTK_MESSAGE_DIALOG (dialog), primary_text);

    gtk_window_set_modal (GTK_WINDOW (dialog), TRUE);
    gtk_dialog_run (GTK_DIALOG (dialog));
    gtk_widget_destroy (dialog);
}

/**
 * Update the value of a message in memory according to the state of a
 * checkbox button.  These messages are conditional dialogs (see
 * dialogue_conditional_new()).  Normally called as an event.
 *
 * \param checbox   GtkCheckbox that triggered this event.
 * \param message   Message ID.
 *
 * \return      FALSE.
 **/
static gboolean dialogue_update_var (GtkWidget *checkbox,
									 gint message)
{
    tab_warning_msg[message].hidden = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON(checkbox));

	return FALSE;
}

/**
 * Create a dialog with an informal text and a checkbox that allow
 * this message not to be displayed again thanks to preferences.
 * WARNING you may need to escape text with g_markup_escape_text()
 * or g_markup_printf_escaped():
 *
 * \param text  Text to be displayed
 * \param var   Variable that both controls whether the dialog will
 *				appear or not and that indicates which variable could
 *				be modified so that this message won't appear again.
 *
 * \return  	A newly-created GtkDialog.
 */
static GtkDialog *dialogue_conditional_new (const gchar *text,
											const gchar *var,
											GtkMessageType type,
											GtkButtonsType buttons)
{
    GtkWidget *checkbox;
    GtkWidget *dialog = NULL;
    GtkWidget *vbox;
	gchar *label;
    gint i;

	if (!var || !strlen (var))
        return NULL;

    for  (i = 0; tab_warning_msg[i].name; i++)
    {
        if (!strcmp (tab_warning_msg[i].name, var))
        {
            if (tab_warning_msg[i].hidden)
            {
                return NULL;
            }
            break;
        }
    }

    dialog = gtk_message_dialog_new (GTK_WINDOW (grisbi_app_get_active_window (NULL)),
									 GTK_DIALOG_DESTROY_WITH_PARENT,
									 type, buttons,
									 NULL);
    gtk_dialog_set_default_response (GTK_DIALOG(dialog), GTK_RESPONSE_CLOSE);
    gtk_message_dialog_set_markup (GTK_MESSAGE_DIALOG (dialog), text);

    vbox = gtk_dialog_get_content_area (GTK_DIALOG (dialog));

	switch (buttons)
	{
		case GTK_BUTTONS_NONE:
		case GTK_BUTTONS_OK:
		case GTK_BUTTONS_CANCEL:
		case GTK_BUTTONS_CLOSE:
			label = _("Do not show this message again");
			break;
		case GTK_BUTTONS_YES_NO:
		case GTK_BUTTONS_OK_CANCEL:
			label = _("Keep this choice and no longer see this message?");
			break;
	}
    checkbox = gtk_check_button_new_with_label (label);
    g_signal_connect (G_OBJECT (checkbox),
					  "toggled",
					  G_CALLBACK (dialogue_update_var),
					  GINT_TO_POINTER (i));
    gtk_box_pack_start (GTK_BOX (vbox), checkbox, TRUE, TRUE, MARGIN_BOX);
    gtk_widget_show_all (checkbox);

    gtk_window_set_modal (GTK_WINDOW (dialog), TRUE);

    return GTK_DIALOG (dialog);
}

/**
 * This function pop ups a dialog with an informal text and a checkbox
 * that allow this message not to be displayed again thanks to
 * preferences.
 *
 * \param text  Text to be displayed
 * \param var   Variable that both controls whether the dialog will
 *				appear or not and that indicates which variable could
 *				be modified so that this message won't appear again.
 *
 * \return
 **/
static void dialogue_conditional_special (const gchar *text,
										  const gchar *var,
										  GtkMessageType type)
{
    GtkDialog *dialog;

    dialog = dialogue_conditional_new (text, var, type, GTK_BUTTONS_CLOSE);
    if (! dialog)
		return;

    gtk_dialog_run (GTK_DIALOG (dialog));
    gtk_widget_destroy (GTK_WIDGET (dialog));
}

/**
 * This function pop ups a dialog with an informal text and a checkbox
 * that allow this message not to be displayed again thanks to
 * preferences.
 *
 * \param 	text text to be displayed
 * \param 	var variable that both controls whether the dialog will
 * 			appear or not and that indicates which variable could be modified
 * 			so that this message won't appear again.
 *
 * \return
 **/
static void dialogue_conditional (const gchar *text,
								  const gchar *var)
{
    dialogue_conditional_special (text, var, GTK_MESSAGE_WARNING);
}

/******************************************************************************/
/* Public functions                                                             */
/******************************************************************************/
/**
 * Display an info dialog window with a hint displayed larger and in
 * bold.
 *
 * \param text	Text to display in window
 * \param hint	Hint to display
 *
 * \return
 **/
void dialogue_hint (const gchar *text,
					const gchar *hint)
{
    dialogue_special (GTK_MESSAGE_INFO, text, hint);
}

/**
 * Display an info dialog window.
 *
 * \param text Text to display in window
 *
 * \return
 **/
void dialogue (const gchar *texte_dialogue)
{
    dialogue_special (GTK_MESSAGE_INFO, texte_dialogue, NULL);
}

/**
 * Display an error dialog window
 *
 * \param text Text to display in window
 *
 * \return
 **/
void dialogue_error (const gchar *text)
{
    dialogue_special (GTK_MESSAGE_ERROR, text, NULL);
}

/**
 * Display an error dialog window with an optional hint.
 *
 * \param text Text to display in window
 * \param hint Text to display in window as hint (bold, larger)
 *
 * \return
 **/
void dialogue_error_hint (const gchar *text,
						  const gchar *hint)
{
    dialogue_special (GTK_MESSAGE_ERROR, text, hint);
}

/**
 * Display a warning dialog window
 *
 * \param text Text to display in window
 *
 * \return
 **/
void dialogue_warning (const gchar *text)
{
    dialogue_special (GTK_MESSAGE_WARNING, text, NULL);
}

/**
 * Display a warning dialog window with an optional hint.
 *
 * \param text Text to display in window
 * \param hint Text to display in window as hint (bold, larger)
 *
 * \return
 **/
void dialogue_warning_hint (const gchar *text,
							const gchar *hint)
{
    dialogue_special (GTK_MESSAGE_WARNING, text, hint);
}

/**
 * Display a dialog window with arbitrary icon.
 * WARNING you may need to escape text with g_markup_escape_text()
 * or g_markup_printf_escaped():
 *
 * \param param 	Type of Window to display
 * \param buttons
 * \param text 		Text to display in window
 * \param hint
 *
 * \return
 **/
GtkWidget *dialogue_special_no_run (GtkMessageType param,
									GtkButtonsType buttons,
									const gchar *text,
									const gchar *hint)
{
    GtkWindow *window;
    GtkWidget *dialog;
    const gchar *primary_text;

    primary_text = hint ? hint : text;
    window = GTK_WINDOW (grisbi_app_get_active_window (NULL));

    if (window)
    {
        dialog = gtk_message_dialog_new (window,
										 GTK_DIALOG_DESTROY_WITH_PARENT | GTK_DIALOG_MODAL,
										 param, buttons,
										 NULL);
    }
    else
    {
        dialog = gtk_message_dialog_new (NULL, 0, param, buttons, NULL);
    }

    if (hint)
    {
        gtk_message_dialog_set_markup (GTK_MESSAGE_DIALOG (dialog), dialogue_make_hint (primary_text, NULL));
        gtk_message_dialog_format_secondary_markup (GTK_MESSAGE_DIALOG (dialog), "%s", text);
    }
    else
         gtk_message_dialog_set_markup (GTK_MESSAGE_DIALOG (dialog), primary_text);

    return dialog;
}

/**
 * Update the value of a message in memory according to the state of a
 * checkbox button.  These messages are conditional dialogs (see
 * dialogue_conditional_new()).  Normally called as an event.
 *
 * \param checkbox	GtkCheckbox that triggered this event.
 * \param message	struct ConditionalMessage Message
 *
 * \return      	FALSE.
 **/
gboolean dialogue_update_struct_message (GtkWidget *checkbox,
										 ConditionalMsg *msg)
{
    msg->hidden = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (checkbox));

    return FALSE;
}

/**
 * This function pop ups a dialog with a hint (first sentence, in
 * bold), an informal text and a checkbox that allow this message not
 * to be displayed again thanks to preferences.  It calls
 * dialogue_conditional to achieve display.
 *
 * \param text	text to be displayed
 * \param hint	hint to be displayed
 * \param var	variable that both controls whether the dialog will
 *				appear or not and that indicates which variable could be modified
 *				so that this message won't appear again.
 *
 * \return
 **/
void dialogue_conditional_hint (const gchar *text,
								const gchar *hint,
								const gchar *var)
{
    dialogue_conditional (dialogue_make_hint(hint, text), var);
}

/**
 * Pop up a warning dialog window with a question and wait for user to
 * press 'YES' or 'NO'. If provided, a hint is displayed on the top of the
 * window larger and in bold.
 *
 * \param texte				Text to be displayed
 * \param hint				Hint to be displayed, may be NULL
 * \param default_answer	GTK_RESPONSE_OK or GTK_RESPONSE_CANCEL, will give the focus to the button
 *
 * \return 					TRUE if user pressed 'YES'. FALSE otherwise.
 **/
gboolean dialogue_yes_no (const gchar *text,
						  const gchar *hint,
						  gint default_answer)
{
    GtkWidget *dialog;
    const gchar *primary_text;
    gint response;

    primary_text = hint ? hint : text;
    dialog = gtk_message_dialog_new (GTK_WINDOW (grisbi_app_get_active_window (NULL)),
									 GTK_DIALOG_DESTROY_WITH_PARENT,
									 GTK_MESSAGE_QUESTION,
									 GTK_BUTTONS_YES_NO,
									 NULL);
    gtk_message_dialog_set_markup (GTK_MESSAGE_DIALOG (dialog), dialogue_make_hint (primary_text, NULL));

    if (hint)
        gtk_message_dialog_format_secondary_text (GTK_MESSAGE_DIALOG (dialog), "%s", text);

    gtk_dialog_set_default_response (GTK_DIALOG (dialog), default_answer);

    response = gtk_dialog_run (GTK_DIALOG (dialog));

	gtk_widget_destroy (dialog);
    if (response == GTK_RESPONSE_YES)
        return TRUE;
    else
        return FALSE;
}

/**
 * Pop up a warning dialog window with a question and a checkbox that allow
 * this message not to be displayed again thanks to preferences and wait
 * for user to press 'YES' or 'NO'.
 *
 * \param var variable that both controls whether the dialog will
 * appear or not and that indicates which variable could be modified
 * so that this message won't appear again.
 *
 * \return TRUE if user pressed 'YES'.  FALSE otherwise.
 */
gboolean dialogue_conditional_yes_no (const gchar *var)
{
    GtkDialog *dialog;
    gchar *text;
    gint i;
    gint response;

    for  (i = 0; tab_warning_msg[i].name; i++)
    {
        if (!strcmp (tab_warning_msg[i].name, var))
        {
            if (tab_warning_msg[i].hidden)
            {
                return tab_warning_msg[i].default_answer;
            }
            break;
        }
    }

	text = dialogue_make_hint (gettext (tab_warning_msg[i].hint), gettext (tab_warning_msg[i].message));
    dialog = dialogue_conditional_new (text, var, GTK_MESSAGE_WARNING, GTK_BUTTONS_YES_NO);
	g_free (text);

    response = gtk_dialog_run (GTK_DIALOG (dialog));

    if (response == GTK_RESPONSE_YES)
        tab_warning_msg[i].default_answer = TRUE;
    else
        tab_warning_msg[i].default_answer = FALSE;

    gtk_widget_destroy (GTK_WIDGET (dialog));

	return tab_warning_msg[i].default_answer;
}

/**
 * Pop up a warning dialog window with a question and a checkbox that allow
 * this message not to be displayed again thanks to preferences and wait
 * for user to press 'YES' or 'NO'.
 *
 * \param 	struct ConditionalMessage
 *
 * \return	TRUE if user pressed 'YES'. FALSE otherwise.
 **/
gboolean dialogue_conditional_yes_no_with_items (const gchar *tab_name,
												 const gchar *struct_name,
												 const gchar *tmp_msg)
{
    GtkWidget *checkbox;
    GtkWidget *dialog;
    GtkWidget *vbox;
	gint msg_no = 0;
    gchar *text;
    gint response;

	if (g_strcmp0 (tab_name, "tab_delete_msg") == 0)
	{
		for  (msg_no = 0; tab_delete_msg[msg_no].name; msg_no++)
		{
			if (strcmp (tab_delete_msg[msg_no].name, struct_name) == 0)
			{
				break;
			}
		}
	}
	else
	{
		for  (msg_no = 0; tab_warning_msg[msg_no].name; msg_no++)
		{
			if (strcmp (tab_warning_msg[msg_no].name, struct_name) == 0)
				break;
		}
	}

    if (tab_delete_msg[msg_no].hidden)
        return tab_delete_msg[msg_no].default_answer;

    text = dialogue_make_hint (gettext (tab_delete_msg[msg_no].hint), tmp_msg);
    dialog = gtk_message_dialog_new (GTK_WINDOW (grisbi_app_get_active_window (NULL)),
									 GTK_DIALOG_DESTROY_WITH_PARENT,
									 GTK_MESSAGE_WARNING,
									 GTK_BUTTONS_YES_NO,
									 NULL);
    gtk_dialog_set_default_response (GTK_DIALOG(dialog), GTK_RESPONSE_NO);
    gtk_message_dialog_set_markup (GTK_MESSAGE_DIALOG (dialog), text);
	g_free (text);

    vbox = gtk_dialog_get_content_area (GTK_DIALOG (dialog));

    checkbox = gtk_check_button_new_with_label (_("Keep this choice and no longer see this message?"));
    g_signal_connect (G_OBJECT (checkbox),
					  "toggled",
                      G_CALLBACK (dialogue_update_struct_message),
                      &tab_delete_msg[msg_no]);
    gtk_box_pack_start (GTK_BOX (vbox), checkbox, TRUE, TRUE, MARGIN_BOX);
    gtk_widget_show_all (checkbox);

    response = gtk_dialog_run (GTK_DIALOG (dialog));

    if (response == GTK_RESPONSE_YES)
        tab_delete_msg[msg_no].default_answer = TRUE;
	else
		tab_delete_msg[msg_no].default_answer = FALSE;

    gtk_widget_destroy (GTK_WIDGET (dialog));

	return tab_delete_msg[msg_no].default_answer;
}

/**
 * Pop up a warning dialog window with a question and a checkbox that allow
 * this message not to be displayed again thanks to preferences and wait
 * for user to press 'YES' or 'NO'.
 *
 * \param 	struct ConditionalMessage
 *
 * \return	TRUE if user pressed 'YES'. FALSE otherwise.
 **/
gboolean dialogue_conditional_yes_no_with_struct (ConditionalMsg *msg)
{
    GtkWidget *checkbox;
    GtkWidget *dialog;
    GtkWidget *vbox;
    gchar *text;
    gint response;

    if (msg->hidden)
        return msg->default_answer;

    text = dialogue_make_hint (gettext (msg->hint), msg->message);
    dialog = gtk_message_dialog_new (GTK_WINDOW (grisbi_app_get_active_window (NULL)),
									 GTK_DIALOG_DESTROY_WITH_PARENT,
									 GTK_MESSAGE_WARNING,
									 GTK_BUTTONS_YES_NO,
									 NULL);
    gtk_dialog_set_default_response (GTK_DIALOG(dialog), GTK_RESPONSE_NO);
    gtk_message_dialog_set_markup (GTK_MESSAGE_DIALOG (dialog), text);
	g_free (text);

    vbox = gtk_dialog_get_content_area (GTK_DIALOG (dialog));

    checkbox = gtk_check_button_new_with_label (_("Keep this choice and no longer see this message?"));
    g_signal_connect (G_OBJECT (checkbox),
					  "toggled",
                      G_CALLBACK (dialogue_update_struct_message),
                      msg);
    gtk_box_pack_start (GTK_BOX (vbox), checkbox, TRUE, TRUE, MARGIN_BOX);
    gtk_widget_show_all (checkbox);

    response = gtk_dialog_run (GTK_DIALOG (dialog));

    if (response == GTK_RESPONSE_YES)
        msg->default_answer = TRUE;
    else
        msg->default_answer = FALSE;

    gtk_widget_destroy (GTK_WIDGET (dialog));

	return msg->default_answer;
}

/**
 * return the number of message
 *
 * \param	struct ConditionalMessage
 * \param	name of message
 *
 * \return	message number or -1 is not present.
 */
gint dialogue_conditional_yes_no_get_no_struct (ConditionalMsg *msg,
												const gchar *name)
{
    gint i;

    for  (i = 0; msg[i].name; i++)
    {
        if (strcmp (msg[i].name, name) == 0)
            return i;
    }
    return -1;
}

/**
 * use pango layout to produce a string that will contain a text.
 *
 * \param	text Text
 *
 * \return	a pango formated string It returns a newly allocated string which must
 *			be freed when no more used.
 **/
gchar *dialogue_make_red (const gchar *text)
{
    gchar *tmp_str;

    tmp_str = g_markup_printf_escaped ("<span foreground=\"red\">%s</span>", text);

    return tmp_str;
}

/**
 * use pango layout to produce a string that will contain a text.
 *
 * \param attribut  exemple foreground=\"red\", size=\"x-large\"
 * \param text      Text
 *
 * \return			a pango formated string It returns a newly allocated string which must
 *					be freed when no more used.
 */
gchar *dialogue_make_pango_attribut (const gchar *attribut,
							const gchar *text)
{
    gchar *span_format;
    gchar *tmp_str;

    span_format = g_strconcat ("<span ", attribut, ">%s</span>", NULL);
    tmp_str = g_markup_printf_escaped (span_format , text);
    g_free (span_format);

    return tmp_str;
}

/**
 * use pango layout to produce a string that will contain a text.
 *
 * \param text Text
 *
 * \return a pango formated string It returns a newly allocated string which must
 *         be freed when no more used.
 **/
gchar *dialogue_make_blue (const gchar *text)
{
    gchar *tmp_str;

    tmp_str = g_markup_printf_escaped ("<span foreground=\"blue\">%s</span>", text);

    return tmp_str;
}

/**
 * Displays a dialog message stating that bad things will happen
 * soon.  This dialog should not appear theorically, but it is here
 * just in case to spot bugs.
 *
 * \param
 *
 * \return
 **/
void dialogue_error_brain_damage (void)
{
    dialogue_error_hint (_("Hi, you are in the middle of nowhere, between two lines of code."
						   " Grisbi is expected to crash very soon. Have a nice day."),
						 _("Serious brain damage expected."));
}

/**
 * Displays a dialog message stating that bad things will happen
 * soon.  This dialog should not appear theorically, but it is here
 * just in case to spot bugs.
 *
 * \param
 *
 * \return
 **/
void dialogue_error_memory (void)
{
    dialogue_error_hint (_("Bad things will happen soon.  Be sure to save any modification "
						   "in a separate file in case Grisbi would corrupt files."),
						 _("Cannot allocate memory"));

}


/**
 *
 *
 * \param
 *
 * \return
 **/
void dialogue_message (const gchar *label, ...)
{
    va_list ap;
    gint i = 0;

    while (tab_warning_msg[i].name)
    {
		if (!strcmp (tab_warning_msg[i].name, label))
		{
			if (! tab_warning_msg[i].hidden)
			{
				gchar hint_buffer[1024];
				gchar message_buffer[1024];

				va_start (ap, label);
				vsnprintf (hint_buffer, sizeof hint_buffer, _(tab_warning_msg[i].hint), ap);
				vsnprintf (message_buffer, sizeof message_buffer, _(tab_warning_msg[i].message), ap);

				dialogue_conditional_hint (message_buffer,hint_buffer, tab_warning_msg[i].name);
				va_end(ap);
			}
			return;
		}
		i ++;
    }
}

/**
 * Display an info dialog window with a hint displayed larger and in
 * bold.
 * add an entry and return the content of the entry
 *
 * \param text 					Text to display in window
 * \param hint 					Hint to display
 * \param entry_description 	label to set in front of the entry
 *
 * \return
 **/
gchar *dialogue_hint_with_entry (const gchar *text,
								 const gchar *hint,
								 const gchar *entry_description)
{
    GtkWidget *dialog;
    const gchar *primary_text;
    GtkWidget *entry;
    GtkWidget *hbox;
    GtkWidget *label;
    gchar *string;

    primary_text = hint ? hint : text;
    dialog = gtk_message_dialog_new (GTK_WINDOW (grisbi_app_get_active_window (NULL)),
									 GTK_DIALOG_DESTROY_WITH_PARENT,
									 GTK_MESSAGE_INFO,
									 GTK_BUTTONS_CLOSE,
									 NULL);
    gtk_message_dialog_set_markup (GTK_MESSAGE_DIALOG (dialog), primary_text);

    if (hint)
        gtk_message_dialog_format_secondary_text (GTK_MESSAGE_DIALOG (dialog), "%s", text);

    hbox = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, MARGIN_BOX);
    gtk_box_pack_start (GTK_BOX (gtk_dialog_get_content_area (GTK_DIALOG (dialog))), hbox, FALSE, FALSE, 0);

    label = gtk_label_new (entry_description);
    gtk_box_pack_start (GTK_BOX (hbox), label, FALSE, FALSE, 0);

    entry = gtk_entry_new ();
    gtk_box_pack_start (GTK_BOX (hbox), entry, FALSE, FALSE, 0);

    gtk_widget_show_all (dialog);

    gtk_window_set_modal (GTK_WINDOW (dialog), TRUE);
    gtk_dialog_run (GTK_DIALOG (dialog));
    string = g_strdup (gtk_entry_get_text (GTK_ENTRY (entry)));
    gtk_widget_destroy (dialog);

    return string;
}

/**
 * return the content_area of dialog
 *
 * \param	dialog
 *
 * \return	the content_area
 **/
GtkWidget *dialog_get_content_area (GtkWidget *dialog)
{
    return gtk_dialog_get_content_area (GTK_DIALOG (dialog));
}

/**
 *
 *
 * \param
 *
 * \return
 **/
gpointer dialogue_get_tab_delete_msg (void)
{
	return &tab_delete_msg[0];
}

/**
 *
 *
 * \param
 *
 * \return
 **/
gpointer dialogue_get_tab_warning_msg (void)
{
	return &tab_warning_msg[0];
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
