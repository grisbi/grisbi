#ifndef _EXPORT_H
#define _EXPORT_H (1)

/* START_INCLUDE_H */
/* END_INCLUDE_H */


/** Structure used to hold exported account information.  */
struct exported_account
{
    gint account_nb;        /** Account number, to use with gsb_data_account_get...() functions */
    enum exported_format    /** Export format. */
    {
        EXPORT_QIF,
        EXPORT_CSV,
    } format;
    gchar *extension;       /** qif ou csv */
    gchar *filename;        /** Filename to export into. */
    GtkWidget * chooser;    /** GtkFileChooser responsible of choosing file to export account into. */
};

/* START_DECLARATION */
void export_accounts ( void );
/* END_DECLARATION */

#endif
