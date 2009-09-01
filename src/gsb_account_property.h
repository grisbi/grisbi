#ifndef _COMPTES_GESTION_H
#define _COMPTES_GESTION_H (1)
/* START_INCLUDE_H */
/* END_INCLUDE_H */

struct iso_13616_iban
{
    gchar *locale;
    gchar *iban;
    gint nbre_char;
};

/* START_DECLARATION */
void gsb_account_property_clear_config ( void );
GSList *gsb_account_property_create_combobox_list ( void );
GtkWidget *gsb_account_property_create_page ( void );
void gsb_account_property_fill_page ( void );
void gsb_account_property_set_label_code_banque ( gint bank_number );
void gsb_account_property_set_label_code_bic ( gint bank_number );
/* END_DECLARATION */
#endif
