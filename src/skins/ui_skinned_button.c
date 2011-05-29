/*
 * Audacious - a cross-platform multimedia player
 * Copyright (c) 2007 Tomasz Moń
 * Copyright (c) 2011 John Lindgren
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; under version 3 of the License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses>.
 *
 * The Audacious team does not consider modular code linking to
 * Audacious or using our public API to be a derived work.
 */

#include "draw-compat.h"
#include "skins_cfg.h"
#include "ui_skinned_button.h"

enum {BUTTON_TYPE_NORMAL, BUTTON_TYPE_TOGGLE, BUTTON_TYPE_SMALL};

typedef struct {
    gint type;
    gint w, h;
    gint nx, ny, px, py;
    gint pnx, pny, ppx, ppy;
    SkinPixmapId si1, si2;
    gboolean hover, pressed, active;
    ButtonCB on_press;
    ButtonCB on_release;
    ButtonCB on_rclick;
} ButtonData;

DRAW_FUNC_BEGIN (button_draw)
    ButtonData * data = g_object_get_data ((GObject *) wid, "buttondata");
    g_return_val_if_fail (data, FALSE);

    GdkPixbuf * p = gdk_pixbuf_new (GDK_COLORSPACE_RGB, TRUE, 8, data->w,
     data->h);

    gboolean down = data->hover && data->pressed;

    switch (data->type)
    {
    case BUTTON_TYPE_NORMAL:
        skin_draw_pixbuf (wid, aud_active_skin, p, down ? data->si2 :
         data->si1, down ? data->px : data->nx, down ? data->py : data->ny, 0,
         0, data->w, data->h);
        break;
    case BUTTON_TYPE_TOGGLE:
        if (data->active)
            skin_draw_pixbuf (wid, aud_active_skin, p, down ? data->si2 :
             data->si1, down ? data->ppx : data->pnx, down ? data->ppy :
             data->pny, 0, 0, data->w, data->h);
        else
            skin_draw_pixbuf (wid, aud_active_skin, p, down ? data->si2 :
             data->si1, down ? data->px : data->nx, down ? data->py : data->ny,
             0, 0, data->w, data->h);
        break;
    }

    pixbuf_draw (cr, p, 0, 0, FALSE);

    g_object_unref (p);
DRAW_FUNC_END

static gboolean button_press (GtkWidget * button, GdkEventButton * event)
{
    ButtonData * data = g_object_get_data ((GObject *) button, "buttondata");
    g_return_val_if_fail (data, FALSE);

    if (event->button != 1)
        return FALSE;

    data->pressed = TRUE;

    if (data->on_press)
        data->on_press (button, event);

    if (data->type != BUTTON_TYPE_SMALL)
        gtk_widget_queue_draw (button);

    return TRUE;
}

static gboolean button_release (GtkWidget * button, GdkEventButton * event)
{
    ButtonData * data = g_object_get_data ((GObject *) button, "buttondata");
    g_return_val_if_fail (data, FALSE);

    if (event->button == 3 && data->on_rclick)
    {
        data->on_rclick (button, event);
        return TRUE;
    }

    if (event->button != 1)
        return FALSE;

    data->pressed = FALSE;

    if (data->type == BUTTON_TYPE_TOGGLE)
        data->active = ! data->active;

    if (data->on_release)
        data->on_release (button, event);

    if (data->type != BUTTON_TYPE_SMALL)
        gtk_widget_queue_draw (button);

    return TRUE;
}

static gboolean enter_notify (GtkWidget * button, GdkEventCrossing * event)
{
    ButtonData * data = g_object_get_data ((GObject *) button, "buttondata");
    g_return_val_if_fail (data, FALSE);

    data->hover = TRUE;

    if (data->pressed && data->type != BUTTON_TYPE_SMALL)
        gtk_widget_queue_draw (button);

    return TRUE;
}

static gboolean leave_notify (GtkWidget * button, GdkEventCrossing * event)
{
    ButtonData * data = g_object_get_data ((GObject *) button, "buttondata");
    g_return_val_if_fail (data, FALSE);

    data->hover = FALSE;

    if (data->pressed && data->type != BUTTON_TYPE_SMALL)
        gtk_widget_queue_draw (button);

    return TRUE;
}

