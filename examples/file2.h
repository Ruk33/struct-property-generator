#ifndef generate_properties
#define generate_properties
#endif

generate_properties struct some_other_struct {
    int bar[4];
    char *dynamic_string;
    char *null_string;
    char fixed_string[32];
    size_t size_t_property;
    signed int sint;
    unsigned int uint;
};
