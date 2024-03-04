/*
 *    Copyright 2020 Piyush Shah <shahpiyushv@gmail.com>
 *
 *   Licensed under the Apache License, Version 2.0 (the "License");
 *   you may not use this file except in compliance with the License.
 *   You may obtain a copy of the License at
 *
 *       http://www.apache.org/licenses/LICENSE-2.0
 *
 *   Unless required by applicable law or agreed to in writing, software
 *   distributed under the License is distributed on an "AS IS" BASIS,
 *   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *   See the License for the specific language governing permissions and
 *   limitations under the License.
 */

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#include <json_generator.h>

#define MAX_INT_IN_STR  	24
#define MAX_FLOAT_IN_STR 	30
#define MEM_CHUNK_SIZE      250


static inline int json_gen_get_empty_len(json_gen_str_t *jstr)
{
	return (jstr->buf_size - (jstr->free_ptr - jstr->buf) - 1);
}


/* This will add the incoming string to the JSON string buffer and dynamically
 * resize it if the buffer is full.
 */
static int json_gen_add_to_str(json_gen_str_t *jstr, const char *str)
{
	if (!str)
		return 0;
	int len = strlen(str);
	int required_len = jstr->total_len + len + 1; // +1 for null terminator

	// Check if realloc is needed
	if (required_len > jstr->buf_size) {
		// Calculate new size, possibly with some extra space to minimize realloc calls
		int new_size = required_len + MEM_CHUNK_SIZE; // Adding extra space to minimize realloc calls
		char *new_buf = realloc(jstr->buf, new_size);
		if (!new_buf)
			return -1; // Realloc failed
		jstr->buf = new_buf;
		jstr->buf_size = new_size;
		jstr->free_ptr = jstr->buf + jstr->total_len; // Update free_ptr position
	}

	// Add the string
	strcpy(jstr->free_ptr, str);
	jstr->free_ptr += len;
	jstr->total_len += len;

	return 0;
}


void json_gen_str_start(json_gen_str_t *jstr, json_gen_flush_cb_t flush_cb,
						void *priv)
{
	memset(jstr, 0, sizeof(json_gen_str_t));

	// Initial dynamic buffer allocation
	jstr->buf = (char *)malloc(MEM_CHUNK_SIZE);
	jstr->buf_size = MEM_CHUNK_SIZE;

	jstr->flush_cb = flush_cb;
	jstr->free_ptr = jstr->buf;
	jstr->priv = priv;
}


int json_gen_str_end(json_gen_str_t *jstr)
{
	if (jstr->buf && jstr->total_len < jstr->buf_size)
		jstr->buf[jstr->total_len] = '\0'; // Ensure null termination

	// Callback to flush function
	int total_len = jstr->total_len;
	if (jstr->buf) {
		*jstr->free_ptr = '\0';
		if (jstr->flush_cb)
			jstr->flush_cb(jstr->buf, jstr->priv);
	}

	return total_len + 1; /* +1 for the NULL termination */
}


char *json_gen_get_json_string(json_gen_str_t *jstr)
{
	return jstr->buf;
}


void json_gen_free_buffer(json_gen_str_t *jstr)
{
	memset(jstr, 0, sizeof(*jstr));

	if (jstr->buf) {
		free(jstr->buf);
		jstr->buf = NULL;
	}
}

static inline void json_gen_handle_comma(json_gen_str_t *jstr)
{
	if (jstr->comma_req)
		json_gen_add_to_str(jstr, ",");
}


static int json_gen_handle_name(json_gen_str_t *jstr, const char *name)
{
	json_gen_add_to_str(jstr, "\"");
	json_gen_add_to_str(jstr, name);
	return json_gen_add_to_str(jstr, "\":");
}


int json_gen_start_object(json_gen_str_t *jstr)
{
	json_gen_handle_comma(jstr);
	jstr->comma_req = false;
	return json_gen_add_to_str(jstr, "{");
}

int json_gen_end_object(json_gen_str_t *jstr)
{
	jstr->comma_req = true;
	return json_gen_add_to_str(jstr, "}");
}


int json_gen_start_array(json_gen_str_t *jstr)
{
	json_gen_handle_comma(jstr);
	jstr->comma_req = false;
	return json_gen_add_to_str(jstr, "[");
}

int json_gen_end_array(json_gen_str_t *jstr)
{
	jstr->comma_req = true;
	return json_gen_add_to_str(jstr, "]");
}

int json_gen_push_object(json_gen_str_t *jstr, const char *name)
{
	json_gen_handle_comma(jstr);
	json_gen_handle_name(jstr, name);
	jstr->comma_req = false;
	return json_gen_add_to_str(jstr, "{");
}

int json_gen_pop_object(json_gen_str_t *jstr)
{
	jstr->comma_req = true;
	return json_gen_add_to_str(jstr, "}");
}

int json_gen_push_object_str(json_gen_str_t *jstr, const char *name, const char *object_str)
{
	json_gen_handle_comma(jstr);
	json_gen_handle_name(jstr, name);
	jstr->comma_req = true;
	return json_gen_add_to_str(jstr, object_str);
}

int json_gen_push_array(json_gen_str_t *jstr, const char *name)
{
	json_gen_handle_comma(jstr);
	json_gen_handle_name(jstr, name);
	jstr->comma_req = false;
	return json_gen_add_to_str(jstr, "[");
}
int json_gen_pop_array(json_gen_str_t *jstr)
{
	jstr->comma_req = true;
	return json_gen_add_to_str(jstr, "]");
}

