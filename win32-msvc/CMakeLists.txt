CMAKE_MINIMUM_REQUIRED(VERSION 2.6)
if(POLICY CMP0015)
   # policies not known to CMake 2.6 ...
   cmake_policy(SET CMP0015 NEW)
endif(POLICY CMP0015)

PROJECT (Grisbi C)

# MS Visual Studio 10 Macros
# $(Platform)      The name of current project platform (for example, "Win32").
# $(Configuration) The name of the current project configuration (for example, "Debug").
# $(TargetDir)     The directory of the primary output file for the build (defined as drive + path); includes the trailing backslash '\'.
# see http://msdn.microsoft.com/en-us/library/c02as0cs%28v=VS.100%29.aspx for more macros

SET(CMAKE_RUNTIME_OUTPUT_DIRECTORY target\\$(Platform))
SET(CMAKE_LIBRARY_OUTPUT_DIRECTORY target\\$(Platform)\\$(Configuration)\\plugins)
SET(package_name grisbi)

ADD_DEFINITIONS(-D_CRT_SECURE_NO_WARNINGS)
ADD_DEFINITIONS(-D__PRETTY_FUNCTION__="")
ADD_DEFINITIONS(-D_WIN32)

# Define to 1 if translation of program messages to the user's native language is requested.
ADD_DEFINITIONS(-DENABLE_NLS=1)

# Define if you have the iconv() function.
ADD_DEFINITIONS(-DHAVE_ICONV=1)

# Define if you have <langinfo.h> and nl_langinfo(CODESET).
ADD_DEFINITIONS(-DHAVE_LANGINFO_CODESET=1)

# Define if your <locale.h> file defines LC_MESSAGES.
ADD_DEFINITIONS(-DHAVE_LC_MESSAGES=1)

# Define to 1 if you have the `intl' library (-lintl).
ADD_DEFINITIONS(-DHAVE_LIBINTL=1)

# Define to 1 if you have the <limits.h> header file.
ADD_DEFINITIONS(-DHAVE_LIMITS_H=1)

# Define to 1 if you have the <locale.h> header file.
ADD_DEFINITIONS(-DHAVE_LOCALE_H=1)

# Define to 1 if you have the <malloc.h> header file.
ADD_DEFINITIONS(-DHAVE_MALLOC_H=1)

# Define to 1 if you have the <memory.h> header file.
ADD_DEFINITIONS(-DHAVE_MEMORY_H=1)

# Define to 1 if you have the `mempcpy' function.
ADD_DEFINITIONS(-DHAVE_MEMPCPY=1)

# Define to 1 to have grisbi plugins management system activated
ADD_DEFINITIONS(-DHAVE_PLUGINS=1)

# Define to 1 if you have the `setlocale' function.
ADD_DEFINITIONS(-DHAVE_SETLOCALE=1)

# Define to 1 if you have the <string.h> header file.
ADD_DEFINITIONS(-DHAVE_STRING_H=1)

# Define to 1 to have OFX support disabled
#ADD_DEFINITIONS(-DNOOFX=1)
# Define to 1 if you want to have OFX support using libofx 0.7.0
ADD_DEFINITIONS(-DOFX_0_7=1)

# Define to 1 when you do not want to use SSL as crypt algorithm (NOT RECOMMANDED)
#ADD_DEFINITIONS(-DNOSSL=1)

INCLUDE_DIRECTORIES(
	target/$(Platform)/gtk-dev/include/atk-1.0
	target/$(Platform)/gtk-dev/include/cairo
	target/$(Platform)/gtk-dev/include/glib-2.0
	target/$(Platform)/gtk-dev/include/gdk-pixbuf-2.0
	target/$(Platform)/gtk-dev/include/gtk-2.0
	target/$(Platform)/gtk-dev/include/pango-1.0
	target/$(Platform)/gtk-dev/include
	target/$(Platform)/gtk-dev/lib/glib-2.0/include
	target/$(Platform)/gtk-dev/lib/gtk-2.0/include
	target/$(Platform)/plugins-dev/libofx/include
	target/$(Platform)/plugins-dev/libxml2/include
	target/$(Platform)/plugins-dev/iconv/include
	target/$(Platform)/plugins-dev/openssl/include
	../win32-msvc/src
	../win32-msvc
	../win32
	../src
	../src/plugins/gnucash
	../src/plugins/ofx
	../src/plugins/openssl
)

