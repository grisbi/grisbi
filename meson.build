# for ninja:
# meson setup builddir
# cd builddir ; meson compile

# regenerate configuration (run from builddir/)
# meson setup --reconfigure --wipe

project('grisbi', 'c',
  version : '3.90.0')

# for config.h
conf_data = configuration_data({
  'VERSION' : '"' + meson.project_version() + '"',
  'PACKAGE' : '"grisbi"',
  'HAVE_GOFFICE' : true,
  'HAVE_OFX' : true,
  'HAVE_SSL' : true,
  'HAVE_XML2' : true,
  'EARLY_DEBUG' : get_option('early-debug'),
  })
localdir = join_paths(get_option('prefix'), get_option('datadir'))
pixmaps_dir = join_paths(localdir, 'pixmaps', 'grisbi')
conf_data.set_quoted('LOCALEDIR', join_paths(localdir, 'locale'))
conf_data.set_quoted('PIXMAPS_DIR', pixmaps_dir)
conf_data.set_quoted('DATA_PATH', join_paths(localdir, 'grisbi'))
conf_data.set_quoted('HELP_PATH', join_paths(get_option('prefix'), 'doc', 'grisbi'))
conf_data.set_quoted('UI_DIR', join_paths(localdir, 'grisbi', 'ui'))

minor_version = meson.project_version().split('.')[1].to_int()
if get_option('debug') or minor_version % 2 == 1
  conf_data.set('DEBUG', true)
  git = find_program('git', required : false)
  if git.found()
    r = run_command(git, 'rev-parse', '--short', 'HEAD', check : false)
    conf_data.set_quoted('GIT_HASH', r.stdout().strip())
  else
    conf_data.set_quoted('GIT_HASH', 'unset')
  endif
endif

# dependencies
goffice_dep = dependency('libgoffice-0.10')
gtk_dep = dependency('gtk+-3.0')
libofx_dep = dependency('libofx')
libxml2_dep = dependency('libxml-2.0')
openssl_dep = dependency('openssl')
zlib_dep = dependency('zlib')

grisbi_dep = [goffice_dep, gtk_dep, libofx_dep, libxml2_dep, openssl_dep, zlib_dep]

incdir = include_directories(['src', 'src/widgets', 'src/etats', 'src/prefs'])

subdir('po')

# tests for functions
compiler = meson.get_compiler('c')
if compiler.has_function('backtrace')
  conf_data.set('HAVE_BACKTRACE', true)
endif

# Ubuntu 22.04 provides an old libglib without g_sort_array()
if compiler.has_function('g_sort_array',
  dependencies : gtk_dep)
  conf_data.set('HAVE_G_SORT_ARRAY', true)
endif

gnome = import('gnome')
grisbi_resources = gnome.compile_resources(
    'grisbi_resources',
    'src/grisbi.gresource.xml',
    source_dir : 'src',
    c_name : 'grisbi'
)

