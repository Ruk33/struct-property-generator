#include <stddef.h>
#include <stdio.h>
int print_foo(char *dest, int n, struct foo *src)
{
    if (!dest || !src) return 0;
    int written = 0;
    int tmp = 0;
    {
    tmp = snprintf(dest + written, n - written, "bar: %d\n", src->bar);
    if (tmp > 0) written += tmp;
    }
    {
    tmp = snprintf(dest + written, n - written, "baz: %s\n", src->baz);
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
    tmp = snprintf(dest + written, n - written, "bar: %d\n", src->bar[i]);
    if (tmp > 0) written += tmp;
    }
    {
    tmp = snprintf(dest + written, n - written, "dynamic_string: %s\n", src->dynamic_string);
    if (tmp > 0) written += tmp;
    }
    {
    tmp = snprintf(dest + written, n - written, "fixed_string: %s\n", src->fixed_string);
    if (tmp > 0) written += tmp;
    }
    {
    tmp = snprintf(dest + written, n - written, "size_t_property: %ld\n", src->size_t_property);
    if (tmp > 0) written += tmp;
    }
    {
    tmp = snprintf(dest + written, n - written, "sint: %d\n", src->sint);
    if (tmp > 0) written += tmp;
    }
    {
    tmp = snprintf(dest + written, n - written, "uint: %d\n", src->uint);
    if (tmp > 0) written += tmp;
    }
    dest[written] = 0;
    return written;
}
