#ifndef USE_CONFIG_FILE

#ifndef __GRISBI_SETTINGS_H__
#define __GRISBI_SETTINGS_H__


#include <gtk/gtk.h>
#include <glib.h>

/*START_INCLUDE*/
/*END_INCLUDE*/

G_BEGIN_DECLS

#define GRISBI_TYPE_SETTINGS    (grisbi_settings_get_type ())
#define GRISBI_SETTINGS(obj)    (G_TYPE_CHECK_INSTANCE_CAST ((obj), GRISBI_TYPE_SETTINGS, GrisbiSettings))
#define GRISBI_IS_SETTINGS(obj) (G_TYPE_CHECK_INSTANCE_TYPE((obj), GRISBI_TYPE_SETTINGS))

typedef struct _GrisbiSettings          GrisbiSettings;
typedef struct _GrisbiSettingsClass     GrisbiSettingsClass;
typedef struct _GrisbiSettingsPrivate   GrisbiSettingsPrivate;


struct _GrisbiSettings
{
    GObject parent;

    /*<private>*/
    GrisbiSettingsPrivate *priv;
};

struct _GrisbiSettingsClass
{
    GObjectClass parent_class;
};

/* START_DECLARATION */
GType               grisbi_settings_get_type			(void) G_GNUC_CONST;
GrisbiSettings *	grisbi_settings_load_app_config		(void);
void                grisbi_settings_save_app_config		(void);

/* sert au changement de système de configuration PROVISOIRE */
void				grisbi_settings_save_in_config_file (void);
/* END_DECLARATION */

G_END_DECLS

#endif  /* __GRISBI_SETTINGS_H__ */
#endif	/* USE_CONFIG_FILE */
