#ifndef __GRISBI_UI_H__
#define __GRISBI_UI_H__

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <gtk/gtk.h>

/*START_INCLUDE*/
#include "gsb_debug.h"
#include "export.h"
#include "file_obfuscate.h"
#include "file_obfuscate_qif.h"
#include "grisbi_app.h"
#include "gsb_account.h"
#include "gsb_assistant_account.h"
#include "gsb_assistant_archive.h"
#include "gsb_assistant_archive_export.h"
#include "gsb_file.h"
#include "gsb_scheduler_list.h"
#include "gsb_transactions_list.h"
#include "help.h"
#include "import.h"
#include "menu.h"
#include "parametres.h"
#include "tip.h"
#include "erreur.h"
/*END_INCLUDE*/


/* Entrées de menu "groupables" par bloc */

/* Entrées du menu toujours actives */
static const GtkActionEntry always_sensitive_entries[] =
{
/* name, stock_id, label, accelerator, tooltip, callback */
    /* File menu */
    {"FileMenuAction", NULL, N_("_File"), NULL, NULL, NULL},
#ifdef GTKOSXAPPLICATION
    { "NewAction", GTK_STOCK_NEW, N_("_New account file..."), "<Meta>N", NULL,
     G_CALLBACK ( gsb_file_new ) },
    {"OpenAction",  GTK_STOCK_OPEN, N_("_Open..."), "<Meta>O", NULL,
     G_CALLBACK ( gsb_file_open_menu ) },
#else
    { "NewAction", GTK_STOCK_NEW, N_("_New account file..."), NULL, NULL,
     G_CALLBACK ( gsb_file_new ) },
    {"OpenAction",  GTK_STOCK_OPEN, N_("_Open..."), NULL, NULL,
     G_CALLBACK ( gsb_file_open_menu ) },
#endif
    {"ImportFileAction", GTK_STOCK_CONVERT,  N_("_Import file..."), NULL, NULL,
     G_CALLBACK ( importer_fichier ) },
    {"ObfuscateQifAction", GTK_STOCK_FIND, N_("_Obfuscate QIF file..."), "", NULL,
     G_CALLBACK ( file_obfuscate_qif_run ) },
    {"QuitAction", GTK_STOCK_QUIT, N_("_Quit"), NULL, NULL,
     G_CALLBACK ( grisbi_app_quit ) },

    /* Edit menu */
    {"EditMenuAction", NULL, N_("_Edit"), NULL, NULL, NULL },
    {"PrefsAction", GTK_STOCK_PREFERENCES, N_("_Preferences"), NULL, NULL,
     G_CALLBACK ( gsb_menu_preferences ) },

    /* View menu */
    {"ViewMenuAction", NULL, N_("_View"), NULL, NULL, NULL },

    /* Help menu */
    {"HelpMenuAction", NULL, N_("_Help"), NULL, NULL, NULL },
#ifdef GTKOSXAPPLICATION
    {"ManualAction", GTK_STOCK_HELP, N_("_Manual"), "<Meta>H", NULL,
     G_CALLBACK ( help_manual ) },
#else
    {"ManualAction", GTK_STOCK_HELP, N_("_Manual"), NULL, NULL,
     G_CALLBACK ( help_manual ) },
#endif
    {"QuickStartAction", NULL, N_("_Quick start"), NULL, NULL,
     G_CALLBACK ( help_quick_start ) },
    {"TranslationAction", NULL, N_("_Translation"), NULL, NULL,
     G_CALLBACK ( help_translation ) },
    {"AboutAction", GTK_STOCK_ABOUT, N_("_About Grisbi..."), NULL, NULL,
     G_CALLBACK ( a_propos ) },
    {"GrisbiWebsiteAction", NULL, N_("_Grisbi website"), NULL, NULL,
     G_CALLBACK ( help_website ) },
    {"ReportBugAction", NULL, N_("_Report a bug"), NULL, NULL,
     G_CALLBACK ( help_bugreport ) },
    {"TipAction", GTK_STOCK_DIALOG_INFO, N_("_Tip of the day"), NULL, NULL,
     G_CALLBACK ( force_display_tip ) },
};

static GtkToggleActionEntry show_full_screen_entrie[] =
{
/* Name, stock_id, label, accelerator, tooltip, callback, is_active */
    {"ShowFullScreenAction", NULL, N_("Full screen mode"), NULL, NULL,
     G_CALLBACK ( gsb_menu_full_screen_mode ), 0 },
};


