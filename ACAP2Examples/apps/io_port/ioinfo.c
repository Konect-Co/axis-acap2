#include <stdlib.h>
#include <glib.h>
#include <glib-object.h>
#include <glib/gprintf.h>
#include <axsdk/axparameter.h>
#include "ioinfo.h"

static void
input_port_update_name(const gchar *name,
    const gchar *value,
    gpointer data);

static void
input_port_update_trig(const gchar *name,
    const gchar *value,
    gpointer data);

static void
output_port_update_name(const gchar *name,
    const gchar *value,
    gpointer data);

static void
output_port_update_active(const gchar *name,
    const gchar *value,
    gpointer data);

static void
output_port_update_button(const gchar *name,
    const gchar *value,
    gpointer data);

static void
output_port_update_pulse_time(const gchar *name,
    const gchar *value,
    gpointer data);

port_t*
port_new(guint index,
    direction_t direction)
{
  port_t *port;

  port = g_slice_new(port_t);
  port->index = index;
  port->direction = direction;

  return port;
}

void
port_print(port_t *port)
{
  switch (port->direction) {
    case DIRECTION_INPUT:
      input_port_print((input_port_t*)port);
      break;
    case DIRECTION_OUTPUT:
      output_port_print((output_port_t*)port);
      break;
    default:
      g_assert_not_reached();
  }
}

void
port_free(port_t *port)
{
  switch (port->direction) {
    case DIRECTION_INPUT:
      input_port_free((input_port_t*)port);
      break;
    case DIRECTION_OUTPUT:
      output_port_free((output_port_t*)port);
      break;
    default:
      g_assert_not_reached();
  }
}

input_port_t*
input_port_new(guint index,
    const gchar *name,
    trig_t trig)
{
  input_port_t *input_port;

  input_port = g_slice_new(input_port_t);
  input_port->port.index = index;
  input_port->port.direction = DIRECTION_INPUT;
  input_port->name = g_strdup(name);
  input_port->trig = trig;

  return input_port;
}

void
input_port_print(input_port_t *input_port)
{
    g_printf("---Input port---\n"
        "     Index: %d\n"
        "      Name: %s\n"
        "      Trig: %s\n",
        input_port->port.index,
        input_port->name,
        trig_to_string(input_port->trig));
}

void
input_port_free(input_port_t *input_port)
{
  if (input_port != NULL) {
    g_free(input_port->name);
    g_slice_free(input_port_t, input_port);
  }
}

output_port_t*
output_port_new(guint index,
    const gchar *name,
    active_t active,
    button_t button,
    guint pulse_time)
{
  output_port_t *output_port;

  output_port = g_slice_new(output_port_t);
  output_port->port.index = index;
  output_port->port.direction = DIRECTION_OUTPUT;
  output_port->name = g_strdup(name);
  output_port->active = active;
  output_port->button = button;
  output_port->pulse_time = pulse_time;

  return output_port;
}

void
output_port_print(output_port_t *output_port)
{
  g_printf("---Output port---\n"
      "     Index: %d\n"
      "      Name: %s\n"
      "    Active: %s\n"
      "    Button: %s\n"
      "Pulse time: %d\n",
      output_port->port.index,
      output_port->name,
      active_to_string(output_port->active),
      button_to_string(output_port->button),
      output_port->pulse_time);
}

static void
input_port_update_name(G_GNUC_UNUSED const gchar *name,
    const gchar *value,
    gpointer data)
{
  input_port_t *input_port = data;

  g_free(input_port->name);
  input_port->name = g_strdup(value);

  g_printf("Update detected");
  input_port_print(input_port);
}

static void
input_port_update_trig(G_GNUC_UNUSED const gchar *name,
    const gchar *value,
    gpointer data)
{
  input_port_t *input_port = data;

  input_port->trig = string_to_trig(value);

  g_printf("Update detected");
  input_port_print(input_port);
}

