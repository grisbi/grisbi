/* gtkcellrendererexpander.h
 * Copyright (C) 2000  Red Hat, Inc.,  Jonathan Blandford <jrb@redhat.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

#ifndef __GTK_CELL_RENDERER_EXPANDER_H__
#define __GTK_CELL_RENDERER_EXPANDER_H__

#include <gtk/gtkcellrenderer.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


#define GTK_TYPE_CELL_RENDERER_EXPANDER			(gtk_cell_renderer_expander_get_type ())
#define GTK_CELL_RENDERER_EXPANDER(obj)			(G_TYPE_CHECK_INSTANCE_CAST ((obj), GTK_TYPE_CELL_RENDERER_EXPANDER, GtkCellRendererExpander))
#define GTK_CELL_RENDERER_EXPANDER_CLASS(klass)		(G_TYPE_CHECK_CLASS_CAST ((klass), GTK_TYPE_CELL_RENDERER_EXPANDER, GtkCellRendererExpanderClass))
#define GTK_IS_CELL_RENDERER_EXPANDER(obj)		(G_TYPE_CHECK_INSTANCE_TYPE ((obj), GTK_TYPE_CELL_RENDERER_EXPANDER))
#define GTK_IS_CELL_RENDERER_EXPANDER_CLASS(klass)	(G_TYPE_CHECK_CLASS_TYPE ((klass), GTK_TYPE_CELL_RENDERER_EXPANDER))
#define GTK_CELL_RENDERER_EXPANDER_GET_CLASS(obj)         (G_TYPE_INSTANCE_GET_CLASS ((obj), GTK_TYPE_CELL_RENDERER_EXPANDER, GtkCellRendererExpanderClass))

typedef struct _GtkCellRendererExpander GtkCellRendererExpander;
typedef struct _GtkCellRendererExpanderClass GtkCellRendererExpanderClass;

struct _GtkCellRendererExpander
{
  GtkCellRenderer parent;

  /*< private >*/
  guint active : 1;
  guint activatable : 1;
  guint radio : 1;
};

struct _GtkCellRendererExpanderClass
{
  GtkCellRendererClass parent_class;

  void (* expanderd) (GtkCellRendererExpander *cell_renderer_expander,
		    const gchar                 *path);

  /* Padding for future expansion */
  void (*_gtk_reserved1) (void);
  void (*_gtk_reserved2) (void);
  void (*_gtk_reserved3) (void);
  void (*_gtk_reserved4) (void);
};

/* START_DECLARATION */
GtkCellRenderer * gtk_cell_renderer_expander_new (void);
/* END_DECLARATION */


#ifdef __cplusplus
}
#endif /* __cplusplus */


#endif /* __GTK_CELL_RENDERER_EXPANDER_H__ */
