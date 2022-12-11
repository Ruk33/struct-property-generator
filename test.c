#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "examples/file.h"
#include "examples/file2.h"
#include "examples/generated.c"

int main(void)
{
    struct some_other_struct s = {0};
    s.bar[0] = 2;
    s.bar[1] = 3;
    s.dynamic_string = "lorem ipsum";
    strcpy(s.fixed_string, "testing!");
    s.size_t_property = 42;
    s.sint = -21;
    s.uint = 333;
    char buf[256] = {0};
    print_some_other_struct(buf, sizeof(buf), &s);
    printf("generated:\n");
    printf("%s", buf);
    char expected[256] = {0};
    snprintf(
        expected,
        sizeof(expected),
        "bar: %d\n"
        "bar: %d\n"
        "bar: %d\n"
        "bar: %d\n"
        "dynamic_string: %s\n"
        "null_string: NULL\n"
        "fixed_string: %s\n"
        "size_t_property: %ld\n"
        "sint: %d\n"
        "uint: %d\n",
        s.bar[0],
        s.bar[1],
        s.bar[2],
        s.bar[3],
        s.dynamic_string,
        s.fixed_string,
        s.size_t_property,
        s.sint,
        s.uint
    );
    printf("--\n");
    printf("expected:\n");
    printf("%s", expected);
    assert(strcmp(buf, expected) == 0);
    printf("\n\n");
    printf("test succeed!\n");
    return 0;
}