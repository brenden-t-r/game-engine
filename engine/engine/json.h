#ifndef GAMEENGINE_JSON_H
#define GAMEENGINE_JSON_H

#include "../dependencies/json.h"

#include <string>
#include <cassert>

void json_debug_print(json_object_element_s* node) {
    if (node->value->type == json_type_string) {
        printf("%s: %s\n", node->name->string, json_value_as_string(node->value)->string);
    } else if (node->value->type == json_type_number) {
        printf("%s: %s\n", node->name->string, json_value_as_number(node->value)->number);
    } else if (node->value->type == json_type_true) {
        printf("%s: true", node->name->string);
    } else if (node->value->type == json_type_false) {
        printf("%s: false", node->name->string);
    } else {
        printf("\n-%s-\n", node->name->string);
    }
}

int json_to_int(json_value_s* val) {
    assert(val->type == json_type_number);
    json_number_t* value = json_value_as_number(val);
    return std::stoi(value->number);
}

float json_to_float(json_value_s* val) {
    assert(val->type == json_type_number);
    json_number_t* value = json_value_as_number(val);
    return std::stof(value->number);
}

const char* json_to_string(json_value_s* val) {
    assert(val->type == json_type_string);
    const char* str = json_value_as_string(val)->string;
    size_t length = strlen(str);
    char* clone = new char[length + 1];
    memcpy(clone, str, length + 1); // Copy including null terminator
    return clone;
}

#endif //GAMEENGINE_JSON_H
