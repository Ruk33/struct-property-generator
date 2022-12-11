#include <stddef.h>
#include <stdio.h>
int print_foo(char *dest, int n, struct foo *src)
{
    if (!dest || !src) return 0;
    int written = 0;
    int tmp = 0;
    {
    tmp = 0;
    tmp = snprintf(dest + written, n - written, "bar: %d\n", src->bar);
    if (tmp > 0) written += tmp;
    }
    {
    tmp = 0;
    if (src->baz)
        tmp = snprintf(dest + written, n - written, "baz: %s\n", src->baz);
    else
        tmp = snprintf(dest + written, n - written, "baz: NULL\n");
    if (tmp > 0) written += tmp;
    }
    dest[written] = 0;
    return written;
}
int print_struct_as_type(char *dest, int n, struct struct_as_type *src)
{
    if (!dest || !src) return 0;
    int written = 0;
    int tmp = 0;
    {
    tmp = 0;
    tmp = snprintf(dest + written, n - written, "foo: %d\n", src->foo);
    if (tmp > 0) written += tmp;
    }
    dest[written] = 0;
    return written;
}
int print_inline_struct(char *dest, int n, struct inline_struct *src)
{
    if (!dest || !src) return 0;
    int written = 0;
    int tmp = 0;
    {
    tmp = 0;
    tmp = snprintf(dest + written, n - written, "baz: %d\n", src->baz);
    if (tmp > 0) written += tmp;
    }
    dest[written] = 0;
    return written;
}
int print_some_other_struct(char *dest, int n, struct some_other_struct *src)
{
    if (!dest || !src) return 0;
    int written = 0;
    int tmp = 0;
    for (size_t i = 0; i < sizeof(src->bar) / sizeof(*(src->bar)); i++)
    {
    tmp = 0;
    tmp = snprintf(dest + written, n - written, "bar: %d\n", src->bar[i]);
    if (tmp > 0) written += tmp;
    }
    {
    tmp = 0;
    if (src->dynamic_string)
        tmp = snprintf(dest + written, n - written, "dynamic_string: %s\n", src->dynamic_string);
    else
        tmp = snprintf(dest + written, n - written, "dynamic_string: NULL\n");
    if (tmp > 0) written += tmp;
    }
    {
    tmp = 0;
    if (src->null_string)
        tmp = snprintf(dest + written, n - written, "null_string: %s\n", src->null_string);
    else
        tmp = snprintf(dest + written, n - written, "null_string: NULL\n");
    if (tmp > 0) written += tmp;
    }
    {
    tmp = 0;
    if (src->fixed_string)
        tmp = snprintf(dest + written, n - written, "fixed_string: %s\n", src->fixed_string);
    else
        tmp = snprintf(dest + written, n - written, "fixed_string: NULL\n");
    if (tmp > 0) written += tmp;
    }
    {
    tmp = 0;
    tmp = snprintf(dest + written, n - written, "size_t_property: %ld\n", src->size_t_property);
    if (tmp > 0) written += tmp;
    }
    {
    tmp = 0;
    tmp = snprintf(dest + written, n - written, "sint: %d\n", src->sint);
    if (tmp > 0) written += tmp;
    }
    {
    tmp = 0;
    tmp = snprintf(dest + written, n - written, "uint: %d\n", src->uint);
    if (tmp > 0) written += tmp;
    }
    {
    tmp = 0;
    tmp = snprintf(dest + written, n - written, "custom_type: %d\n", src->custom_type);
    if (tmp > 0) written += tmp;
    }
    {
    tmp = 0;
    tmp = snprintf(dest + written, n - written, "st.");
    if (tmp > 0) written += tmp;
    tmp = print_struct_as_type(dest + written, n - written, &src->st);
    if (tmp > 0) written += tmp;
    }
    {
    tmp = 0;
    tmp = snprintf(dest + written, n - written, "is.");
    if (tmp > 0) written += tmp;
    tmp = print_inline_struct(dest + written, n - written, &src->is);
    if (tmp > 0) written += tmp;
    }
    /*
    {
    tmp = 0;
    tmp = snprintf(dest + written, n - written, "ignore_me_again.");
    if (tmp > 0) written += tmp;
    tmp = print_{(dest + written, n - written, &src->ignore_me_again);
    if (tmp > 0) written += tmp;
    }
    */
    {
    tmp = 0;
    tmp = snprintf(dest + written, n - written, "another_property: %d\n", src->another_property);
    if (tmp > 0) written += tmp;
    }
    dest[written] = 0;
    return written;
}
