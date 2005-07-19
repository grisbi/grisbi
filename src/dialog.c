/* ************************************************************************** */
/* Fichier qui s'occupe de la page d'accueil ( de démarrage lors de           */
/* l'ouverture d'un fichier de comptes                                        */
/*                                                                            */
/*                                  accueil.c                                 */
/*                                                                            */
/*     Copyright (C)	2000-2003 Cédric Auger (cedric@grisbi.org)	      */
/*			2003-2004 Benjamin Drieu (bdrieu@april.org)	      */
/* 			http://www.grisbi.org				      */
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

#include "include.h"


/*START_INCLUDE*/
#include "dialog.h"
#include "parametres.h"
#include "include.h"
#include "dialog.h"
/*END_INCLUDE*/

/*START_STATIC*/
static gchar *demande_texte ( gchar *titre_fenetre,
		       gchar *question );
static void dialogue_conditional_special ( gchar *text, gchar * var, GtkMessageType type );
static gboolean question_yes_no ( gchar *texte );
/*END_STATIC*/



/*START_EXTERN*/
extern GtkWidget *window;
/*END_EXTERN*/


/** All messages */
struct conditional_message messages[] =
{
    { "no-tip-available", N_("No tip of the day available"), 
      N_("Make sure that grisbi was correctly installed and that tips file is readable."),
      FALSE, FALSE, },
    { "qif-does-not-define-currencies", N_("QIF format does not define currencies."),
      N_("All transactions will be converted into currency of their account."), 
      FALSE, FALSE, },
    { "encryption-is-irreversible", N_("Encryption is irreversible."),
      N_("Grisbi encrypts files in a secure way that does not allow recovery without original password.  This means that if you forget your password, you will loose all your data.  Use with caution.\n\nI repeat: if you forget your password, there is no coming back, don't complain to us."), 
      FALSE, FALSE, },
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
void dialogue_error ( gchar *text )
{
    dialogue_special ( GTK_MESSAGE_ERROR, text );
}



/**
 * Display an error dialog window with an optional hint.
 *
 * \param text Text to display in window
 * \param hint Text to display in window as hint (bold, larger)
 */
void dialogue_error_hint ( gchar *text, gchar * hint )
{
    dialogue_special ( GTK_MESSAGE_ERROR, make_hint(hint, text) );
}



/**
 * Display a warning dialog window
 *
 * \param text Text to display in window
 */
void dialogue_warning ( gchar *text )
{
    dialogue_special ( GTK_MESSAGE_WARNING, text );
}



/**
 * Display a warning dialog window with an optional hint.
 *
 * \param text Text to display in window
 * \param hint Text to display in window as hint (bold, larger)
 */
void dialogue_warning_hint ( gchar *text, gchar * hint )
{
    dialogue_special ( GTK_MESSAGE_WARNING, make_hint(hint, text) );
}



/**
 * Display a dialog window with arbitrary icon.
 *
 * \param param Type of Window to display
 * \param text Text to display in window
 */
void dialogue_special ( GtkMessageType param, gchar * text )
{
    GtkWidget *dialog;

    dialog = gtk_message_dialog_new ( GTK_WINDOW (window), 
				      GTK_DIALOG_DESTROY_WITH_PARENT,
				      param, GTK_BUTTONS_CLOSE, text );
    gtk_label_set_markup ( GTK_LABEL ( GTK_MESSAGE_DIALOG(dialog)->label ), text );

    gtk_window_set_modal ( GTK_WINDOW ( dialog ), TRUE );
    gtk_dialog_run (GTK_DIALOG (dialog));
    gtk_widget_destroy ( dialog );
}



/**
 * Display a dialog window with arbitrary icon.
 *
 * \param param Type of Window to display
 * \param text Text to display in window
 */
GtkWidget * dialogue_special_no_run ( GtkMessageType param, GtkButtonsType buttons,
				      gchar * text )
{
    GtkWidget *dialog;

    dialog = gtk_message_dialog_new ( GTK_WINDOW (window), 
				      GTK_DIALOG_DESTROY_WITH_PARENT,
				      param, buttons, text );
    gtk_label_set_markup ( GTK_LABEL ( GTK_MESSAGE_DIALOG(dialog)->label ), text );

    gtk_window_set_modal ( GTK_WINDOW ( dialog ), TRUE );

    return dialog;
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
void dialogue_conditional_hint ( gchar *hint, gchar * text, gchar * var )
{
    dialogue_conditional ( make_hint(hint, text), var );
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
void dialogue_conditional_info_hint ( gchar *hint, gchar * text, gchar * var )
{
    dialogue_conditional_special ( make_hint(hint, text), var, GTK_MESSAGE_INFO );
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
void dialogue_conditional ( gchar *text, gchar * var )
{
    dialogue_conditional_special ( text, var, GTK_MESSAGE_WARNING );
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
void dialogue_conditional_special ( gchar *text, gchar * var, GtkMessageType type )
{
    GtkWidget * vbox, * checkbox, *dialog;
    int i;

    if ( !var || !strlen ( var ) )
	return;

    for  ( i = 0; messages[i].name; i++ )
    {
	if ( !strcmp ( messages[i].name, var ) )
	{
	    if ( messages[i].hidden )
	    {
		return;
	    }
	    break;
	}
    }

    dialog = gtk_message_dialog_new ( GTK_WINDOW (window),
				      GTK_DIALOG_DESTROY_WITH_PARENT,
				      type,
				      GTK_BUTTONS_CLOSE,
				      text );
    gtk_dialog_set_default_response ( GTK_DIALOG( dialog ),
				      GTK_RESPONSE_CLOSE );
    gtk_label_set_markup ( GTK_LABEL ( GTK_MESSAGE_DIALOG(dialog)->label ), text );

    vbox = GTK_DIALOG(dialog) -> vbox;

    checkbox = gtk_check_button_new_with_label ( _("Do not show this message again") );
    gtk_box_pack_start ( GTK_BOX ( vbox ), checkbox, TRUE, TRUE, 6 );
    gtk_widget_show_all ( checkbox );

    gtk_window_set_modal ( GTK_WINDOW ( dialog ), TRUE );
    gtk_dialog_run ( GTK_DIALOG (dialog) );

    messages[i].hidden = gtk_toggle_button_get_active ( GTK_TOGGLE_BUTTON(checkbox) );
    printf ("-> %d\n", messages[i].hidden );

    gtk_widget_destroy ( dialog );
}



/**
 * Alias for question_yes_no().
 *
 * \deprecated This should not be used in newly written code.  Use
 * question_yes_no() instead.
 */
gboolean question ( gchar *texte )
{
    return question_yes_no ( texte );
}



/**
 * Pop up a warning dialog window with a question and wait for user to
 * press 'OK' or 'Cancel'.  A hint is displayed on the top of the
 * window larger and in bold.
 *
 * \param hint Hint to be displayed
 * \param texte Text to be displayed
 *
 * \return TRUE if user pressed 'OK'.  FALSE otherwise.
 */
gboolean question_yes_no_hint ( gchar * hint, gchar *texte )
{
    return question_yes_no ( make_hint ( hint, texte ) );
}



/**
 * Pop up a warning dialog window with a question and wait for user to
 * press 'OK' or 'Cancel'.
 *
 * \param texte  Text to be displayed
 *
 * \return TRUE if user pressed 'OK'.  FALSE otherwise.
 */
gboolean question_yes_no ( gchar *texte )
{
    GtkWidget *dialog;
    gint response;

    dialog = gtk_message_dialog_new ( GTK_WINDOW (window),
				      GTK_DIALOG_DESTROY_WITH_PARENT,
				      GTK_MESSAGE_QUESTION,
				      GTK_BUTTONS_OK_CANCEL,
				      texte );
    gtk_label_set_markup ( GTK_LABEL ( GTK_MESSAGE_DIALOG(dialog)->label ), texte );

    response = gtk_dialog_run (GTK_DIALOG (dialog));
    gtk_widget_destroy ( dialog );

    if ( response == GTK_RESPONSE_OK )
	return TRUE;
    else
	return FALSE;
}




/*************************************************************************************************************/
/* fonction demande_texte */
/* affiche une boite de dialogue qui demande l'entrée d'un texte */
/* renvoie NULL si annulé ou le texte */
/*************************************************************************************************************/
gchar *demande_texte ( gchar *titre_fenetre,
		       gchar *question )
{
    GtkWidget *dialog;
    gint resultat;
    GtkWidget *label;
    GtkWidget *entree;
    gchar *retour;

    dialog = gtk_dialog_new_with_buttons ( titre_fenetre,
					   GTK_WINDOW (window),
					   GTK_DIALOG_MODAL,
					   GTK_STOCK_OK,0,
					   GTK_STOCK_CANCEL,1,
					   NULL );

    label = gtk_label_new ( question );
    gtk_box_pack_start ( GTK_BOX ( GTK_DIALOG ( dialog ) -> vbox ),
			 label,
			 FALSE,
			 FALSE,
			 0 );
    gtk_widget_show ( label );

    entree = gtk_entry_new ();
    gtk_entry_set_activates_default ( GTK_ENTRY ( entree ),
				      TRUE );
    gtk_window_set_focus ( GTK_WINDOW ( dialog ),
			   entree );
    gtk_box_pack_start ( GTK_BOX ( GTK_DIALOG ( dialog ) -> vbox ),
			 entree,
			 FALSE,
			 FALSE,
			 0 );
    gtk_widget_show ( entree );

    resultat = gtk_dialog_run ( GTK_DIALOG ( dialog ));

    if ( resultat )
    {
	gtk_widget_destroy ( GTK_WIDGET ( dialog ));
	return ( NULL );
    }

    retour = g_strstrip ( g_strdup ( gtk_entry_get_text ( GTK_ENTRY ( entree ))));

    gtk_widget_destroy ( GTK_WIDGET ( dialog ));
    return ( retour );
}
/*************************************************************************************************************/



/*************************************************************************************************************/
gboolean blocage_boites_dialogues ( GtkWidget *dialog,
				    gpointer null )
{
    gtk_signal_emit_stop_by_name ( GTK_OBJECT ( dialog ),
				   "delete_event" );

    return ( TRUE );
}



/**
 * Concatenate two strings and use pango layout to produce a string
 * that will contain an hint and a text.
 *
 * \param hint Text to use as hint
 * \param text Text to use as secondary text
 *
 * \return a pango formated string with the two arguments concatenated
 */
gchar * make_hint ( gchar * hint, gchar * text )
{
    return g_strconcat ("<span size=\"larger\" weight=\"bold\">",
			hint, "</span>\n\n",
			text, NULL );
}



/**
 * Displays a dialog message stating that bad things will happen
 * soon.  This dialog should not appear theorically, but it is here
 * just in case to spot bugs.
 */
void dialogue_error_brain_damage ()
{
    dialogue_error_hint ( _("Hi, you are in the middle of nowhere, between two lines of code.  Grisbi is expected to crash very soon.  Have a nice day."), 
			  _("Serious brain damage expected.") );
}


/**
 * Displays a dialog message stating that bad things will happen
 * soon.  This dialog should not appear theorically, but it is here
 * just in case to spot bugs.
 */
void dialogue_error_memory ()
{
    dialogue_error_hint ( _("Bad things will happen soon.  Be sure that if to save any modification in a separate file in case they would corrupt files."),
			  _("Cannot allocate memory") );

}



/**
 *
 *
 *
 */
void dialog_message ( gchar * label, ... )
{
    va_list ap;
    int i = 0;

    while ( messages[i] . name )
    {
	if ( !strcmp ( messages[i] . name, label ) )
	{
	    if ( ! messages[i] . hidden )
	    {
		gchar hint_buffer[1024];
		va_start ( ap, label );
		vsnprintf ( hint_buffer, 1024, _(messages[i] . hint), ap );

		dialogue_conditional_hint ( hint_buffer,
					    _(messages[i] . message),
					    messages[i] . name );
	    }
	    return;
	}
	i ++;
    }
}
 

/* Local Variables: */
/* c-basic-offset: 4 */
/* End: */