grisbi_src = [
  'src/accueil.c',
  'src/bet_data.c',
  'src/bet_data_finance.c',
  'src/bet_finance_ui.c',
  'src/bet_future.c',
  'src/bet_graph.c',
  'src/bet_hist.c',
  'src/bet_tab.c',
  'src/categories_onglet.c',
  'src/classement_echeances.c',
  'src/csv_parse.c',
  'src/csv_template_rule.c',
  'src/custom_list.c',
  'src/dialog.c',
  'src/erreur.c',
  'src/etats/etats_page_accounts.c',
  'src/etats/etats_page_amount.c',
  'src/etats/etats_page_budget.c',
  'src/etats/etats_page_category.c',
  'src/etats/etats_page_payee.c',
  'src/etats/etats_page_period.c',
  'src/etats/etats_page_text.c',
  'src/etats/etats_page_transfer.c',
  'src/etats_affiche.c',
  'src/etats_calculs.c',
  'src/etats_config.c',
  'src/etats_csv.c',
  'src/etats_gtktable.c',
  'src/etats_html.c',
  'src/etats_onglet.c',
  'src/etats_prefs.c',
  'src/etats_support.c',
  'src/export.c',
  'src/export_csv.c',
  'src/file_obfuscate.c',
  'src/file_obfuscate_qif.c',
  'src/go-charmap-sel.c',
  'src/go-optionmenu.c',
  'src/grisbi_app.c',
  'src/grisbi_conf.c',
  'src/grisbi_prefs.c',
  'src/grisbi_win.c',
  'src/gsb_account.c',
  'src/gsb_assistant.c',
  'src/gsb_assistant_account.c',
  'src/gsb_assistant_archive.c',
  'src/gsb_assistant_archive_export.c',
  'src/gsb_assistant_file.c',
  'src/gsb_assistant_first.c',
  'src/gsb_assistant_reconcile_config.c',
  'src/gsb_autofunc.c',
  'src/gsb_automem.c',
  'src/gsb_bank.c',
  'src/gsb_calendar.c',
  'src/gsb_calendar_entry.c',
  'src/gsb_category.c',
  'src/gsb_combo_box.c',
  'src/gsb_currency.c',
  'src/gsb_currency_popup.c',
  'src/gsb_data_account.c',
  'src/gsb_data_archive.c',
  'src/gsb_data_archive_store.c',
  'src/gsb_data_bank.c',
  'src/gsb_data_budget.c',
  'src/gsb_data_category.c',
  'src/gsb_data_currency.c',
  'src/gsb_data_currency_link.c',
  'src/gsb_data_form.c',
  'src/gsb_data_fyear.c',
  'src/gsb_data_import_rule.c',
  'src/gsb_data_mix.c',
  'src/gsb_data_partial_balance.c',
  'src/gsb_data_payee.c',
  'src/gsb_data_payment.c',
  'src/gsb_data_print_config.c',
  'src/gsb_data_reconcile.c',
  'src/gsb_data_report.c',
  'src/gsb_data_report_amout_comparison.c',
  'src/gsb_data_report_text_comparison.c',
  'src/gsb_data_scheduled.c',
  'src/gsb_data_transaction.c',
  'src/gsb_debug.c',
  'src/gsb_dirs.c',
  'src/gsb_file.c',
  'src/gsb_file_load.c',
  'src/gsb_file_others.c',
  'src/gsb_file_save.c',
  'src/gsb_file_util.c',
  'src/gsb_form.c',
  'src/gsb_form_scheduler.c',
  'src/gsb_form_transaction.c',
  'src/gsb_form_widget.c',
  'src/gsb_fyear.c',
  'src/gsb_locale.c',
  'src/gsb_payment_method.c',
  'src/gsb_real.c',
  'src/gsb_reconcile.c',
  'src/gsb_reconcile_list.c',
  'src/gsb_regex.c',
  'src/gsb_report.c',
  'src/gsb_rgba.c',
  'src/gsb_scheduler.c',
  'src/gsb_scheduler_list.c',
  'src/gsb_select_icon.c',
  'src/gsb_transactions_list.c',
  'src/gsb_transactions_list_sort.c',
  'src/gtk_combofix.c',
  'src/help.c',
  'src/import.c',
  'src/import_csv.c',
  'src/imputation_budgetaire.c',
  'src/main.c',
  'src/menu.c',
  'src/meta_budgetary.c',
  'src/meta_categories.c',
  'src/meta_payee.c',
  'src/metatree.c',
  'src/navigation.c',
  'src/plugins/gnucash/gnucash.c',
  'src/plugins/ofx/ofx.c',
  'src/plugins/openssl/openssl.c',
  'src/prefs/prefs_page_accueil.c',
  'src/prefs/prefs_page_archives.c',
  'src/prefs/prefs_page_bank.c',
  'src/prefs/prefs_page_bet_account.c',
  'src/prefs/prefs_page_bet_general.c',
  'src/prefs/prefs_page_currency.c',
  'src/prefs/prefs_page_currency_link.c',
  'src/prefs/prefs_page_display_adr.c',
  'src/prefs/prefs_page_display_fonts.c',
  'src/prefs/prefs_page_display_form.c',
  'src/prefs/prefs_page_display_gui.c',
  'src/prefs/prefs_page_display_ope.c',
  'src/prefs/prefs_page_divers.c',
  'src/prefs/prefs_page_files.c',
  'src/prefs/prefs_page_form_completion.c',
  'src/prefs/prefs_page_form_options.c',
  'src/prefs/prefs_page_fyear.c',
  'src/prefs/prefs_page_import.c',
  'src/prefs/prefs_page_metatree.c',
  'src/prefs/prefs_page_msg_delete.c',
  'src/prefs/prefs_page_msg_warning.c',
  'src/prefs/prefs_page_options_ope.c',
  'src/prefs/prefs_page_payment_method.c',
  'src/prefs/prefs_page_reconcile.c',
  'src/prefs/prefs_page_reconcile_sort.c',
  'src/print_dialog_config.c',
  'src/print_report.c',
  'src/print_transactions_list.c',
  'src/print_tree_view_list.c',
  'src/qif.c',
  'src/tiers_onglet.c',
  'src/tip.c',
  'src/traitement_variables.c',
  'src/transaction_list.c',
  'src/transaction_list_select.c',
  'src/transaction_list_sort.c',
  'src/transaction_model.c',
  'src/utils.c',
  'src/utils_buttons.c',
  'src/utils_dates.c',
  'src/utils_editables.c',
  'src/utils_files.c',
  'src/utils_operations.c',
  'src/utils_prefs.c',
  'src/utils_real.c',
  'src/utils_str.c',
  'src/utils_widgets.c',
  'src/widgets/widget_account_property.c',
  'src/widgets/widget_assistant.c',
  'src/widgets/widget_bank_details.c',
  'src/widgets/widget_bet_graph_options.c',
  'src/widgets/widget_bet_graph_others.c',
  'src/widgets/widget_bet_graph_pie.c',
  'src/widgets/widget_cmp_amount.c',
  'src/widgets/widget_cmp_text.c',
  'src/widgets/widget_css_rules.c',
  'src/widgets/widget_currency_details.c',
  'src/widgets/widget_currency_link.c',
  'src/widgets/widget_currency_popup.c',
  'src/widgets/widget_import_asso.c',
  'src/widgets/widget_import_files.c',
  'src/widgets/widget_list_accounts.c',
  'src/widgets/widget_list_metatree.c',
  'src/widgets/widget_loan.c',
  'src/widgets/widget_reconcile.c',
  'src/widgets/widget_search_tiers_categ_ib.c',
  'src/widgets/widget_search_transaction.c',
  'src/widgets/widget_transfer.c',
  ]