static void
output_port_update_name(G_GNUC_UNUSED const gchar *name,
    const gchar *value,
    gpointer data)
{
  output_port_t *output_port = data;

  g_free(output_port->name);
  output_port->name = g_strdup(value);

  g_printf("Update detected");
  output_port_print(output_port);
}

static void
output_port_update_active(G_GNUC_UNUSED const gchar *name,
    const gchar *value,
    gpointer data)
{
  output_port_t *output_port = data;
  output_port->active = string_to_active(value);

  g_printf("Update detected");
  output_port_print(output_port);
}

static void
output_port_update_button(G_GNUC_UNUSED const gchar *name,
    const gchar *value,
    gpointer data)
{
  output_port_t *output_port = data;

  output_port->button = string_to_button(value);

  g_printf("Update detected");
  output_port_print(output_port);
}

static void
output_port_update_pulse_time(G_GNUC_UNUSED const gchar *name,
    const gchar *value,
    gpointer data)
{
  output_port_t *output_port = data;

  output_port->pulse_time = g_ascii_strtoull(value, NULL, 0);

  g_printf("Update detected");
  output_port_print(output_port);
}

void
output_port_free(output_port_t *output_port)
{
  if (output_port != NULL) {
    g_free(output_port->name);
    g_slice_free(output_port_t, output_port);
  }
}

gboolean
get_integer(AXParameter *axparameter,
    const gchar *param,
    guint *value,
    GError **error)
{
  gchar *svalue = NULL;
  gboolean result = FALSE;

  if (!ax_parameter_get(axparameter, param, &svalue, error))  {
    goto error;
  }

  *value = (guint)g_ascii_strtoull(svalue, NULL, 0);

  result = TRUE;

error:
  if (svalue != NULL) {
    g_free(svalue);
  }
  return result;
}

gboolean
get_trig(AXParameter *axparameter,
    const gchar *param,
    trig_t *trig,
    GError **error)
{
  gchar *svalue = NULL;
  gboolean result = FALSE;

  if (!ax_parameter_get(axparameter, param, &svalue, error)) {
    goto error;
  }

  if (g_strcmp0(svalue, "open") == 0) {
    *trig = TRIG_OPEN;
  } else if (g_strcmp0(svalue, "closed") == 0) {
    *trig = TRIG_CLOSED;
  } else {
    g_assert(0);
  }

  result = TRUE;
error:
  if (svalue != NULL) {
    g_free(svalue);
  }
  return result;
}

const gchar*
trig_to_string(trig_t trig)
{
  static const gchar *table[] = {
    "open",
    "close"
  };

  return table[trig];
}

trig_t
string_to_trig(const gchar *value)
{
  trig_t trig;

  if (g_strcmp0(value, "open") == 0) {
    trig = TRIG_OPEN;
  } else if (g_strcmp0(value, "closed") == 0) {
    trig = TRIG_CLOSED;
  } else {
    g_assert_not_reached();
  }

  return trig;
}

gboolean
get_direction(AXParameter *axparameter,
    guint index,
    direction_t *direction,
    GError **error)
{
  gchar *param = NULL;
  gchar *svalue = NULL;
  gboolean result = FALSE;

  param = g_strdup_printf("IOPort.I%d.Direction", index);

  if (!ax_parameter_get(axparameter, param, &svalue, error)) {
    goto error;
  }

  if (g_strcmp0(svalue, "input") == 0) {
    *direction = DIRECTION_INPUT;
  } else if (g_strcmp0(svalue, "output") == 0) {
    *direction = DIRECTION_OUTPUT;
  } else {
    g_assert(0);
  }

  result = TRUE;
error:
  if (svalue != NULL) {
    g_free(svalue);
  }
  if (param != NULL) {
    g_free(param);
  }
  return result;
}

