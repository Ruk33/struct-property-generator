#ifndef generate_properties
#define generate_properties
#endif

typedef int custom_type;
typedef custom_type yet_another_custom_type;

generate_properties struct struct_as_type {
    int foo;
};

typedef struct struct_as_type st;

typedef generate_properties struct inline_struct {
    int baz;
} is;

generate_properties struct some_other_struct {
    int bar[4];
    char *dynamic_string;
    char *null_string;
    char fixed_string[32];
    size_t size_t_property;
    signed int sint;
    unsigned int uint;
    yet_another_custom_type custom_type;
    st st;
    is is;
};