grisbi_src += grisbi_resources

# special for macOS
if host_machine.system() == 'darwin'
  grisbi_src += files('src/grisbi_osx.m')
  add_languages('objc', native : false)
  conf_data.set('GTKOSXAPPLICATION', true)
  conf_data.set('OS_OSX', true)
  gtk_mac_integration_dep = dependency('gtk-mac-integration')
  grisbi_dep += gtk_mac_integration_dep
endif

# special for Windows
if host_machine.system() == 'windows'
  confgen_data = configuration_data({
    'VERSION' : meson.project_version(),
    'NSIS_BITS' : '64bit',
    'NSIS_DEFAULTINSTDIR' : 'Program Files',
    'NSIS_INSTDIR' : get_option('prefix').replace('/', '\\'),
    'TOP_SRCDIR_WINBS' : meson.global_source_root(),
    'NSIS_SRCDIR' : meson.global_source_root().replace('/', '\\'),
    })

  r = run_command('date', '+-%Y.%m.%d-%H', check : true)
  confgen_data.set('NSIS_DATE', r.stdout().strip())

  versions = meson.project_version().split('.')
  confgen_data.set('GRISBI_MAJOR_VERSION', versions[0])
  confgen_data.set('GRISBI_MINOR_VERSION', versions[1])
  confgen_data.set('GRISBI_MICRO_VERSION', versions[2])

  # generate files from .in
  configure_file(output : 'grisbi.nsi',
    input : 'share/grisbi.nsi.in',
    configuration : confgen_data
    )

  meson.add_install_script('share/msys2-extra-install-hook.sh', get_option('prefix'))
endif

# grisbi target
executable('grisbi',
  sources : grisbi_src,
  include_directories : incdir,
  dependencies : grisbi_dep,
  link_args : ['-lm'],
  install : true)

# manpage
install_man('grisbi.1')

# ui files
install_data([
  'src/ui/csv_template_rule.ui',
  'src/ui/etats_prefs.ui',
  'src/ui/grisbi-dark.css',
  'src/ui/grisbi-widgets.css',
  'src/ui/grisbi.css',
  'src/ui/grisbi_menu.ui',
  'src/ui/grisbi_prefs.ui',
  'src/ui/grisbi_win.ui',
  ],
  install_dir : join_paths (get_option ('datadir'), 'grisbi/ui')
  )

# default categories
install_data([
  'share/categories/C/00_general.cgsb',
  'share/categories/C/empty.cgsb',
  ],
  install_dir : join_paths (get_option ('datadir'), 'grisbi/categories/C')
  )

install_data([
  'share/categories/de/keine.cgsb',
  'share/categories/de/minimal.cgsb',
  'share/categories/de/standard.cgsb',
  ],
  install_dir : join_paths (get_option ('datadir'), 'grisbi/categories/de')
  )

install_data([
  'share/categories/fr/00_general.cgsb',
  'share/categories/fr/empty.cgsb',
  'share/categories/fr/liberal.cgsb',
  'share/categories/fr/plan-associatif-simplifie.cgsb',
  'share/categories/fr/plan-comptable.cgsb',
  ],
  install_dir : join_paths (get_option ('datadir'), 'grisbi/categories/fr')
  )

install_data([
  'share/categories/it/00_general.cgsb',
  'share/categories/it/empty.cgsb',
  ],
  install_dir : join_paths (get_option ('datadir'), 'grisbi/categories/it')
  )

install_data([
  'share/categories/ru/00_general.cgsb',
  'share/categories/ru/empty.cgsb',
  ],
  install_dir : join_paths (get_option ('datadir'), 'grisbi/categories/ru')
  )

# misc
install_data(
  'share/grisbi.xml',
  install_dir : join_paths (get_option ('datadir'), 'mime/packages')
  )

install_data(
  ['src/grisbi.keys', 'src/grisbi.mime'],
  install_dir : join_paths (get_option ('datadir'), 'mime-info')
  )

subdir('pixmaps')
subdir('help')

# generate config.h
configure_file(output : 'config.h',
  configuration : conf_data)