gboolean
get_button(AXParameter *axparameter,
    const gchar *param,
    button_t *button,
    GError **error)
{
  gchar *svalue = NULL;
  gboolean result = FALSE;

  if (!ax_parameter_get(axparameter, param, &svalue, error)) {
    goto error;
  }

  *button = string_to_button(svalue);

  result = TRUE;
error:
  if (svalue != NULL) {
    g_free(svalue);
  }
  return result;
}

const gchar*
button_to_string(button_t button)
{
  static const gchar *table[] = {
    "none",
    "pulse",
    "actinact"
  };

  return table[button];
}

button_t
string_to_button(const gchar *value)
{
  button_t button;

  if (g_strcmp0(value, "none") == 0) {
    button = BUTTON_NONE;
  } else if(g_strcmp0(value, "pulse") == 0) {
    button = BUTTON_PULSE;
  } else if (g_strcmp0(value, "actinact") == 0) {
    button = BUTTON_ACTINACT;
  } else {
    g_assert_not_reached();
  }

  return button;
}

gboolean
get_active(AXParameter *axparameter,
    const gchar *param,
    active_t *active,
    GError **error)
{
  gchar *svalue = NULL;
  gboolean result = FALSE;

  if (!ax_parameter_get(axparameter, param, &svalue, error)) {
    goto error;
  }

  *active = string_to_active(svalue);

  result = TRUE;
error:
  if (svalue != NULL) {
    g_free(svalue);
  }
  return result;
}

const gchar*
active_to_string(active_t active)
{
  static const gchar *table[] = {
    "open",
    "closed"
  };

  return table[active];
}

active_t
string_to_active(const gchar *value)
{
  active_t active;
  if (g_strcmp0(value, "open") == 0) {
    active = ACTIVE_OPEN;
  } else if (g_strcmp0(value, "closed") == 0) {
    active = ACTIVE_CLOSED;
  } else {
    g_assert_not_reached();
  }

  return active;
}

gboolean
get_input_port_settings(AXParameter *axparameter,
    guint index,
    input_port_t **input_port,
    GError **error)
{
  gchar *name = NULL;
  trig_t trig;
  gchar *param_name = NULL;
  gchar *param_trig = NULL;
  gboolean result = FALSE;

  param_name = g_strdup_printf("IOPort.I%d.Input.Name", index);
  param_trig = g_strdup_printf("IOPort.I%d.Input.Trig", index);

  if (!ax_parameter_get(axparameter, param_name, &name, error)) {
    goto error;
  }

  if (!get_trig(axparameter, param_trig, &trig, error)) {
    goto error;
  }

  *input_port = input_port_new(index, name, trig);

  if (!ax_parameter_register_callback(axparameter, param_name,
        input_port_update_name, *input_port, error)) {
    goto error;
  }

  if (!ax_parameter_register_callback(axparameter, param_trig,
        input_port_update_trig, *input_port, error)) {
    goto error;
  }

  result = TRUE;
error:
  if (name != NULL) {
    g_free(name);
  }
  if (param_trig != NULL) {
    g_free(param_trig);
  }
  if (param_name != NULL) {
    g_free(param_name);
  }
  if (!result) {
    input_port_free(*input_port);
  }
  return result;
}

