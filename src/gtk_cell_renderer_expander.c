/* gtkcellrendererexpander.c
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

#include <config.h>
#include <stdlib.h>
#include <glib-object.h>
#include <glib-object.h>

#include "include.h"


#define _gtk_marshal_VOID__STRING	g_cclosure_marshal_VOID__STRING


/*START_INCLUDE*/
#include "gtk_cell_renderer_expander.h"
/*END_INCLUDE*/

/*START_STATIC*/
static  gint gtk_cell_renderer_expander_activate (GtkCellRenderer      *cell,
				   GdkEvent             *event,
				   GtkWidget            *widget,
				   const gchar          *path,
				   GdkRectangle         *background_area,
				   GdkRectangle         *cell_area,
				   GtkCellRendererState  flags);
static  void gtk_cell_renderer_expander_class_init (GtkCellRendererExpanderClass *class);
static gboolean gtk_cell_renderer_expander_get_active (GtkCellRendererExpander *expander);
static  void gtk_cell_renderer_expander_get_property (GObject     *object,
				       guint        param_id,
				       GValue      *value,
				       GParamSpec  *pspec);
static  void gtk_cell_renderer_expander_get_size (GtkCellRenderer *cell,
				   GtkWidget       *widget,
				   GdkRectangle    *cell_area,
				   gint            *x_offset,
				   gint            *y_offset,
				   gint            *width,
				   gint            *height);
static GType gtk_cell_renderer_expander_get_type (void);
static  void gtk_cell_renderer_expander_init (GtkCellRendererExpander *cellexpander);
static  void gtk_cell_renderer_expander_render (GtkCellRenderer      *cell,
				 GdkDrawable          *window,
				 GtkWidget            *widget,
				 GdkRectangle         *background_area,
				 GdkRectangle         *cell_area,
				 GdkRectangle         *expose_area,
				 GtkCellRendererState  flags);
static void gtk_cell_renderer_expander_set_active (GtkCellRendererExpander *expander,
				     gboolean               setting);
static  void gtk_cell_renderer_expander_set_property (GObject      *object,
				       guint         param_id,
				       const GValue *value,
				       GParamSpec   *pspec);
/*END_STATIC*/

/*START_EXTERN*/
extern GtkTreeStore *model;
extern GtkWidget *window;
/*END_EXTERN*/



enum {
  EXPANDERD,
  LAST_SIGNAL
};

enum {
  PROP_ZERO,
  PROP_ACTIVATABLE,
  PROP_ACTIVE,
  PROP_INCONSISTENT
};

#define EXPANDER_WIDTH 12

static guint expander_cell_signals[LAST_SIGNAL] = { 0 };

#define GTK_CELL_RENDERER_EXPANDER_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE ((obj), GTK_TYPE_CELL_RENDERER_EXPANDER, GtkCellRendererExpanderPrivate))

typedef struct _GtkCellRendererExpanderPrivate GtkCellRendererExpanderPrivate;
struct _GtkCellRendererExpanderPrivate
{
  guint inconsistent : 1;
};


GType gtk_cell_renderer_expander_get_type (void)
{
  static GType cell_expander_type = 0;

  if (!cell_expander_type)
    {
      static const GTypeInfo cell_expander_info =
      {
	sizeof (GtkCellRendererExpanderClass),
	NULL,		/* base_init */
	NULL,		/* base_finalize */
	(GClassInitFunc) gtk_cell_renderer_expander_class_init,
	NULL,		/* class_finalize */
	NULL,		/* class_data */
	sizeof (GtkCellRendererExpander),
	0,              /* n_preallocs */
	(GInstanceInitFunc) gtk_cell_renderer_expander_init,
      };

      cell_expander_type =
	g_type_register_static (GTK_TYPE_CELL_RENDERER, "GtkCellRendererExpander",
				&cell_expander_info, 0);
    }

  return cell_expander_type;
}

static void gtk_cell_renderer_expander_init (GtkCellRendererExpander *cellexpander)
{
  cellexpander->activatable = TRUE;
  cellexpander->active = FALSE;
  GTK_CELL_RENDERER (cellexpander)->mode = GTK_CELL_RENDERER_MODE_ACTIVATABLE;
  GTK_CELL_RENDERER (cellexpander)->xpad = 2;
  GTK_CELL_RENDERER (cellexpander)->ypad = 2;
}

