/**
 * \file ioinfo.h
 *
 * \brief The is an example illustrating how to read I/O port configuration
 * from the device.
 */

#ifndef _IOINFO_H_
#define _IOINFO_H_

/**
 * Determines the I/O direction of a port
 */
typedef enum _direction_t {
  DIRECTION_INPUT,
  DIRECTION_OUTPUT
} direction_t;

/**
 * Determines when to trig
 */
typedef enum _trig_t {
  TRIG_OPEN,
  TRIG_CLOSED
} trig_t;

/**
 * The button type associated with an output port
 */
typedef enum _button_t {
  BUTTON_NONE,
  BUTTON_PULSE,
  BUTTON_ACTINACT
} button_t;

/**
 * The active state of an output port
 */
typedef enum _active_t {
  ACTIVE_OPEN,
  ACTIVE_CLOSED
} active_t;

/**
 * The base type of an input or output port
 */
typedef struct _port_t {
  /** The parameter index of the port */
  guint index;
  /** The direction of the port */
  direction_t direction;
} port_t;

/**
 * Describes an input port
 */
typedef struct _input_port_t {
  /** The base port */
  port_t port;
  /** The name of the input port */
  gchar *name;
  /** The trig state of the port */
  trig_t trig;
} input_port_t;

/**
 * Describes an output port
 */
typedef struct _output_port_t {
  /** The base port */
  port_t port;
  /** The name of the output port */
  gchar *name;
  /** The active state of the port */
  active_t active;
  /** The button state of the port */
  button_t button;
  /** The pulse time of the port */
  guint pulse_time;
} output_port_t;

/**
 * \brief Create a new base port data structure
 *
 * \brief index the parameter index of the port
 * \brief direction the direction of the port
 * \return a newly allocated base port
 */
port_t*
port_new(guint index,
    direction_t direction);

/**
 * Print a representation of the port to stdout
 *
 * \param port the port
 */
void
port_print(port_t *port);

/**
 * \brief Free resources associated with a base port
 *
 * \param port the base port
 */
void
port_free(port_t *port);

/**
 * \brief Create a new input port data structure
 *
 * \param index the parameter index of the port
 * \param name the name of the port
 * \param trig the trig state of the port
 * \return a newly allocated input port
 */
input_port_t*
input_port_new(guint index,
    const gchar *name,
    trig_t trig);

/**
 * \brief Print a representation of an input port to stdout
 *
 * \brief input_port the input port
 */
void
input_port_print(input_port_t *input_port);

/**
 * \brief Free resource associated with an input port data structure
 *
 * \param input_port the input port
 */
void
input_port_free(input_port_t *input_port);

/**
 * \brief Create a new output port data structure
 *
 * \param index the parameter index of the port
 * \param name the name of the port
 * \param active the active state of the port
 * \param button the button state of the port
 * \param pulse_time the pulse time of the port
 * \return a newly allocated output port
 */
output_port_t*
output_port_new(guint index,
    const gchar *name,
    active_t active,
    button_t button,
    guint pulse_time);

/**
 * \brief Print a representation of an output port to stdout
 *
 * \param output_port the output port
 */
void
output_port_print(output_port_t *output_port);

/**
 * \brief Free resources associated with an input port data structure
 *
 * \param output_port the output port
 */
void
output_port_free(output_port_t *output_port);

/**
 * \brief Get the integer representation of a parameter
 *
 * \param axparameter a reference to the axparameter library
 * \param param the parameter
 * \param value a pointer to a location to store the value in
 * \param error a pointer to a location to store an error in or NULL
 * \return TRUE if successful otherwise FALSE in which case error will be set
 */
gboolean
get_integer(AXParameter *axparameter,
    const gchar *param,
    guint *value,
    GError **error);
/**
 * \brief Get the trig state of an input port
 *
 * \param axparameter a reference to the axparameter library
 * \param param the name of the parameter 
 * \param trig a location to store the trig state in
 * \param error a pointer to a location to store an error in or NULL
 * \return TRUE if successful otherwise FALSE in which case error will be set
 */

gboolean
get_trig(AXParameter *axparameter,
    const gchar *param,
    trig_t *trig,
    GError **error);

