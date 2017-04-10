/*
 * Gstreamer Daemon - Gst Launch under steroids
 * Copyright (C) 2017 RidgeRun Engineering <support@ridgerun.com>
 *
 * This file is part of Gstd.
 *
 * Gstd is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Gstd is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with Gstd.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "gstd_event_creator.h"
#include "gstd_event_factory.h"

enum
{
  PROP_RECEIVER = 1,
  N_PROPERTIES                  // NOT A PROPERTY
};

struct _GstdEventCreator
{
  GObject parent;

  GObject *receiver;
};

struct _GstdEventCreatorClass
{
  GObjectClass parent_class;
};

static void
gstd_event_creator_set_property (GObject *,
    guint, const GValue *, GParamSpec *);
static void gstd_event_creator_create (GstdICreator * iface,
    const gchar * name, const gchar * description, GstdObject ** out);

/**
 * gstd_event_send_event:
 * @self: The member of the corresponding element that will send a gst event.
 * @name: Event type that will be sent. 
 * @description: (nullable) Parameters of the event_type.
 * 
 * Sends the specified event to the receiver object.
 *
 * Returns: TRUE if the event is sent succesfully to the receiver. FALSE otherwise.
 */
static GstdReturnCode gstd_event_creator_send_event (GstdEventCreator * self,
    const gchar * name, const gchar * description);

static void
gstd_icreator_interface_init (GstdICreatorInterface * iface)
{
  iface->create = gstd_event_creator_create;
}

G_DEFINE_TYPE_WITH_CODE (GstdEventCreator, gstd_event_creator,
    G_TYPE_OBJECT, G_IMPLEMENT_INTERFACE (GSTD_TYPE_ICREATOR,
        gstd_icreator_interface_init));

/* Gstd Event debugging category */
GST_DEBUG_CATEGORY_STATIC (gstd_event_creator_debug);
#define GST_CAT_DEFAULT gstd_event_creator_debug
#define GSTD_DEBUG_DEFAULT_LEVEL GST_LEVEL_INFO

static void
gstd_event_creator_class_init (GstdEventCreatorClass * klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  GParamSpec *properties[N_PROPERTIES] = { NULL, };
  object_class->set_property = gstd_event_creator_set_property;

  properties[PROP_RECEIVER] =
      g_param_spec_object ("receiver",
      "Receiver",
      "The object that will receive the event",
      G_TYPE_OBJECT,
      G_PARAM_CONSTRUCT_ONLY | G_PARAM_WRITABLE | G_PARAM_STATIC_STRINGS);

  g_object_class_install_properties (object_class, N_PROPERTIES, properties);

  /* Initialize debug category with nice colors */
  guint debug_color = GST_DEBUG_FG_BLACK | GST_DEBUG_BOLD | GST_DEBUG_BG_WHITE;
  GST_DEBUG_CATEGORY_INIT (gstd_event_creator_debug, "gstdeventcreator",
      debug_color, "Gstd Event Creator category");
}

static void
gstd_event_creator_init (GstdEventCreator * self)
{
  GST_INFO_OBJECT (self, "Initializing gstd event creator");
  self->receiver = NULL;
}

static GstdReturnCode
gstd_event_creator_send_event (GstdEventCreator * self,
    const gchar * event_type, const gchar * description)
{
  GST_INFO_OBJECT (self, "Event Creator sending event %s", event_type);
  GstEvent *event = gstd_event_factory_make (event_type, description);
  if (event) {
    if (gst_element_send_event (GST_ELEMENT (self->receiver), event))
      return GSTD_EOK;
    else
      return GSTD_BAD_COMMAND;
  }
  return GSTD_BAD_VALUE;
}

static void
gstd_event_creator_set_property (GObject * object,
    guint property_id, const GValue * value, GParamSpec * pspec)
{
  GstdEventCreator *self = GSTD_EVENT_CREATOR (object);

  switch (property_id) {
    case PROP_RECEIVER:
      self->receiver = g_value_dup_object (value);
      GST_INFO_OBJECT (self, "Changed receiver to %p", self->receiver);
      break;
    default:
      /* We don't have any other property... */
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
  }
}

static void
gstd_event_creator_create (GstdICreator * iface, const gchar * name,
    const gchar * description, GstdObject ** out)
{
  GstdEventCreator *self;
  
  g_return_if_fail (iface);
  g_return_if_fail (name);
  g_return_if_fail (description);
  g_return_if_fail (out);

  self = GSTD_EVENT_CREATOR(iface);
  
  /* We don't return the newly created event */
  *out = NULL;
  
  gstd_event_creator_send_event (self, name, description);
}
