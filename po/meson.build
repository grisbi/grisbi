# To generate the grisbi.pot file
# meson compile grisbi-pot
#
# to update the .po files
# meson compile grisbi-update-po
#
# See https://mesonbuild.com/Localisation.html

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

# Validate MetaInfo file
metainfo_file = 'po/grisbi.metainfo.xml'
ascli_exe = find_program('appstreamcli', required: false)
if ascli_exe.found()
  test('validate metainfo file',
        ascli_exe,
        args: ['validate',
               '--no-net',
               '--pedantic',
               metainfo_file]
  )
endif