LINK_DIRECTORIES(
	target/$(Platform)/gtk-dev/lib
	target/$(Platform)/plugins-dev/libofx/lib
	target/$(Platform)/plugins-dev/libxml2/lib
	target/$(Platform)/plugins-dev/iconv/lib
	target/$(Platform)/plugins-dev/openssl/lib
	target/$(Platform)/plugins-dev/openssl/lib/VC
	$(TargetDir)
)

SET (grisbi_source
	../src/accueil.c
	../src/accueil.h
	../src/affichage.c
	../src/affichage.h
	../src/affichage_liste.c
	../src/affichage_liste.h
	../src/barre_outils.c
	../src/barre_outils.h
	../src/bet_config.c
	../src/bet_config.h
	../src/bet_data.c
	../src/bet_data.h
	../src/bet_data_finance.c
	../src/bet_data_finance.h
	../src/bet_finance_ui.c
	../src/bet_finance_ui.h
	../src/bet_future.c
	../src/bet_future.h
	../src/bet_hist.c
	../src/bet_hist.h
	../src/bet_tab.c
	../src/bet_tab.h
	../src/categories_onglet.c
	../src/categories_onglet.h
	../src/classement_echeances.c
	../src/classement_echeances.h
	../src/csv_parse.c
	../src/csv_parse.h
	../src/custom_list.c
	../src/custom_list.h
	../src/dialog.c
	../src/dialog.h
	../src/erreur.c
	../src/erreur.h
	../src/etats.h
	../src/etats_affiche.c
	../src/etats_affiche.h
	../src/etats_calculs.c
	../src/etats_calculs.h
	../src/etats_config.c
	../src/etats_config.h
	../src/etats_csv.c
	../src/etats_csv.h
	../src/etats_gtktable.c
	../src/etats_gtktable.h
	../src/etats_html.c
	../src/etats_html.h
	../src/etats_latex.c
	../src/etats_latex.h
	../src/etats_onglet.c
	../src/etats_onglet.h
	../src/etats_support.c
	../src/etats_support.h
	../src/export.c
	../src/export.h
	../src/export_csv.c
	../src/export_csv.h
	../src/fenetre_principale.c
	../src/fenetre_principale.h
	../src/file_obfuscate.c
	../src/file_obfuscate.h
	../src/file_obfuscate_qif.c
	../src/file_obfuscate_qif.h
	../src/go-charmap-sel.c
	../src/go-charmap-sel.h
	../src/go-optionmenu.c
	../src/go-optionmenu.h
	../src/gsb_account.c
	../src/gsb_account.h
	../src/gsb_account_property.c
	../src/gsb_account_property.h
	../src/gsb_archive_config.c
	../src/gsb_archive_config.h
	../src/gsb_assistant.c
	../src/gsb_assistant.h
	../src/gsb_assistant_account.c
	../src/gsb_assistant_account.h
	../src/gsb_assistant_archive.c
	../src/gsb_assistant_archive.h
	../src/gsb_assistant_archive_export.c
	../src/gsb_assistant_archive_export.h
	../src/gsb_assistant_file.c
	../src/gsb_assistant_file.h
	../src/gsb_assistant_first.c
	../src/gsb_assistant_first.h
	../src/gsb_assistant_reconcile_config.c
	../src/gsb_assistant_reconcile_config.h
	../src/gsb_autofunc.c
	../src/gsb_autofunc.h
	../src/gsb_automem.c
	../src/gsb_automem.h
	../src/gsb_bank.c
	../src/gsb_bank.h
	../src/gsb_calendar.c
	../src/gsb_calendar.h
	../src/gsb_calendar_entry.c
	../src/gsb_calendar_entry.h
	../src/gsb_category.c
	../src/gsb_category.h
	../src/gsb_combo_box.c
	../src/gsb_combo_box.h
	../src/gsb_currency.c
	../src/gsb_currency.h
	../src/gsb_currency_config.c
	../src/gsb_currency_config.h
	../src/gsb_currency_link_config.c
	../src/gsb_currency_link_config.h
	../src/gsb_data_account.c
	../src/gsb_data_account.h
	../src/gsb_data_archive.c
	../src/gsb_data_archive.h
	../src/gsb_data_archive_store.c
	../src/gsb_data_archive_store.h
	../src/gsb_data_bank.c
	../src/gsb_data_bank.h
	../src/gsb_data_budget.c
	../src/gsb_data_budget.h
	../src/gsb_data_category.c
	../src/gsb_data_category.h
	../src/gsb_data_currency.c
	../src/gsb_data_currency.h
	../src/gsb_data_currency_link.c
	../src/gsb_data_currency_link.h
	../src/gsb_data_form.c
	../src/gsb_data_form.h
	../src/gsb_data_fyear.c
	../src/gsb_data_fyear.h
	../src/gsb_data_import_rule.c
	../src/gsb_data_import_rule.h
	../src/gsb_data_mix.c
	../src/gsb_data_mix.h
	../src/gsb_data_partial_balance.c
	../src/gsb_data_partial_balance.h
	../src/gsb_data_payee.c
	../src/gsb_data_payee.h
	../src/gsb_data_payment.c
	../src/gsb_data_payment.h
	../src/gsb_data_print_config.c
	../src/gsb_data_print_config.h
	../src/gsb_data_reconcile.c
	../src/gsb_data_reconcile.h
	../src/gsb_data_report.c
	../src/gsb_data_report.h
	../src/gsb_data_report_amout_comparison.c
	../src/gsb_data_report_amout_comparison.h
	../src/gsb_data_report_text_comparison.c
	../src/gsb_data_report_text_comparison.h
	../src/gsb_data_scheduled.c
	../src/gsb_data_scheduled.h
	../src/gsb_data_transaction.c
	../src/gsb_data_transaction.h
	../src/gsb_debug.c
	../src/gsb_debug.h
	../src/gsb_file.c
	../src/gsb_file.h
	../src/gsb_file_config.c
	../src/gsb_file_config.h
	../src/gsb_file_load.c
	../src/gsb_file_load.h
	../src/gsb_file_others.c
	../src/gsb_file_others.h
	../src/gsb_file_save.c
	../src/gsb_file_save.h
	../src/gsb_file_util.c
	../src/gsb_file_util.h
	../src/gsb_form.c
	../src/gsb_form.h
	../src/gsb_form_config.c
	../src/gsb_form_config.h
	../src/gsb_form_scheduler.c
	../src/gsb_form_scheduler.h
	../src/gsb_form_transaction.c
	../src/gsb_form_transaction.h
	../src/gsb_form_widget.c
	../src/gsb_form_widget.h
	../src/gsb_fyear.c
	../src/gsb_fyear.h
	../src/gsb_fyear_config.c
	../src/gsb_fyear_config.h
	../src/gsb_payment_method.c
	../src/gsb_payment_method.h
	../src/gsb_payment_method_config.c
	../src/gsb_payment_method_config.h
	../src/gsb_plugins.c
	../src/gsb_plugins.h
	../src/gsb_real.c
	../src/gsb_real.h
	../src/gsb_reconcile.c
	../src/gsb_reconcile.h
	../src/gsb_reconcile_config.c
	../src/gsb_reconcile_config.h
	../src/gsb_reconcile_list.c
	../src/gsb_reconcile_list.h
	../src/gsb_reconcile_sort_config.c
	../src/gsb_reconcile_sort_config.h
	../src/gsb_report.c
	../src/gsb_report.h
	../src/gsb_scheduler.c
	../src/gsb_scheduler.h
	../src/gsb_scheduler_list.c
	../src/gsb_scheduler_list.h
	../src/gsb_select_icon.c
	../src/gsb_select_icon.h
	../src/gsb_status.c
	../src/gsb_status.h
	../src/gsb_transactions_list.c
	../src/gsb_transactions_list.h
	../src/gsb_transactions_list_sort.c
	../src/gsb_transactions_list_sort.h
	../src/gtk_combofix.c
	../src/gtk_combofix.h
	../src/help.c
	../src/help.h
	../src/import.c
	../src/import.h
	../src/import_csv.c
	../src/import_csv.h
	../src/imputation_budgetaire.c
	../src/imputation_budgetaire.h
	../src/include.h
	../src/main.c
	../src/main.h
#	../src/main_cunit.c
#	../src/main_cunit.h
	../src/menu.c
	../src/menu.h
	../src/meta_budgetary.c
	../src/meta_budgetary.h
	../src/meta_categories.c
	../src/meta_categories.h
	../src/meta_payee.c
	../src/meta_payee.h
	../src/metatree.c
	../src/metatree.h
	../src/navigation.c
	../src/navigation.h
	../src/parametres.c
	../src/parametres.h
	../src/parse_cmdline.c
	../src/parse_cmdline.h
	../src/print_config.c
	../src/print_config.h
	../src/print_dialog_config.c
	../src/print_dialog_config.h
	../src/print_report.c
	../src/print_report.h
	../src/print_transactions_list.c
	../src/print_transactions_list.h
	../src/print_tree_view_list.c
	../src/print_tree_view_list.h
	../src/qif.c
	../src/qif.h
	../src/tiers_onglet.c
	../src/tiers_onglet.h
	../src/tip.c
	../src/tip.h
	../src/traitement_variables.c
	../src/traitement_variables.h
	../src/transaction_list.c
	../src/transaction_list.h
	../src/transaction_list_select.c
	../src/transaction_list_select.h
	../src/transaction_list_sort.c
	../src/transaction_list_sort.h
	../src/transaction_model.c
	../src/transaction_model.h
	../src/utils.c
	../src/utils.h
	../src/utils_buttons.c
	../src/utils_buttons.h
	../src/utils_dates.c
	../src/utils_dates.h
	../src/utils_editables.c
	../src/utils_editables.h
	../src/utils_files.c
	../src/utils_files.h
	../src/utils_file_selection.c
	../src/utils_file_selection.h
	../src/utils_font.c
	../src/utils_font.h
	../src/utils_operations.c
	../src/utils_operations.h
	../src/utils_str.c
	../src/utils_str.h
	../win32-msvc/src/getopt.c
	../win32-msvc/src/getopt.h
	../win32-msvc/src/getopt1.c
	../win32-msvc/src/realpath.c
	../win32-msvc/src/realpath.h
	../win32-msvc/src/resource.h
	../win32-msvc/src/grisbi.rc
	../win32-msvc/config.h
	../win32/win32gtkutils.c
	../win32/win32gtkutils.h
	../win32/win32utils.c
	../win32/win32utils.h
	# ../win32/grisbi.ico
)

