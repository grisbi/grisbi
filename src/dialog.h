#ifndef _DIALOG_H
#define _DIALOG_H (1)
/* START_INCLUDE_H */
#include "dialog.h"
/* END_INCLUDE_H */


/* START_DECLARATION */
gboolean blocage_boites_dialogues ( GtkWidget *dialog,
				    gpointer null );
void dialog_message ( gchar * label, ... );
void dialogue ( gchar *texte_dialogue );
void dialogue_conditional ( gchar *text, gchar * var );
void dialogue_conditional_hint ( gchar *hint, gchar * text, gchar * var );
void dialogue_conditional_info_hint ( gchar *hint, gchar * text, gchar * var );
void dialogue_error ( gchar *text );
void dialogue_error_brain_damage ();
void dialogue_error_hint ( gchar *text, gchar * hint );
void dialogue_error_memory ();
void dialogue_hint ( gchar *text, gchar *hint );
void dialogue_special ( GtkMessageType param, gchar * text );
GtkWidget * dialogue_special_no_run ( GtkMessageType param, GtkButtonsType buttons,
				      gchar * text );
void dialogue_warning ( gchar *text );
void dialogue_warning_hint ( gchar *text, gchar * hint );
gchar * make_hint ( gchar * hint, gchar * text );
gboolean question ( gchar *texte );
gboolean question_yes_no_hint ( gchar * hint, gchar *texte );
/* END_DECLARATION */

struct conditional_message
{
    gchar * name;
    gchar * hint;
    gchar * message;
    gboolean default_state;
    gboolean hidden;
};

#endif