static void gtk_cell_renderer_expander_class_init (GtkCellRendererExpanderClass *class)
{
  GObjectClass *object_class = G_OBJECT_CLASS (class);
  GtkCellRendererClass *cell_class = GTK_CELL_RENDERER_CLASS (class);

  object_class->get_property = gtk_cell_renderer_expander_get_property;
  object_class->set_property = gtk_cell_renderer_expander_set_property;

  cell_class->get_size = gtk_cell_renderer_expander_get_size;
  cell_class->render = gtk_cell_renderer_expander_render;
  cell_class->activate = gtk_cell_renderer_expander_activate;
  
  g_object_class_install_property (object_class,
				   PROP_ACTIVE,
				   g_param_spec_boolean ("active",
							 N_("Expander state"),
							 N_("The expander state of the button"),
							 FALSE,
							 G_PARAM_READABLE |
							 G_PARAM_WRITABLE));

  g_object_class_install_property (object_class,
		                   PROP_INCONSISTENT,
				   g_param_spec_boolean ("inconsistent",
					                 N_("Inconsistent state"),
							 N_("The inconsistent state of the button"),
							 FALSE,
							 G_PARAM_READABLE |
							 G_PARAM_WRITABLE));
  
  g_object_class_install_property (object_class,
				   PROP_ACTIVATABLE,
				   g_param_spec_boolean ("activatable",
							 N_("Activatable"),
							 N_("The expander button can be activated"),
							 TRUE,
							 G_PARAM_READABLE |
							 G_PARAM_WRITABLE));

  
  /**
   * GtkCellRendererExpander::expanderd:
   * @cell_renderer: the object which received the signal
   * @path: string representation of #GtkTreePath describing the 
   *        event location
   *
   * The ::expanderd signal is emitted when the cell is expanderd. 
   **/
  expander_cell_signals[EXPANDERD] =
    g_signal_new ("expanderd",
		  G_OBJECT_CLASS_TYPE (object_class),
		  G_SIGNAL_RUN_LAST,
		  G_STRUCT_OFFSET (GtkCellRendererExpanderClass, expanderd),
		  NULL, NULL,
		  _gtk_marshal_VOID__STRING,
		  G_TYPE_NONE, 1,
		  G_TYPE_STRING);

  g_type_class_add_private (object_class, sizeof (GtkCellRendererExpanderPrivate));
}


static void gtk_cell_renderer_expander_get_property (GObject     *object,
				       guint        param_id,
				       GValue      *value,
				       GParamSpec  *pspec)
{
  GtkCellRendererExpander *cellexpander = GTK_CELL_RENDERER_EXPANDER (object);
  GtkCellRendererExpanderPrivate *priv;

  priv = GTK_CELL_RENDERER_EXPANDER_GET_PRIVATE (object);
  
  switch (param_id)
    {
    case PROP_ACTIVE:
      g_value_set_boolean (value, cellexpander->active);
      break;
    case PROP_INCONSISTENT:
      g_value_set_boolean (value, priv->inconsistent);
      break;
    case PROP_ACTIVATABLE:
      g_value_set_boolean (value, cellexpander->activatable);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, param_id, pspec);
      break;
    }
}


static void gtk_cell_renderer_expander_set_property (GObject      *object,
				       guint         param_id,
				       const GValue *value,
				       GParamSpec   *pspec)
{
  GtkCellRendererExpander *cellexpander = GTK_CELL_RENDERER_EXPANDER (object);
  GtkCellRendererExpanderPrivate *priv;

  priv = GTK_CELL_RENDERER_EXPANDER_GET_PRIVATE (object);
  
  switch (param_id)
    {
    case PROP_ACTIVE:
      cellexpander->active = g_value_get_boolean (value);
      g_object_notify (G_OBJECT(object), "active");
      break;
    case PROP_INCONSISTENT:
      priv->inconsistent = g_value_get_boolean (value);
      g_object_notify (G_OBJECT (object), "inconsistent");
      break;
    case PROP_ACTIVATABLE:
      cellexpander->activatable = g_value_get_boolean (value);
      g_object_notify (G_OBJECT(object), "activatable");
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, param_id, pspec);
      break;
    }
}

/**
 * gtk_cell_renderer_expander_new:
 * 
 * Creates a new #GtkCellRendererExpander. Adjust rendering
 * parameters using object properties. Object properties can be set
 * globally (with g_object_set()). Also, with #GtkTreeViewColumn, you
 * can bind a property to a value in a #GtkTreeModel. For example, you
 * can bind the "active" property on the cell renderer to a boolean value
 * in the model, thus causing the check button to reflect the state of
 * the model.
 * 
 * Return value: the new cell renderer
 **/
GtkCellRenderer * gtk_cell_renderer_expander_new (void)
{
  return g_object_new (GTK_TYPE_CELL_RENDERER_EXPANDER, NULL);
}