ADD_EXECUTABLE(grisbi
	WIN32
	${grisbi_source}
)

TARGET_LINK_LIBRARIES( grisbi
	glib-2.0
	intl
	cairo
	gdk_pixbuf-2.0
	gdk-win32-2.0
	gio-2.0
	gmodule-2.0
	gobject-2.0
	gtk-win32-2.0
	pango-1.0
	pangocairo-1.0
	zdll
)

set_target_properties ( grisbi
	PROPERTIES ENABLE_EXPORTS 1
)

ADD_DEPENDENCIES (grisbi prepare_env prepare_mo)

ADD_LIBRARY ( ofx
	SHARED
	../src/plugins/ofx/ofx.c
	../src/plugins/ofx/ofx.h
)

ADD_CUSTOM_COMMAND ( TARGET ofx
	POST_BUILD
#	COMMAND XCOPY /D /Y $(Configuration)\\ofx.dll target\\$(Platform)\\$(Configuration)\\plugins\\
	COMMAND XCOPY /D /Y target\\$(Platform)\\plugins-dev\\libofx\\bin\\* target\\$(Platform)\\$(Configuration)
	COMMAND IF NOT EXIST target\\$(Platform)\\$(Configuration)\\dtd MKDIR target\\$(Platform)\\$(Configuration)\\dtd
	COMMAND XCOPY /D /Y target\\$(Platform)\\plugins-dev\\libofx\\bin\\dtd\\o*.d* target\\$(Platform)\\$(Configuration)\\dtd
)

