/* ************************************************************************** */
/*                                                                            */
/*     Copyright (C)    2000-2008 Cédric Auger (cedric@grisbi.org)            */
/*          2003-2008 Benjamin Drieu (bdrieu@april.org)                       */
/*                      2008-2009 Pierre Biava (grisbi@pierre.biava.name)     */
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


/*START_INCLUDE*/
#include "dialog.h"
#include "parametres.h"
/*END_INCLUDE*/

/*START_STATIC*/
static void dialogue_conditional ( gchar *text, gchar *var );
static GtkDialog *dialogue_conditional_new ( gchar *text,
                        gchar *var,
                        GtkMessageType type,
                        GtkButtonsType buttons );
static void dialogue_conditional_special ( gchar *text, gchar *var, GtkMessageType type );
static gboolean dialogue_update_var ( GtkWidget *checkbox, gint message );
/*END_STATIC*/


/*START_EXTERN*/
extern GtkWidget *window;
/*END_EXTERN*/


/** All messages */
struct conditional_message messages[] =
{
    { "ofx-security-not-implemented", N_("Security feature not implemented"),
      N_("This file contains security information, which processing is not implemented at "
      "this moment."),
      FALSE, FALSE, },

    { "encryption-is-irreversible", N_("Encryption is irreversible."),
      N_("Grisbi encrypts files in a very secure way that does not allow recovery without "
      "original password.  It means that if you forget your password, you will loose all "
      "your data.  Use with caution.\n\nI repeat: if you ever forget your password, there "
      "is no coming back, we cannot help you."), 
      FALSE, FALSE, },

    { "account-file-readable",  N_("Account file is world readable."),
      N_("Your account file should not be readable by anybody else, but it is. You should "
      "change its permissions."),
      FALSE, FALSE, },

    { "account-already-opened", N_("File \"%s\" is already opened"),
      N_("Either this file is already opened by another user or it wasn't closed correctly "
      "(maybe Grisbi crashed?).\nGrisbi can't save the file unless you activate the "
      "\"Force saving locked files\" option in setup."),
      FALSE, FALSE, },
     
    { "minimum-balance-alert", N_("Account under desired balance."),
      N_("Grisbi detected that an account is under a desired balance: %s"), 
      FALSE, FALSE, },
    
    { "no-budgetary-line", N_("No budgetary line was entered"),
      N_("This transaction has no budgetary line entered.  You should use them to "
      "easily produce budgets and make reports on them."),
      FALSE, FALSE, },

    { "recover-split", N_("Recover split?"),
      N_("This is a split of transaction, associated transactions can be recovered as "
      "in last transaction with this payee.  Do you want to recover them?"),
      FALSE, FALSE, },

    { "no-inconsistency-found", N_("No inconsistency found."),
      N_("Grisbi found no known inconsistency in accounts processed."),
      FALSE, FALSE, },

    { "reconcile-transaction", N_("Confirmation of manual (un)reconciliation"),
      N_("You are trying to reconcile or unreconcile a transaction manually, "
	  "which is not a recommended action.\n"
      "Are you really sure you know what you are doing?"), 
      FALSE, FALSE, },

    { "reconcile-start-end-dates", N_("Reconcile start and end dates."),
      N_("In previous versions, Grisbi did not save start date, end date and balance for "
      "reconciliation.  This is now done, so Grisbi will try to guess values from your "
      "accounts.  Thought this can not harm data coherence, false values can be guessed. "
      "Please check in the Preferences window for more information."), 
      FALSE, FALSE, },

    { "development-version", N_("You are running Grisbi version %s"), 
      N_("Warning, please be aware that the version you run is a DEVELOPMENT version. "
      "Never use your original file grisbi: you could make it unusable.\n"
      "Make a copy now."),
      FALSE, FALSE },

    { "gtk_obsolete", N_("You are running Grisbi with GTK version %s"), 
      N_("Warning, please be aware that the version you run is a DEVELOPMENT version. "
     "In any case do not work with this version on your original accounting files. "
     "(File format may change and render files incompatible with previous versions)."),
      FALSE, FALSE },

/*
    { "", N_(),
      N_(), 
      FALSE, FALSE, },
*/
    { NULL },
};


/**
 * Display an info dialog window with a hint displayed larger and in
 * bold.
 *
 * \param text Text to display in window
 * \param hint Hint to display
 */
void dialogue_hint ( gchar *text, gchar *hint )
{
    dialogue_special ( GTK_MESSAGE_INFO, make_hint(hint, text) );
}


