#ifndef OFX_H
#define OFX_H

#ifdef NOOFX
#define recuperation_donnees_ofx(name) dialogue_error_hint(_("This build of Grisbi does not support OFX, please recompile Grisbi with OFX support enabled"), g_strdup_printf (_("Cannot process OFX file '%s'"), name))
#else
gboolean recuperation_donnees_ofx ( gchar *nom_fichier );
#endif

#endif