ADD_LIBRARY ( gnucash
	SHARED
	../src/plugins/gnucash/gnucash.c
	../src/plugins/gnucash/gnucash.h
)

ADD_CUSTOM_COMMAND ( TARGET gnucash
	POST_BUILD
	COMMAND XCOPY /D /Y $(Configuration)\\gnucash.dll target\\$(Platform)\\$(Configuration)\\plugins\\
	COMMAND XCOPY /D /Y target\\$(Platform)\\plugins-dev\\libxml2\\bin\\libxml2.dll target\\$(Platform)\\$(Configuration)\\
	COMMAND XCOPY /D /Y target\\$(Platform)\\plugins-dev\\iconv\\bin\\iconv.dll target\\$(Platform)\\$(Configuration)\\
)

ADD_LIBRARY ( openssl
	SHARED
	../src/plugins/openssl/openssl.c
	../src/plugins/openssl/openssl.h
)

ADD_CUSTOM_COMMAND ( TARGET openssl
	POST_BUILD
#	COMMAND XCOPY /D /Y $(Configuration)\\openssl.dll target\\$(Platform)\\$(Configuration)\\plugins\\
	COMMAND XCOPY /D /Y target\\$(Platform)\\plugins-dev\\openssl\\bin\\libeay32.dll target\\$(Platform)\\$(Configuration)
)