/**
 * Display an info dialog window.
 *
 * \param text Text to display in window
 */
void dialogue ( gchar *texte_dialogue )
{
    dialogue_special ( GTK_MESSAGE_INFO, texte_dialogue );
}


/**
 * Display an error dialog window
 *
 * \param text Text to display in window
 */
G_MODULE_EXPORT void dialogue_error ( gchar *text )
{
    dialogue_special ( GTK_MESSAGE_ERROR, text );
}


/**
 * Display an error dialog window with an optional hint.
 *
 * \param text Text to display in window
 * \param hint Text to display in window as hint (bold, larger)
 */
G_MODULE_EXPORT void dialogue_error_hint ( const gchar *text, gchar *hint )
{
    dialogue_special ( GTK_MESSAGE_ERROR, make_hint (hint, text) );
}


/**
 * Display a warning dialog window
 *
 * \param text Text to display in window
 */
G_MODULE_EXPORT void dialogue_warning ( gchar *text )
{
    dialogue_special ( GTK_MESSAGE_WARNING, text );
}


/**
 * Display a warning dialog window with an optional hint.
 *
 * \param text Text to display in window
 * \param hint Text to display in window as hint (bold, larger)
 */
void dialogue_warning_hint ( gchar *text, gchar *hint )
{
    dialogue_special ( GTK_MESSAGE_WARNING, make_hint (hint, text) );
}


/**
 * Display a dialog window with arbitrary icon.
 * WARNING you may need to escape text with g_markup_escape_text() 
 * or g_markup_printf_escaped():
 *
 * \param param Type of Window to display
 * \param text Text to display in window
 */
void dialogue_special ( GtkMessageType param, gchar *text )
{
    GtkWidget *dialog;

    dialog = gtk_message_dialog_new ( GTK_WINDOW (window), 
                        GTK_DIALOG_DESTROY_WITH_PARENT,
                        param, GTK_BUTTONS_CLOSE,
                        "%s", text );
    gtk_label_set_markup ( GTK_LABEL ( GTK_MESSAGE_DIALOG (dialog)->label ), text );

    gtk_window_set_modal ( GTK_WINDOW ( dialog ), TRUE );
    gtk_dialog_run (GTK_DIALOG (dialog));
    gtk_widget_destroy ( dialog );
}


/**
 * Display a dialog window with arbitrary icon.
 * WARNING you may need to escape text with g_markup_escape_text() 
 * or g_markup_printf_escaped():
 * 
 * \param param Type of Window to display
 * \param text Text to display in window
 */
GtkWidget *dialogue_special_no_run ( GtkMessageType param,
                        GtkButtonsType buttons,
                        gchar *text )
{
    GtkWidget *dialog;

    dialog = gtk_message_dialog_new ( GTK_WINDOW (window), 
                        GTK_DIALOG_DESTROY_WITH_PARENT,
                        param, buttons,
                        "%s", text );
    gtk_label_set_markup ( GTK_LABEL ( GTK_MESSAGE_DIALOG(dialog)->label ), text );

    gtk_window_set_modal ( GTK_WINDOW ( dialog ), TRUE );

    return dialog;
}


/**
 * Update the value of a message in memory according to the state of a
 * checkbox button.  These messages are conditional dialogs (see
 * dialogue_conditional_new()).  Normally called as an event.
 *
 * \param checbox   GtkCheckbox that triggered this event.
 * \param message   struct conditional_message Message
 *
 * \return      FALSE.
 */
gboolean dialogue_update_struct_message ( GtkWidget *checkbox,
                        struct conditional_message *message )
{
    message -> hidden = gtk_toggle_button_get_active ( GTK_TOGGLE_BUTTON ( checkbox ) );