int json_gen_push_array_str(json_gen_str_t *jstr, const char *name, const char *array_str)
{
	json_gen_handle_comma(jstr);
	json_gen_handle_name(jstr, name);
	jstr->comma_req = true;
	return json_gen_add_to_str(jstr, array_str);
}

static int json_gen_set_bool(json_gen_str_t *jstr, bool val)
{
	jstr->comma_req = true;
	if (val)
		return json_gen_add_to_str(jstr, "true");
	else
		return json_gen_add_to_str(jstr, "false");
}
int json_gen_obj_set_bool(json_gen_str_t *jstr, const char *name, bool val)
{
	json_gen_handle_comma(jstr);
	json_gen_handle_name(jstr, name);
	return json_gen_set_bool(jstr, val);
}

int json_gen_arr_set_bool(json_gen_str_t *jstr, bool val)
{
	json_gen_handle_comma(jstr);
	return json_gen_set_bool(jstr, val);
}

static int json_gen_set_int(json_gen_str_t *jstr, int val)
{
	jstr->comma_req = true;
	char str[MAX_INT_IN_STR];
	snprintf(str, MAX_INT_IN_STR, "%d", val);
	return json_gen_add_to_str(jstr, str);
}

int json_gen_obj_set_int(json_gen_str_t *jstr, const char *name, int val)
{
	json_gen_handle_comma(jstr);
	json_gen_handle_name(jstr, name);
	return json_gen_set_int(jstr, val);
}

int json_gen_arr_set_int(json_gen_str_t *jstr, int val)
{
	json_gen_handle_comma(jstr);
	return json_gen_set_int(jstr, val);
}

static int json_gen_set_int64(json_gen_str_t *jstr, int64_t val)
{
	jstr->comma_req = true;
	char str[MAX_INT_IN_STR];
	snprintf(str, MAX_INT_IN_STR, "%ld", val);
	return json_gen_add_to_str(jstr, str);
}

int json_gen_obj_set_int64(json_gen_str_t *jstr, const char *name, int64_t val)
{
	json_gen_handle_comma(jstr);
	json_gen_handle_name(jstr, name);
	return json_gen_set_int64(jstr, val);
}

int json_gen_arr_set_int64(json_gen_str_t *jstr, int64_t val)
{
	json_gen_handle_comma(jstr);
	return json_gen_set_int64(jstr, val);
}

static int json_gen_set_float(json_gen_str_t *jstr, float val)
{
	jstr->comma_req = true;
	char str[MAX_FLOAT_IN_STR];
	snprintf(str, MAX_FLOAT_IN_STR, "%.*f", JSON_FLOAT_PRECISION, val);
	return json_gen_add_to_str(jstr, str);
}
int json_gen_obj_set_float(json_gen_str_t *jstr, const char *name, float val)
{
	json_gen_handle_comma(jstr);
	json_gen_handle_name(jstr, name);
	return json_gen_set_float(jstr, val);
}
int json_gen_arr_set_float(json_gen_str_t *jstr, float val)
{
	json_gen_handle_comma(jstr);
	return json_gen_set_float(jstr, val);
}

static int json_gen_set_string(json_gen_str_t *jstr, const char *val)
{
	jstr->comma_req = true;
	json_gen_add_to_str(jstr, "\"");
	json_gen_add_to_str(jstr, val);
	return json_gen_add_to_str(jstr, "\"");
}

int json_gen_obj_set_string(json_gen_str_t *jstr, const char *name, const char *val)
{
	json_gen_handle_comma(jstr);
	json_gen_handle_name(jstr, name);
	return json_gen_set_string(jstr, val);
}

int json_gen_arr_set_string(json_gen_str_t *jstr, const char *val)
{
	json_gen_handle_comma(jstr);
	return json_gen_set_string(jstr, val);
}

static int json_gen_set_long_string(json_gen_str_t *jstr, const char *val)
{
	jstr->comma_req = true;
	json_gen_add_to_str(jstr, "\"");
	return json_gen_add_to_str(jstr, val);
}

int json_gen_obj_start_long_string(json_gen_str_t *jstr, const char *name, const char *val)
{
	json_gen_handle_comma(jstr);
	json_gen_handle_name(jstr, name);
	return json_gen_set_long_string(jstr, val);
}

int json_gen_arr_start_long_string(json_gen_str_t *jstr, const char *val)
{
	json_gen_handle_comma(jstr);
	return json_gen_set_long_string(jstr, val);
}

int json_gen_add_to_long_string(json_gen_str_t *jstr, const char *val)
{
	return json_gen_add_to_str(jstr, val);
}

int json_gen_end_long_string(json_gen_str_t *jstr)
{
	return json_gen_add_to_str(jstr, "\"");
}
static int json_gen_set_null(json_gen_str_t *jstr)
{
	jstr->comma_req = true;
	return json_gen_add_to_str(jstr, "null");
}
int json_gen_obj_set_null(json_gen_str_t *jstr, const char *name)
{
	json_gen_handle_comma(jstr);
	json_gen_handle_name(jstr, name);
	return json_gen_set_null(jstr);
}

int json_gen_arr_set_null(json_gen_str_t *jstr)
{
	json_gen_handle_comma(jstr);
	return json_gen_set_null(jstr);
}