ADD_DEPENDENCIES (ofx grisbi)
TARGET_LINK_LIBRARIES (ofx
	glib-2.0
	intl
	grisbi
	libofx
)
ADD_DEPENDENCIES (gnucash grisbi)
TARGET_LINK_LIBRARIES (gnucash
	glib-2.0
	intl
	grisbi
	libxml2
	iconv
)
ADD_DEPENDENCIES (openssl grisbi)
TARGET_LINK_LIBRARIES (openssl
	glib-2.0
	intl
	grisbi
	gobject-2.0
	gtk-win32-2.0
	libeay32
	ssleay32
)

ADD_CUSTOM_TARGET (prepare_env)

ADD_CUSTOM_COMMAND(TARGET prepare_env
	POST_BUILD
	COMMAND IF NOT EXIST target\\$(Platform)\\$(Configuration) mkdir target\\$(Platform)\\$(Configuration)
)


# ADD_CUSTOM_COMMAND(TARGET prepare_env
	# POST_BUILD
	# COMMAND xcopy /Y /C /D target\\gtk-dev\\bin\\*.dll target\\Debug
	# COMMAND xcopy /Y /C /D target\\gtk-dev\\bin\\*.dll target\\Release
# )

# ADD_CUSTOM_COMMAND(TARGET prepare_env
	# POST_BUILD
	# COMMAND xcopy /S /Y /D target\\gtk-dev\\etc target\\Debug\\etc\\
	# COMMAND xcopy /S /Y /D target\\gtk-dev\\etc target\\Release\\etc\\
# )

# ADD_CUSTOM_COMMAND(TARGET prepare_env
	# POST_BUILD
	# COMMAND xcopy /S /Y /D target\\gtk-dev\\lib\\gtk-2.0\\2.10.0 target\\Debug\\lib\\gtk-2.0\\2.10.0\\
	# COMMAND xcopy /S /Y /D target\\gtk-dev\\lib\\gtk-2.0\\2.10.0 target\\Release\\lib\\gtk-2.0\\2.10.0\\
# )

ADD_CUSTOM_COMMAND(TARGET prepare_env
	POST_BUILD
	COMMAND xcopy /S /Y /D ..\\pixmaps\\*.png target\\$(Platform)\\$(Configuration)\\pixmaps\\
	# COMMAND xcopy /S /Y /D ..\\pixmaps\\*.png target\\Release\\pixmaps\\
)

ADD_CUSTOM_COMMAND(TARGET prepare_env
	POST_BUILD
	COMMAND xcopy /S /Y /D ..\\help target\\$(Platform)\\$(Configuration)\\help\\
	# COMMAND xcopy /S /Y /D ..\\help target\\Release\\help\\
)