/**
 * \brief Get the string representation of a trig
 *
 * \param trig the trig state
 * \return the string representation of trig
 */
const gchar*
trig_to_string(trig_t trig);

/**
 * \brief Convert a string to a trig
 *
 * \param value the string
 * \return the trig
 */
trig_t
string_to_trig(const gchar *value);

/**
 * \brief Get the direction of a port
 *
 * \param axparameter a reference to the axparameter library
 * \param index the index of the port
 * \param direction a location to store the direction in
 * \param error a pointer to a location to store an error in or NULL
 * \return TRUE if successful otherwise FALSE in which case error will be set
 */
gboolean
get_direction(AXParameter *axparameter,
    guint index,
    direction_t *direction,
    GError **error);

/**
 * \brief Get the button state of an output port
 *
 * \param param the parameter holding th button
 * \param button a location to store the button state in
 * \param axparameter a reference to the axparameter library
 * \param error a pointer to a location to store an error in or NULL
 * \return TRUE if successful otherwise FALSE in which case error will be set
 */
gboolean
get_button(AXParameter *axparameter,
    const gchar *param,
    button_t *button,
    GError **error);

/**
 * \brief get the string representation of a button
 *
 * \param button the button
 * \return the string representation of button
 */
const gchar*
button_to_string(button_t button);

/**
 * \brief Convert a string to a button
 *
 * \param value the string value
 * \return the button
 */
button_t
string_to_button(const gchar *value);

/**
 * \brief Get the active state of an output port
 *
 * \param param the parameter holding the active state
 * \param active a location to store the active state in
 * \param axparameter a reference to the axparameter library
 * \param error a pointer to a location to store an error in or NULL
 * \return TRUE if successful otherwise FALSE in which case error will be set
 */
gboolean
get_active(AXParameter *axparameter,
    const gchar *param,
    active_t *active,
    GError **error);

/**
 * \brief Get the string representation of an active state
 *
 * \param active the active state
 * \return the string representation of the active state
 */
const gchar*
active_to_string(active_t active);

/**
 * \brief Convert a string to an active state
 *
 * \param value the string value
 * \return the active state
 */
active_t
string_to_active(const gchar *value);

/**
 * \brief Get the state of an input port
 *
 * \param axparameter a reference to the axparameter library
 * \param index the index of the input port
 * \param input_port a location to store the input port state in. If successful
 * a new input port will be created and it is the responsibility of the caller
 * to free the newly created input_port.
 * \param error a pointer to a location to store an error in or NULL
 * \return TRUE if successful otherwise FALSE in which case error will be set
 */
gboolean
get_input_port_settings(AXParameter *axparameter,
    guint index,
    input_port_t **input_port,
    GError **error);

/**
 * \brief Get the state of an output port
 *
 * \param axparameter a reference to the axparameter library
 * \param index the index of the output port
 * \param output_port a location to store the output port state in. If
 * successful a new output port will be created and it is the responsibility of
 * the caller to free the newly created output_port.
 * \param error a pointer to a location to store an error in or NULL
 * \return TRUE if successful otherwise FALSE in which case error will be set
 */
gboolean
get_output_port_settings(AXParameter *axparameter,
    guint index,
    output_port_t **output_port,
    GError **error);

/**
 * \brief Get the state of a port
 *
 * \param axparameter a reference to the axparameter library
 * \param index the index of the port
 * \param port a return location to store the port state in.
 * \param error the return location of an error or NULL
 * \return TRUE if successful otherwise FALSE
 */
gboolean
get_port_settings(AXParameter *axparameter,
    guint index,
    port_t **port,
    GError **error);

/**
 * \brief Get the state of all ports
 *
 * \param axparameter a reference to the axparameter library
 * \param ports a location to store all ports in. If successfull
 * a new hash table will be crated and it is the responsiblity the caller to
 * free the hash table. Freeing the hash table will implicitly also free all
 * ports held by the hash table.
 * \param error a pointer to a location to store an error in or NULL
 * \return TRUE if successful otherwise FALSE in which case error will be set
 */
gboolean
get_ports(AXParameter *axparameter,
    GHashTable **ports,
    GError **error);

#endif /* _IOINFO_H_ */
