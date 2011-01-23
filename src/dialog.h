#ifndef _DIALOG_H
#define _DIALOG_H (1)
/* START_INCLUDE_H */
/* END_INCLUDE_H */


struct conditional_message
{
    gchar * name;
    gchar * hint;
    gchar * message;
    gboolean hidden;
    gboolean default_answer;	/** Useful for yes/no questions. */
};


/* START_DECLARATION */
G_MODULE_EXPORT void dialog_message ( gchar *label, ... );
void dialogue ( gchar *texte_dialogue );
void dialogue_conditional_hint ( gchar *hint, gchar *text, gchar *var );
G_MODULE_EXPORT void dialogue_error ( gchar *text );
void dialogue_error_brain_damage ();
G_MODULE_EXPORT void dialogue_error_hint ( const gchar *text, gchar *hint );
void dialogue_error_memory ();
void dialogue_hint ( gchar *text, gchar *hint );
const gchar *dialogue_hint_with_entry ( gchar *text, gchar *hint, gchar *entry_description );
void dialogue_special ( GtkMessageType param, gchar *text );
GtkWidget *dialogue_special_no_run ( GtkMessageType param,
                        GtkButtonsType buttons,
                        gchar *text );
gboolean dialogue_update_struct_message ( GtkWidget *checkbox,
                        struct conditional_message *message );
G_MODULE_EXPORT void dialogue_warning ( gchar *text );
void dialogue_warning_hint ( gchar *text, gchar *hint );
gchar *make_blue ( gchar *text );
gchar *make_hint ( const gchar *hint, const gchar *text );
gchar *make_pango_attribut ( gchar *attribut, gchar *text );
gchar *make_red ( gchar *text );
gboolean question_conditional_yes_no ( gchar *var );
gint question_conditional_yes_no_get_no_struct ( struct conditional_message *msg,
                        gchar *name );
gboolean question_conditional_yes_no_with_struct ( struct conditional_message *message );
gboolean question_yes_no ( gchar *text, gint default_answer );
gboolean question_yes_no_hint ( gchar *hint,
                        gchar *texte,
                        gint default_answer );
/* END_DECLARATION */
#endif