ADD_CUSTOM_COMMAND(TARGET prepare_env
	POST_BUILD
	COMMAND xcopy /S /Y /D ..\\share\\categories target\\$(Platform)\\$(Configuration)\\help\\tips\\categories\\
	# COMMAND xcopy /S /Y /D ..\\share\\categories target\\Release\\help\\tips\\categories\\
)

#ADD_CUSTOM_COMMAND(TARGET prepare_env
#	POST_BUILD
#	COMMAND tools\\dos2unix.cmd ..\\help\\tips\\tips.txt target\\$(Platform)\\$(Configuration)\\help\\tips\\tips#.txt
	# COMMAND tools\\dos2unix.cmd ..\\help\\tips\\tips.txt target\\Release\\help\\tips\\tips.txt
#)

set(
  po_files
  ../po/cs.po
  ../po/da.po
  ../po/de.po
  ../po/el.po
  ../po/eo.po
  ../po/es.po
  ../po/fa.po
  ../po/fr.po
  ../po/he.po
  ../po/it.po
  ../po/lv.po
  ../po/nl.po
  ../po/pl.po
  ../po/pt_BR.po
  ../po/ro.po
  ../po/ru.po
  ../po/zh_CN.po
)

# set (
  # tips_po_files
  # ../help/tips/de.po
  # ../help/tips/es.po
  # ../help/tips/fr.po
# )

set (msgfmt target\\Win32\\gettext\\bin\\msgfmt.exe)

ADD_CUSTOM_TARGET(prepare_mo
	COMMAND IF NOT EXIST target\\lib mkdir target\\lib
	COMMAND IF NOT EXIST target\\lib\\locale mkdir target\\lib\\locale
)

foreach (current_po ${po_files})
  get_filename_component(
    lang
    ${current_po}
    NAME_WE
  )

  set(mo_file target\\lib\\locale\\${lang}\\LC_MESSAGES\\${package_name}.mo)

  add_custom_command(TARGET prepare_mo
    POST_BUILD
	COMMAND IF NOT EXIST target\\lib\\locale\\${lang} mkdir target\\lib\\locale\\${lang}
	COMMAND IF NOT EXIST target\\lib\\locale\\${lang}\\LC_MESSAGES mkdir target\\lib\\locale\\${lang}\\LC_MESSAGES
  )

  add_custom_command(TARGET prepare_mo
	POST_BUILD
    # OUTPUT ${mo_file}
    COMMAND ${msgfmt} -c ${current_po} -o ${mo_file}
    DEPENDS ${current_po}
  )
 endforeach (current_po)

 # foreach (current_po ${tips_po_files})
  # get_filename_component(
    # lang
    # ${current_po}
    # NAME_WE
  # )

  # set(mo_file target\\lib\\locale\\${lang}\\LC_MESSAGES\\${package_name}-tips.mo)

  # add_custom_command(TARGET prepare_mo
    # POST_BUILD
	# COMMAND IF NOT EXIST target\\lib\\locale\\${lang} mkdir target\\lib\\locale\\${lang}
	# COMMAND IF NOT EXIST target\\lib\\locale\\${lang}\\LC_MESSAGES mkdir target\\lib\\locale\\${lang}\\LC_MESSAGES
  # )

  # add_custom_command(TARGET prepare_mo
	# POST_BUILD
  ##  OUTPUT ${mo_file}
    # COMMAND ${msgfmt} -c ${current_po} -o ${mo_file}
    # DEPENDS ${current_po}
  # )
 # endforeach (current_po)

 add_custom_command(TARGET prepare_mo
	POST_BUILD
	COMMAND IF NOT EXIST target\\$(Platform)\\$(Configuration)\\lib mkdir target\\$(Platform)\\$(Configuration)\\lib
	COMMAND xcopy /S /Y /I /D target\\lib\\locale target\\$(Platform)\\$(Configuration)\\lib\\locale
	# COMMAND IF NOT EXIST target\\Release\\lib mkdir target\\Release\\lib
	# COMMAND xcopy /S /Y /I /D target\\lib\\locale target\\Release\\lib\\locale
)