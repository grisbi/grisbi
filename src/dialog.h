#ifndef _DIALOG_H
#define _DIALOG_H (1)
/* START_INCLUDE_H */
/* END_INCLUDE_H */


/* START_DECLARATION */
void dialog_message ( gchar * label, ... );
void dialogue ( gchar *texte_dialogue );
void dialogue_error ( gchar *text );
void dialogue_error_brain_damage ();
void dialogue_error_hint ( gchar *text, gchar * hint );
void dialogue_error_memory ();
void dialogue_hint ( gchar *text, gchar *hint );
const gchar *dialogue_hint_with_entry ( gchar *text, gchar *hint, gchar *entry_description );
void dialogue_special ( GtkMessageType param, gchar * text );
GtkWidget * dialogue_special_no_run ( GtkMessageType param, GtkButtonsType buttons,
				      gchar * text );
void dialogue_warning ( gchar *text );
void dialogue_warning_hint ( gchar *text, gchar * hint );
gchar * make_hint ( gchar * hint, gchar * text );
gboolean question ( gchar *texte );
gboolean question_conditional_yes_no ( gchar * var );
gboolean question_yes_no ( gchar *texte,
			   gint default_answer );
gboolean question_yes_no_hint ( gchar * hint,
				gchar *texte,
				gint default_answer );
/* END_DECLARATION */

struct conditional_message
{
    gchar * name;
    gchar * hint;
    gchar * message;
    gboolean hidden;
    gboolean default_answer;	/** Useful for yes/no questions. */
};

#endif
