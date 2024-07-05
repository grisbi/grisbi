i18n = import('i18n')
# define GETTEXT_PACKAGE
add_project_arguments('-DGETTEXT_PACKAGE="grisbi"', language:'c')
i18n.gettext(meson.project_name(),
  args: ['--from-code=UTF-8'])

i18n.merge_file(
  type : 'desktop',
  input : '../share/grisbi.desktop.in',
  output : 'org.grisbi.Grisbi.desktop',
  po_dir : '',
  install : true,
  install_dir : 'share/applications',
  )

i18n.merge_file(
  type : 'xml',
  input : '../share/grisbi.metainfo.xml.in',
  output : 'grisbi.metainfo.xml',
  po_dir : '',
  install : true,
  install_dir : 'share/metainfo',
  )