/* Entrées du menu actives avec un fichier chargé */
static const GtkActionEntry file_loading_sensitive_entries[] =
{
/* name, stock_id, label, accelerator, tooltip, callback */
    {"SaveAsAction", GTK_STOCK_SAVE_AS,  N_("_Save as..."), NULL, NULL,
     G_CALLBACK ( gsb_file_save_as ) },
    {"FileRevertAction", GTK_STOCK_REVERT_TO_SAVED, NULL, NULL,
     N_("Revert to a saved version of the file..."),
     G_CALLBACK ( gsb_file_save_as ) },
    {"ExportFileAction", GTK_STOCK_CONVERT, N_("_Export accounts as QIF/CSV file..."), NULL, NULL,
     G_CALLBACK ( export_accounts ) },
    {"CreateArchiveAction", GTK_STOCK_CLEAR, N_("Archive transactions..."), NULL, NULL,
     G_CALLBACK ( gsb_assistant_archive_run_by_menu ) },
    {"ExportArchiveAction", GTK_STOCK_HARDDISK, N_("_Export an archive as GSB/QIF/CSV file..."), NULL, NULL,
     G_CALLBACK ( gsb_assistant_archive_export_run ) },
    {"DebugFileAction", GTK_STOCK_FIND, N_("_Debug account file..."), "", NULL,
     G_CALLBACK ( gsb_debug ) },
    {"ObfuscateAction", GTK_STOCK_FIND, N_("_Obfuscate account file..."), "", NULL,
     G_CALLBACK ( file_obfuscate_run ) },
#ifdef GTKOSXAPPLICATION
    {"CloseAction", GTK_STOCK_CLOSE, NULL, "<Meta>W", NULL,
     G_CALLBACK ( grisbi_app_close_file ) },
#else
    {"CloseAction", GTK_STOCK_CLOSE, NULL, NULL, NULL,
     G_CALLBACK ( grisbi_app_close_file ) },
#endif
    {"NewAccountAction", GTK_STOCK_NEW, N_("_New account"), "", NULL,
     G_CALLBACK ( gsb_assistant_account_run ) },
};

static GtkToggleActionEntry file_debug_toggle_entrie[] =
{
/* Name, stock_id, label, accelerator, tooltip, callback, is_active */
    {"DebugModeAction", NULL, N_("Debug mode"), NULL, NULL,
     G_CALLBACK ( gsb_debug_start_log ), 0 },
};

/* Entrées de menu actives si une transaction (y compris planifiée) est sélectionnée */
static const GtkActionEntry select_all_transactions_sensitive_entries[] =
{
/* name, stock_id, label, accelerator, tooltip, callback */
     {"EditTransactionAction", GTK_STOCK_EDIT, N_("_Edit transaction"), "", NULL,
     G_CALLBACK ( gsb_transactions_list_edit_current_transaction ) },
    {"RemoveTransactionAction", GTK_STOCK_DELETE, N_("_Remove transaction"), "", NULL,
     G_CALLBACK ( remove_transaction ) },
    {"TemplateTransactionAction", GTK_STOCK_COPY, N_("Use selected transaction as a template"), "", NULL,
     G_CALLBACK ( gsb_transactions_list_clone_template ) },
    {"CloneTransactionAction", GTK_STOCK_COPY, N_("_Clone transaction"), "", NULL,
     G_CALLBACK ( clone_selected_transaction ) },
};

/* Entrées de menu actives si une transaction d'un compte est sélectionnée */
static const GtkActionEntry select_transaction_sensitive_entries[] =
{
/* name, stock_id, label, accelerator, tooltip, callback */
    {"ConvertToScheduledAction", GTK_STOCK_CONVERT, N_("Convert to _scheduled transaction"), NULL, NULL,
     G_CALLBACK ( schedule_selected_transaction ) },
    {"MoveToAnotherAccountAction", NULL, N_("_Move transaction to another account"), NULL, NULL, NULL },
};

/* Entrées de menu actives si un compte ou l'onglet planification est sélectionné */
static const GtkActionEntry new_transaction_sensitive_entries[] =
{
/* name, stock_id, label, accelerator, tooltip, callback */
    {"NewTransactionAction", GTK_STOCK_NEW, N_("_New transaction"), "", NULL,
     G_CALLBACK ( new_transaction ) },
};