    return FALSE;
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
 */
gboolean dialogue_update_var ( GtkWidget *checkbox, gint message )
{
    messages[message].hidden = gtk_toggle_button_get_active ( GTK_TOGGLE_BUTTON(checkbox) );

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
 *      appear or not and that indicates which variable could
 *      be modified so that this message won't appear again.
 *
 * \return  A newly-created GtkDialog.
 */
GtkDialog *dialogue_conditional_new ( gchar *text,
                        gchar *var,
                        GtkMessageType type,
                        GtkButtonsType buttons )
{
    GtkWidget * vbox, * checkbox, *dialog = NULL;
    int i;

    if ( !var || !strlen ( var ) )
        return NULL;

    for  ( i = 0; messages[i].name; i++ )
    {
        if ( !strcmp ( messages[i].name, var ) )
        {
            if ( messages[i].hidden )
            {
                return NULL;
            }
            break;
        }
    }

    dialog = gtk_message_dialog_new ( GTK_WINDOW (window),
                        GTK_DIALOG_DESTROY_WITH_PARENT,
                        type, buttons,
                        "%s", text );
    gtk_dialog_set_default_response ( GTK_DIALOG( dialog ), GTK_RESPONSE_CLOSE );
    gtk_label_set_markup ( GTK_LABEL ( GTK_MESSAGE_DIALOG(dialog)->label ), text );

    vbox = GTK_DIALOG(dialog) -> vbox;

    checkbox = gtk_check_button_new_with_label ( _("Do not show this message again") );
    g_signal_connect ( G_OBJECT ( checkbox ), "toggled", 
                        G_CALLBACK ( dialogue_update_var ), GINT_TO_POINTER ( i ) );
    gtk_box_pack_start ( GTK_BOX ( vbox ), checkbox, TRUE, TRUE, 6 );
    gtk_widget_show_all ( checkbox );

    gtk_window_set_modal ( GTK_WINDOW ( dialog ), TRUE );

    return GTK_DIALOG (dialog);
}


/**
 * This function pop ups a dialog with a hint (first sentence, in
 * bold), an informal text and a checkbox that allow this message not
 * to be displayed again thanks to preferences.  It calls
 * dialogue_conditional to achieve display.
 *
 * \param text text to be displayed
 * \param hint hint to be displayed
 * \param var variable that both controls whether the dialog will
 * appear or not and that indicates which variable could be modified
 * so that this message won't appear again.
 */
void dialogue_conditional_hint ( gchar *hint, gchar *text, gchar *var )
{
    dialogue_conditional ( make_hint(hint, text), var );
}


/**
 * This function pop ups a dialog with an informal text and a checkbox
 * that allow this message not to be displayed again thanks to
 * preferences.
 *
 * \param text text to be displayed
 * \param var variable that both controls whether the dialog will
 * appear or not and that indicates which variable could be modified
 * so that this message won't appear again.
 */
void dialogue_conditional ( gchar *text, gchar *var )
{
    dialogue_conditional_special ( text, var, GTK_MESSAGE_WARNING );
}


/**
 * This function pop ups a dialog with an informal text and a checkbox
 * that allow this message not to be displayed again thanks to
 * preferences.
 *
 * \param text  Text to be displayed
 * \param var   Variable that both controls whether the dialog will
 *      appear or not and that indicates which variable could
 *      be modified so that this message won't appear again.
 */
void dialogue_conditional_special ( gchar *text, gchar *var, GtkMessageType type )
{
    GtkDialog *dialog;

    dialog = dialogue_conditional_new ( text, var, type, GTK_BUTTONS_CLOSE );
    if ( ! dialog )
    return;

    gtk_dialog_run ( GTK_DIALOG (dialog) );
    gtk_widget_destroy ( GTK_WIDGET (dialog));
}


/**
 * Pop up a warning dialog window with a question and wait for user to
 * press 'OK' or 'Cancel'.  A hint is displayed on the top of the
 * window larger and in bold.
 *
 * \param hint Hint to be displayed
 * \param texte Text to be displayed
 * \param default_answer GTK_RESPONSE_OK or GTK_RESPONSE_CANCEL, will give the focus to the button
 *
 * \return TRUE if user pressed 'OK'.  FALSE otherwise.
 */
gboolean question_yes_no_hint ( gchar *hint,
                        gchar *texte,
                        gint default_answer )
{
    return question_yes_no ( make_hint ( hint, texte ), default_answer );
}


/**
 * Pop up a warning dialog window with a question and wait for user to
 * press 'YES' or 'NO'.
  * WARNING you may need to escape text with g_markup_escape_text() 
 * or g_markup_printf_escaped():
 *
 * \param texte  Text to be displayed
 * \param default_answer GTK_RESPONSE_OK or GTK_RESPONSE_CANCEL, will give the focus to the button
 *
 * \return TRUE if user pressed 'YES'.  FALSE otherwise.
 */
gboolean question_yes_no ( gchar *text, gint default_answer )
{
    GtkWidget *dialog;
    gint response;

    dialog = gtk_message_dialog_new ( GTK_WINDOW (window),
                        GTK_DIALOG_DESTROY_WITH_PARENT,
                        GTK_MESSAGE_QUESTION,
                        GTK_BUTTONS_YES_NO,
                        "%s", text );
    gtk_label_set_markup ( GTK_LABEL ( GTK_MESSAGE_DIALOG(dialog)->label ), text );

    gtk_dialog_set_default_response (GTK_DIALOG (dialog), default_answer );

    response = gtk_dialog_run (GTK_DIALOG (dialog));
    gtk_widget_destroy ( dialog );

    if ( response == GTK_RESPONSE_YES )
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
gboolean question_conditional_yes_no ( gchar *var )
{
    gint response, i;
    GtkDialog *dialog;

    for  ( i = 0; messages[i].name; i++ )
    {
        if ( !strcmp ( messages[i].name, var ) )
        {
            if ( messages[i].hidden )
            {
                return messages[i].default_answer;
            }
            break;
        }
    }

    dialog = dialogue_conditional_new ( make_hint ( _(messages[i].hint),
                        _(messages[i].message)),
                        var,
                        GTK_MESSAGE_WARNING,
                        GTK_BUTTONS_YES_NO );

    response = gtk_dialog_run (GTK_DIALOG (dialog));

    if ( response == GTK_RESPONSE_YES )
        messages[i].default_answer = TRUE;
    else
        messages[i].default_answer = FALSE;

    gtk_widget_destroy (GTK_WIDGET (dialog));
    return messages[i].default_answer;
}


/**
 * Pop up a warning dialog window with a question and a checkbox that allow
 * this message not to be displayed again thanks to preferences and wait
 * for user to press 'YES' or 'NO'.
 *
 * \param struct conditional_message
 *
 * \return TRUE if user pressed 'YES'.  FALSE otherwise.
 */
gboolean question_conditional_yes_no_with_struct ( struct conditional_message *message )
{
    GtkWidget *dialog, *vbox, *checkbox;
    gchar *text;
    gint response;

    if ( message -> hidden )
        return message -> default_answer;

    text = make_hint ( _(message -> hint), message -> message );
    dialog = gtk_message_dialog_new ( GTK_WINDOW (window),
                        GTK_DIALOG_DESTROY_WITH_PARENT,
                        GTK_MESSAGE_WARNING,
                        GTK_BUTTONS_YES_NO,
                        "%s", text );
    gtk_dialog_set_default_response ( GTK_DIALOG( dialog ), GTK_RESPONSE_NO );
    gtk_label_set_markup ( GTK_LABEL ( GTK_MESSAGE_DIALOG ( dialog )->label ), text );

    vbox = GTK_DIALOG ( dialog ) -> vbox;

    checkbox = gtk_check_button_new_with_label ( _("Do not show this message again") );
    g_signal_connect ( G_OBJECT ( checkbox ),
                        "toggled", 
                        G_CALLBACK ( dialogue_update_struct_message ),
                        message );
    gtk_box_pack_start ( GTK_BOX ( vbox ), checkbox, TRUE, TRUE, 6 );
    gtk_widget_show_all ( checkbox );

    response = gtk_dialog_run (GTK_DIALOG (dialog));

    if ( response == GTK_RESPONSE_YES )
        message -> default_answer = TRUE;
    else
        message -> default_answer = FALSE;

    gtk_widget_destroy (GTK_WIDGET (dialog));
    return message -> default_answer;
}


/**
 * return the number of message
 *
 * \param struct conditional_message
 * \param name of message
 *
 * \return message number or -1 is not present.
 */
gint question_conditional_yes_no_get_no_struct ( struct conditional_message *msg,
                        gchar *name )
{
    gint i;

    for  ( i = 0; msg[i].name; i++ )
    {
        if ( strcmp ( msg[i].name, name ) == 0 )
            return i;
    }
    return -1;
}


/**
 * Concatenate two strings and use pango layout to produce a string
 * that will contain an hint and a text.
 *
 * \param hint Text to use as hint
 * \param text Text to use as secondary text
 *
 * \return a pango formated string with the two arguments concatenated. It returns a newly allocated string which must be freed when no more used.
 */
gchar *make_hint ( const gchar *hint, const gchar *text )
{
    gchar *tmp_str = NULL;
    gchar *tmp_markup_str;

    tmp_markup_str = g_markup_printf_escaped (
                     "<span size=\"larger\" weight=\"bold\">%s</span>\n\n", hint );

    if ( text && strlen ( text ) )
        tmp_str = g_strconcat ( tmp_markup_str, text, NULL );
    else
        tmp_str = g_strdup ( tmp_markup_str );

    g_free ( tmp_markup_str );

    return tmp_str;
}


/**
 * use pango layout to produce a string that will contain a text.
 *
 * \param text Text 
 *
 * \return a pango formated string It returns a newly allocated string which must 
 *         be freed when no more used.
 */
gchar *make_red ( gchar *text )
{
    gchar *tmpstr;

    tmpstr = g_markup_printf_escaped ( "<span foreground=\"red\">%s</span>", text );

    return tmpstr;
}


/**
 * use pango layout to produce a string that will contain a text.
 *
 * \param attribut  exemple foreground=\"red\", size=\"x-large\"
 * \param text      Text 
 *
 * \return a pango formated string It returns a newly allocated string which must 
 *         be freed when no more used.
 */
gchar *make_pango_attribut ( gchar *attribut, gchar *text )
{
    gchar *tmpstr;

    tmpstr = g_markup_printf_escaped (
                        g_strconcat ( "<span ", attribut, ">%s</span>", NULL ), text );

    return tmpstr;
}


/**
 * use pango layout to produce a string that will contain a text.
 *
 * \param text Text 
 *
 * \return a pango formated string It returns a newly allocated string which must 
 *         be freed when no more used.
 */
gchar *make_blue ( gchar *text )
{
    gchar *tmpstr;

    tmpstr = g_markup_printf_escaped ( "<span foreground=\"blue\">%s</span>", text );

    return tmpstr;
}


/**
 * Displays a dialog message stating that bad things will happen
 * soon.  This dialog should not appear theorically, but it is here
 * just in case to spot bugs.
 */
void dialogue_error_brain_damage ()
{
    dialogue_error_hint ( _("Hi, you are in the middle of nowhere, between two lines of code."
                        " Grisbi is expected to crash very soon. Have a nice day."), 
                        _("Serious brain damage expected.") );
}


/**
 * Displays a dialog message stating that bad things will happen
 * soon.  This dialog should not appear theorically, but it is here
 * just in case to spot bugs.
 */
void dialogue_error_memory ()
{
    dialogue_error_hint ( _("Bad things will happen soon.  Be sure to save any modification "
                        "in a separate file in case Grisbi would corrupt files."),
                        _("Cannot allocate memory") );

}


/**
 *
 *
 *
 */
void dialog_message ( gchar *label, ... )
{
    va_list ap;
    int i = 0;

    while ( messages[i] . name )
    {
	if ( !strcmp ( messages[i] . name, label ) )
	{
	    if ( ! messages[i] . hidden )
	    {
		gchar hint_buffer[1024], message_buffer[1024];
		va_start ( ap, label );
		vsnprintf ( hint_buffer, 1024, _(messages[i] . hint), ap );
		vsnprintf ( message_buffer, 1024, _(messages[i] . message), ap );

		dialogue_conditional_hint ( hint_buffer, message_buffer,
					    messages[i] . name );
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
 * \param text 			Text to display in window
 * \param hint 			Hint to display
 * \param entry_description 	label to set in front of the entry
 */
const gchar *dialogue_hint_with_entry ( gchar *text, gchar *hint, gchar *entry_description )
{
    GtkWidget *dialog;
    gchar *format_text;
    GtkWidget *entry;
    GtkWidget *hbox;
    GtkWidget *label;
    const gchar *string;

    format_text = make_hint ( hint, text );

    dialog = gtk_message_dialog_new ( GTK_WINDOW (window), 
                        GTK_DIALOG_DESTROY_WITH_PARENT,
                        GTK_MESSAGE_INFO, 
                        GTK_BUTTONS_CLOSE,
                        "%s", format_text );
    gtk_label_set_markup ( GTK_LABEL ( GTK_MESSAGE_DIALOG(dialog)->label ), format_text );

    hbox = gtk_hbox_new (FALSE, 5);
    gtk_box_pack_start ( GTK_BOX (GTK_DIALOG (dialog) -> vbox),
                        hbox,
                        FALSE, FALSE, 0);

    label = gtk_label_new (entry_description);
    gtk_box_pack_start ( GTK_BOX (hbox), label, FALSE, FALSE, 0);

    entry = gtk_entry_new ();
    gtk_box_pack_start ( GTK_BOX (hbox), entry, FALSE, FALSE, 0);

    gtk_widget_show_all (dialog);

    gtk_window_set_modal ( GTK_WINDOW ( dialog ), TRUE );
    gtk_dialog_run (GTK_DIALOG (dialog));
    string = gtk_entry_get_text (GTK_ENTRY (entry));
    gtk_widget_destroy ( dialog );

    return string;
}
/* Local Variables: */
/* c-basic-offset: 4 */
/* End: */
