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
#include <string.h>
#include <json_generator.h>

static const char expected_str[] = "{\"first_bool\":true,\"first_int\":30,"\
		"\"first_int64\":-102030405060708090,\"float_val\":54.16430,"\
		"\"my_str\":\"new_name\",\"null_obj\":null,\"arr\":[[\"arr_string\","\
		"false,45.12000,null,25,908070605040302010,{\"arr_obj_str\":\"sample\""\
		"}]],\"my_obj\":{\"only_val\":5}}";

/* Creating JSON
{
	"first_bool": true,
	"first_int": 30,
	"first_int64": -102030405060708090,
	"float_val": 54.1643,
	"my_str": "new_name",
	"null_obj": null,
	"arr": [
			["arr_string", false, 45.2, null, 25, 908070605040302010, {
			 "arr_obj_str": "sample"
			 }]
			],
	"my_obj": {
		"only_val": 5
	}
}
*/

static int json_gen_perform_test(json_gen_str_t jstr, char **result,
								 const char *expected)
{
	json_gen_str_start(&jstr, NULL, NULL);
	json_gen_start_object(&jstr);
	json_gen_obj_set_bool(&jstr, "first_bool", true);
	json_gen_obj_set_int(&jstr, "first_int", 30);
	json_gen_obj_set_int64(&jstr, "first_int64", -102030405060708090);
	json_gen_obj_set_float(&jstr, "float_val", 54.1643);
	json_gen_obj_set_string(&jstr, "my_str", "new_name");
	json_gen_obj_set_null(&jstr, "null_obj");
	json_gen_push_array(&jstr, "arr");
	json_gen_start_array(&jstr);
	json_gen_arr_set_string(&jstr, "arr_string");
	json_gen_arr_set_bool(&jstr, false);
	json_gen_arr_set_float(&jstr, 45.12);
	json_gen_arr_set_null(&jstr);
	json_gen_arr_set_int(&jstr, 25);
	json_gen_arr_set_int64(&jstr, 908070605040302010);
	json_gen_start_object(&jstr);
	json_gen_obj_set_string(&jstr, "arr_obj_str", "sample");
	json_gen_end_object(&jstr);
	json_gen_end_array(&jstr);
	json_gen_pop_array(&jstr);
	json_gen_push_object(&jstr, "my_obj");
	json_gen_obj_set_int(&jstr, "only_val", 5);
	json_gen_pop_object(&jstr);
	json_gen_end_object(&jstr);
	json_gen_str_end(&jstr);
	*result = json_gen_get_json_string(&jstr);

	if (strcmp(expected, *result) == 0) {
		return 0;
	} else {
		return -1;
	}
}

int main(int argc, char **argv)
{
	json_gen_str_t jstr;
	char *result_str;
	printf("Creating JSON string [may require Line wrap enabled on console]\r\n");
	int ret = json_gen_perform_test(jstr, &result_str, expected_str);
	printf("Expected:  %s\r\n", expected_str);
	printf("Generated: %s\r\n", result_str);
	if (ret == 0) {
		printf("Test Passed!\r\n");
	} else {
		printf("Test Failed!\r\n");
	}
	json_gen_free_buffer(&jstr);
	return ret;
}