static GtkToggleActionEntry view_transaction_form_toggle_entries[] =
{
/* Name, stock_id, label, accelerator, tooltip, callback, is_active */
    {"ShowTransactionFormAction", NULL, N_("Show transaction _form"), NULL, NULL,
     G_CALLBACK ( gsb_gui_toggle_show_form ), 0 },
};

/* Entrée de menu active dès qu'on affiche le détail d'un compte */
static const GtkActionEntry select_account_remove_current_sensitive_entries[] =
{
    {"RemoveAccountAction", GTK_STOCK_DELETE, N_("_Remove current account"), "", NULL,
     G_CALLBACK ( gsb_account_delete ) },
};

static GtkRadioActionEntry select_account_radio_entries[] =
{
/* name, stock_id, label, accelerator, tooltip, callback */
    {"ShowOneLineAction", NULL, N_("Show _one line per transaction"), NULL, NULL,
     ONE_LINE_PER_TRANSACTION },
    {"ShowTwoLinesAction", NULL, N_("Show _two lines per transaction"), NULL, NULL,
     TWO_LINES_PER_TRANSACTION },
    {"ShowThreeLinesAction", NULL, N_("Show _three lines per transaction"), NULL, NULL,
     THREE_LINES_PER_TRANSACTION },
    {"ShowFourLinesAction", NULL, N_("Show _four lines per transaction"), NULL, NULL,
     FOUR_LINES_PER_TRANSACTION },
};

static GtkToggleActionEntry select_account_toggle_entries[] =
{
/* Name, stock_id, label, accelerator, tooltip, callback, is_active */
#ifdef GTKOSXAPPLICATION
    {"ShowReconciledAction", NULL, N_("Show _reconciled"), "<Meta>R", NULL,
     G_CALLBACK ( gsb_gui_toggle_show_reconciled ), 0 },
    {"ShowArchivedAction", NULL, N_("Show _lines archives"), "<Meta>L", NULL,
     G_CALLBACK ( gsb_gui_toggle_show_archived ), 0 },
#else
    {"ShowReconciledAction", NULL, N_("Show _reconciled"), "<Alt>R", NULL,
     G_CALLBACK ( gsb_gui_toggle_show_reconciled ), 0 },
    {"ShowArchivedAction", NULL, N_("Show _lines archives"), "<Alt>L", NULL,
     G_CALLBACK ( gsb_gui_toggle_show_archived ), 0 },
#endif
};





/* entrées individuelles du menu */
 /* Entrée du menu pour la liste des fichiers si nécessaire */
static const GtkActionEntry file_recent_files_entrie[] =
{
/* name, stock_id, label, accelerator, tooltip, callback */
    {"RecentFilesAction", NULL, N_("_Recently opened files"), NULL, NULL, NULL },
};

/* Entrée du menu active dès qu'une modification du fichier est faite */
static const GtkActionEntry file_save_entrie[] =
{
/* name, stock_id, label, accelerator, tooltip, callback */
#ifdef GTKOSXAPPLICATION
    {"SaveAction", GTK_STOCK_SAVE, N_("_Save"), "<Meta>S", NULL,
     G_CALLBACK ( gsb_file_save ) },
#else
    {"SaveAction", GTK_STOCK_SAVE, N_("_Save"), NULL, NULL,
     G_CALLBACK ( gsb_file_save ) },
#endif
};

/* Entrée de menu existante si il existe au moins un compte clos */
static GtkToggleActionEntry show_closed_toggle_entries[] =
{
/* Name, stock_id, label, accelerator, tooltip, callback, is_active */
    {"ShowClosedAction", NULL, N_("Show _closed accounts"), NULL, NULL,
     G_CALLBACK ( gsb_gui_toggle_show_closed_accounts ),0 }
};

 /* Entrée du menu réinitialiser la largeur des colonnes */
static const GtkActionEntry init_width_col_entrie[] =
{
/* name, stock_id, label, accelerator, tooltip, callback */
    {"InitWidthColAction", NULL, N_("Reset the column width"), NULL, NULL,
     G_CALLBACK ( gsb_menu_reinit_largeur_col_menu ) },
};



#endif  /* __GRISBI_UI_H__  */
