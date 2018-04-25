#ifndef _EXPORT_H
#define _EXPORT_H (1)

#include <gtk/gtk.h>

/* START_INCLUDE_H */
/* END_INCLUDE_H */


/** Structure used to hold exported account information.  */
struct ExportedAccount
{
    gint 		account_nb;        	/** Account number, to use with gsb_data_account_get...() functions */
    enum 		ExportedFormat		/** Export format. */
    {
        EXPORT_QIF,
        EXPORT_CSV,
    } format;
    gchar *		extension;			/** qif ou csv */
    gchar *		filename;			/** Filename to export into. */
    GtkWidget * chooser;			/** GtkFileChooser responsible of choosing file to export account into. */
};

/* START_DECLARATION */
void 	export_accounts 	(void);
gchar *	export_get_csv_separator (void);
void 	export_set_csv_separator (gchar *separator);
/* END_DECLARATION */

#endif