static GtkWidget * button_new_base (gint type, gint w, gint h)
{
    GtkWidget * button;

    if (type == BUTTON_TYPE_SMALL)
    {
        button = gtk_event_box_new ();
        gtk_event_box_set_visible_window ((GtkEventBox *) button, FALSE);
    }
    else
        button = gtk_drawing_area_new ();

    gtk_widget_set_size_request (button, w, h);
    gtk_widget_add_events (button, GDK_BUTTON_PRESS_MASK |
     GDK_BUTTON_RELEASE_MASK | GDK_ENTER_NOTIFY_MASK | GDK_LEAVE_NOTIFY_MASK);

    if (type != BUTTON_TYPE_SMALL)
        g_signal_connect (button, DRAW_SIGNAL, (GCallback) button_draw, NULL);

    g_signal_connect (button, "button-press-event", (GCallback) button_press,
     NULL);
    g_signal_connect (button, "button-release-event", (GCallback)
     button_release, NULL);
    g_signal_connect (button, "enter-notify-event", (GCallback) enter_notify,
     NULL);
    g_signal_connect (button, "leave-notify-event", (GCallback) leave_notify,
     NULL);

    ButtonData * data = g_malloc0 (sizeof (ButtonData));
    data->type = type;
    data->w = w;
    data->h = h;
    g_object_set_data ((GObject *) button, "buttondata", data);

    return button;
}

GtkWidget * button_new (gint w, gint h, gint nx, gint ny, gint px, gint py,
 SkinPixmapId si1, SkinPixmapId si2)
{
    GtkWidget * button = button_new_base (BUTTON_TYPE_NORMAL, w, h);
    ButtonData * data = g_object_get_data ((GObject *) button, "buttondata");
    g_return_val_if_fail (data, NULL);

    data->nx = nx;
    data->ny = ny;
    data->px = px;
    data->py = py;
    data->si1 = si1;
    data->si2 = si2;

    return button;
}

GtkWidget * button_new_toggle (gint w, gint h, gint nx, gint ny, gint px, gint
 py, gint pnx, gint pny, gint ppx, gint ppy, SkinPixmapId si1, SkinPixmapId si2)
{
    GtkWidget * button = button_new_base (BUTTON_TYPE_TOGGLE, w, h);
    ButtonData * data = g_object_get_data ((GObject *) button, "buttondata");
    g_return_val_if_fail (data, NULL);

    data->nx = nx;
    data->ny = ny;
    data->px = px;
    data->py = py;
    data->pnx = pnx;
    data->pny = pny;
    data->ppx = ppx;
    data->ppy = ppy;
    data->si1 = si1;
    data->si2 = si2;

    return button;
}

GtkWidget * button_new_small (gint w, gint h)
{
    return button_new_base (BUTTON_TYPE_SMALL, w, h);
}

void button_on_press (GtkWidget * button, ButtonCB callback)
{
    ButtonData * data = g_object_get_data ((GObject *) button, "buttondata");
    g_return_if_fail (data);

    data->on_press = callback;
}

void button_on_release (GtkWidget * button, ButtonCB callback)
{
    ButtonData * data = g_object_get_data ((GObject *) button, "buttondata");
    g_return_if_fail (data);

    data->on_release = callback;
}

void button_on_rclick (GtkWidget * button, ButtonCB callback)
{
    ButtonData * data = g_object_get_data ((GObject *) button, "buttondata");
    g_return_if_fail (data);

    data->on_rclick = callback;
}

gboolean button_get_active (GtkWidget * button)
{
    ButtonData * data = g_object_get_data ((GObject *) button, "buttondata");
    g_return_val_if_fail (data && data->type == BUTTON_TYPE_TOGGLE, FALSE);

    return data->active;
}

void button_set_active (GtkWidget * button, gboolean active)
{
    ButtonData * data = g_object_get_data ((GObject *) button, "buttondata");
    g_return_if_fail (data && data->type == BUTTON_TYPE_TOGGLE);

    if (data->active == active)
        return;

    data->active = active;
    gtk_widget_queue_draw (button);
}