static void gtk_cell_renderer_expander_get_size (GtkCellRenderer *cell,
				   GtkWidget       *widget,
				   GdkRectangle    *cell_area,
				   gint            *x_offset,
				   gint            *y_offset,
				   gint            *width,
				   gint            *height)
{
  gint calc_width;
  gint calc_height;

  calc_width = (gint) cell->xpad * 2 + EXPANDER_WIDTH;
  calc_height = (gint) cell->ypad * 2 + EXPANDER_WIDTH;

  if (width)
    *width = calc_width;

  if (height)
    *height = calc_height;

  if (cell_area)
    {
      if (x_offset)
	{
	  *x_offset = ((gtk_widget_get_direction (widget) == GTK_TEXT_DIR_RTL) ?
		       (1.0 - cell->xalign) : cell->xalign) * (cell_area->width - calc_width);
	  *x_offset = MAX (*x_offset, 0);
	}
      if (y_offset)
	{
	  *y_offset = cell->yalign * (cell_area->height - calc_height);
	  *y_offset = MAX (*y_offset, 0);
	}
    }
}

static void gtk_cell_renderer_expander_render (GtkCellRenderer      *cell,
				 GdkDrawable          *window,
				 GtkWidget            *widget,
				 GdkRectangle         *background_area,
				 GdkRectangle         *cell_area,
				 GdkRectangle         *expose_area,
				 GtkCellRendererState  flags)
{
  GtkCellRendererExpander *cellexpander = (GtkCellRendererExpander *) cell;
  GtkCellRendererExpanderPrivate *priv;
  gint width, height;
  gint x_offset, y_offset;
  GtkShadowType shadow;
  GtkStateType state = 0;
  GtkExpanderStyle style;

  priv = GTK_CELL_RENDERER_EXPANDER_GET_PRIVATE (cell);

  gtk_cell_renderer_expander_get_size (cell, widget, cell_area,
				     &x_offset, &y_offset,
				     &width, &height);
  width -= cell->xpad*2;
  height -= cell->ypad*2;

  if (width <= 0 || height <= 0)
    return;

  if (priv->inconsistent)
    shadow = GTK_SHADOW_ETCHED_IN;
  else
    shadow = cellexpander->active ? GTK_SHADOW_IN : GTK_SHADOW_OUT;

  if ((flags & GTK_CELL_RENDERER_SELECTED) == GTK_CELL_RENDERER_SELECTED)
    {
      if (GTK_WIDGET_HAS_FOCUS (widget))
	state = GTK_STATE_SELECTED;
      else
	state = GTK_STATE_ACTIVE;
    }
  else
    {
      if (cellexpander->activatable)
        state = GTK_STATE_NORMAL;
      else
        state = GTK_STATE_INSENSITIVE;
    }

  if ( cell->is_expanded )
    style = GTK_EXPANDER_EXPANDED;
  else
    style = GTK_EXPANDER_COLLAPSED;


  if ( cell->is_expander )
    gtk_paint_expander (widget->style,
			window,
			state,
			expose_area,
			widget,
			"cellcheck",
			cell_area->x + cell_area->width / 2,
			cell_area->y + cell_area->height / 2,
			style );

}

static gint gtk_cell_renderer_expander_activate (GtkCellRenderer      *cell,
				   GdkEvent             *event,
				   GtkWidget            *widget,
				   const gchar          *path,
				   GdkRectangle         *background_area,
				   GdkRectangle         *cell_area,
				   GtkCellRendererState  flags)
{
  GtkCellRendererExpander *cellexpander;
  
  cellexpander = GTK_CELL_RENDERER_EXPANDER (cell);
  if (cellexpander->activatable)
    {
      g_signal_emit (cell, expander_cell_signals[EXPANDERD], 0, path);
      return TRUE;
    }

  return FALSE;
}


/**
 * gtk_cell_renderer_expander_get_active:
 * @expander: a #GtkCellRendererExpander
 *
 * Returns whether the cell renderer is active. See
 * gtk_cell_renderer_expander_set_active().
 *
 * Return value: %TRUE if the cell renderer is active.
 **/
gboolean gtk_cell_renderer_expander_get_active (GtkCellRendererExpander *expander)
{
  g_return_val_if_fail (GTK_IS_CELL_RENDERER_EXPANDER (expander), FALSE);

  return expander->active;
}

/**
 * gtk_cell_renderer_expander_set_active:
 * @expander: a #GtkCellRendererExpander.
 * @setting: the value to set.
 *
 * Activates or deactivates a cell renderer.
 **/
void gtk_cell_renderer_expander_set_active (GtkCellRendererExpander *expander,
				     gboolean               setting)
{
  g_return_if_fail (GTK_IS_CELL_RENDERER_EXPANDER (expander));

  g_object_set (expander, "active", setting ? TRUE : FALSE, NULL);
}