gboolean
get_output_port_settings(AXParameter *axparameter,
    guint index,
    output_port_t **output_port,
    GError **error)
{
  gchar *name = NULL;
  active_t active;
  button_t button;
  guint pulse_time;
  gchar *param_name = NULL;
  gchar *param_active = NULL;
  gchar *param_button = NULL;
  gchar *param_pulse = NULL;
  gboolean result = FALSE;

  param_name = g_strdup_printf("IOPort.I%d.Output.Name", index);
  param_active = g_strdup_printf("IOPort.I%d.Output.Active", index);
  param_button = g_strdup_printf("IOPort.I%d.Output.Button", index);
  param_pulse = g_strdup_printf("IOPort.I%d.Output.PulseTime", index);

  if (!ax_parameter_get(axparameter, param_name, &name, error)) {
    goto error;
  }

  if (!get_active(axparameter, param_active, &active, error)) {
    goto error;
  }

  if (!get_button(axparameter, param_button, &button, error)) {
    goto error;
  }

  if (!get_integer(axparameter, param_pulse, &pulse_time, error)) {
    goto error;
  }

  *output_port = output_port_new(index, name, active, button, pulse_time);

  if (!ax_parameter_register_callback(axparameter, param_name,
        output_port_update_name, output_port, error)) {
    goto error;
  }

  if (!ax_parameter_register_callback(axparameter, param_active,
        output_port_update_active, output_port, error)) {
    goto error;
  }

  if (!ax_parameter_register_callback(axparameter, param_button,
        output_port_update_button, output_port, error)) {
    goto error;
  }

  if (!ax_parameter_register_callback(axparameter, param_pulse,
        output_port_update_pulse_time, output_port, error)) {
    goto error;
  }

  result = TRUE;
error:
  if (name != NULL) {
    g_free(name);
  }
  if (param_pulse != NULL) {
    g_free(param_pulse);
  }
  if (param_button != NULL) {
    g_free(param_button);
  }
  if (param_name != NULL) {
    g_free(param_name);
  }
  return result;
}

gboolean
get_ports(AXParameter *axparameter,
    GHashTable **ports,
    GError **error)
{
  guint nbr_input_ports;
  guint nbr_output_ports;
  GHashTable *lports = NULL;
  guint i;
  gboolean result = FALSE;

  if (!get_integer(axparameter, "Input.NbrOfInputs",
        &nbr_input_ports, error)) {
    goto error;
  }

  if (!get_integer(axparameter, "Output.NbrOfOutputs",
        &nbr_output_ports, error)) {
    goto error;
  }

  lports = g_hash_table_new_full(g_direct_hash, g_direct_equal,
      NULL, (GDestroyNotify)port_free);

  for (i = 0; i < nbr_input_ports + nbr_output_ports; i++) {
    input_port_t *input_port;
    output_port_t *output_port;
    port_t *port;
    direction_t direction;

    if (!get_direction(axparameter, i, &direction, error)) {
      goto error;
    }

    switch (direction) {
      case DIRECTION_INPUT:
        if (!get_input_port_settings(axparameter, i, &input_port, error)) {
          goto error;
        }
        port = (port_t*)input_port;
        break;
      case DIRECTION_OUTPUT:
        if (!get_output_port_settings(axparameter, i, &output_port, error)) {
          goto error;
        }
        port = (port_t*)output_port;
        break;
      default:
        g_assert_not_reached();
    }

    g_hash_table_insert(lports, GUINT_TO_POINTER(port->index), port);
  }

  *ports = lports;

  result = TRUE;
error:
  if (!result) {
    if (lports != NULL) {
      g_hash_table_unref(lports);
    }
  }
  return result;
}

int main(void)
{
  GMainLoop *loop;
  AXParameter *axparameter;
  GHashTable *ports = NULL;
  GHashTableIter iter;
  guint index;
  port_t *port;
  gboolean result = FALSE;
  GError *error = NULL;

  if ((axparameter = ax_parameter_new(__FILE__, &error)) == NULL) {
    goto error;
  }

  if (!(result = get_ports(axparameter, &ports, &error))) {
    goto error;
  }

  g_hash_table_iter_init(&iter, ports);
  while (g_hash_table_iter_next(&iter, (gpointer*)&index,
        (gpointer*)&port)) {
    port_print(port);
    g_printf("\n");
  }

  loop = g_main_loop_new(NULL, FALSE);

  g_main_loop_run(loop);

error:
  if (ports != NULL) {
    g_hash_table_unref(ports);
  }
  if (axparameter != NULL) {
    ax_parameter_free(axparameter);
  }
  if (error != NULL) {
    g_message("Error: %s", error->message);
    g_error_free(error);
  }

  return result ? EXIT_SUCCESS : EXIT_FAILURE;
}
