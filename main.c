#include <assert.h> // assert
#include <stdlib.h> // malloc, free, size_t
#include <stdio.h>  // printf, fopen, fclose, ftell, fseek, SEEK_END, SEEK_SET
#include <ctype.h>  // isspace, isalpha, isdigit

enum token_type {
    token_none,
    token_pound,
    token_identifier,
    token_number,
    token_open_parenthesis,
    token_close_parenthesis,
    token_open_brace,
    token_close_brace,
    token_open_brackets,
    token_close_brackets,
    token_keyword,
    token_colon,
    token_less_than,
    token_greather_than,
    token_file_path,
    token_string,
    token_star,
    token_unknown,
    token_eof,
};

struct token {
    const char *text;
    size_t len;
    enum token_type type;
};

struct tokenizer {
    const char *file;
    const char *cursor;
    struct token prev;
    struct token token;
};

static char *read_file(const char *file_path)
{
    assert(file_path);
    FILE *file = fopen(file_path, "r");
    if (!file)
        return 0;
    size_t prev = ftell(file);
    fseek(file, 0, SEEK_END);
    size_t size = ftell(file);
    fseek(file, prev, SEEK_SET);
    char *result = malloc(size + 1);
    if (!result) {
        fclose(file);
        return 0;
    }
    fread(result, size, 1, file);
    // null terminator
    result[size] = 0;
    fclose(file);
    return result;
}

static const char *skip_whitespace(const char *src)
{
    assert(src);
    while (*src) {
        if (isspace(*src)) {
            src++;
            continue;
        }
        // skip // comments
        if (*src == '/' && *(src + 1) == '/') {
            src++;
            src++;
            while (*src && *src != '\n')
                src++;
            continue;
        }
        // skip /* comments
        if (*src == '/' && *(src + 1) == '*') {
            src++;
            src++;
            // search end of comment
            while (!(*src == '*' && *(src + 1) == '/'))
                src++;
            if (*src == '*' && *(src + 1) == '/') {
                src++;
                src++;
            }
            continue;
        }
        break;
    }
    return src;
}

// just for debug.
static void print_token(struct token token)
{
    printf(
        "token (len: %d, type: %d): '%.*s'\n",
        token.len,
        token.type,
        token.len,
        token.text
    );
}

static struct token get_token(const char *src)
{
    assert(src);
    src = skip_whitespace(src);

    struct token token = {0};
    token.text = src;
    token.len = 1;

    switch (*src) {
    case 0:
    case EOF:
        token.len = 0;
        token.type = token_eof;
        break;
    case '[':
        token.type = token_open_brackets;
        break;
    case ']':
        token.type = token_close_brackets;
        break;
    case '*':
        token.type = token_star;
        break;
    case '<':
        token.type = token_less_than;
        break;
    case '>':
        token.type = token_greather_than;
        break;
    case '#':
        token.type = token_pound;
        break;
    case '{':
        token.type = token_open_brace;
        break;
    case '}':
        token.type = token_close_brace;
        break;
    case '(':
        token.type = token_open_parenthesis;
        break;
    case ')':
        token.type = token_close_parenthesis;
        break;
    case ';':
        token.type = token_colon;
        break;
    case '"': {
        const char *start = src;
        token.type = token_string;
        src++;
        while (*src != '"')
            src++;
        src++;
        token.len = src - start;
    } break;
    default: {
        const char *start = src;
        if (isdigit(*src)) {
            token.type = token_number;
            do {
                src++;
                if (*src == '.' || *src == 'f')
                    src++;
            } while (isdigit(*src));
        } else if (isalpha(*src)) {
            token.type = token_identifier;
            do {
                src++;
                if (*src == '_')
                    src++;
                // this is not really required but...
                if (*src == '.') {
                    token.type = token_file_path;
                    src++;
                }
                // identifiers can contain digits in their names.
                while (isdigit(*src))
                    src++;
            } while (isalpha(*src));
        } else {
            src++;
            token.type = token_unknown;
        }
        token.len = src - start;
    } break;
    }
    return token;
}

static int token_matches(struct token token, const char *match)
{
    assert(match);
    size_t match_len = 0;
    for (size_t i = 0; i < token.len; i++) {
        if (token.text[i] != *match)
            return 0;
        match++;
        match_len++;
    }
    return match_len == token.len && *match == 0;
}

static void consume_token(struct tokenizer *tokenizer)
{
    assert(tokenizer);
    assert(tokenizer->file);
    if (!tokenizer->cursor)
        tokenizer->cursor = tokenizer->file;
    if (tokenizer->token.type == token_eof)
        return;
    tokenizer->prev = tokenizer->token;
    tokenizer->token = get_token(tokenizer->cursor);
    tokenizer->cursor = tokenizer->token.text + tokenizer->token.len;
}

static void parse_generate_properties(struct tokenizer *tokenizer)
{
    assert(tokenizer);
    // we only care about structs prefixed with "generate_properties"
    if (tokenizer->token.type != token_identifier)
        return;
    if (!token_matches(tokenizer->token, "generate_properties"))
        return;
    if (token_matches(tokenizer->prev, "define"))
        return;

    // consume struct keyword
    consume_token(tokenizer);
    // consume name
    consume_token(tokenizer);
    printf(
        "int print_%.*s(char *dest, int n, struct %.*s *src)\n",
        tokenizer->token.len,
        tokenizer->token.text,
        tokenizer->token.len,
        tokenizer->token.text
    );
    char *identation = "    ";
    printf("{\n");
    printf("%sif (!dest || !src) return 0;\n", identation);
    printf("%sint written = 0;\n", identation);
    printf("%sint tmp = 0;\n", identation);
    // consume {
    consume_token(tokenizer);
    // consume properties
    while (1) {
        // type, const, enum or struct
        consume_token(tokenizer);
        // print_token(tokenizer->token);
        if (tokenizer->token.type != token_identifier)
            break;
        // const
        if (tokenizer->token.type == token_identifier && token_matches(tokenizer->token, "const"))
            consume_token(tokenizer);
        // struct
        if (tokenizer->token.type == token_identifier && token_matches(tokenizer->token, "struct"))
            consume_token(tokenizer);
        // enum
        int is_enum = tokenizer->token.type == token_identifier && token_matches(tokenizer->token, "enum");
        if (is_enum)
            consume_token(tokenizer);
        // unsigned
        if (tokenizer->token.type == token_identifier && token_matches(tokenizer->token, "unsigned"))
            consume_token(tokenizer);
        // signed
        if (tokenizer->token.type == token_identifier && token_matches(tokenizer->token, "signed"))
            consume_token(tokenizer);
        struct token type = tokenizer->token;
        // print_token(tokenizer->token);
        consume_token(tokenizer);
        // pointer *
        int is_pointer = tokenizer->token.type == token_star;
        if (is_pointer)
            consume_token(tokenizer);
        struct token name = tokenizer->token;
        // print_token(tokenizer->token);

        // check if it's an array
        consume_token(tokenizer);
        int is_array = tokenizer->token.type == token_open_brackets;
        if (is_array) {
            // find colon ;
            while (tokenizer->token.type != token_colon)
                consume_token(tokenizer);
            // todo: this is a bit weird, for some reason
            // it's not required to consume the colon token.
            // 
            // consume ;
            // consume_token(tokenizer);
        }

        // todo:
        // make sure pointers aren't null.

        // special case for fixed size strings
        if (!is_pointer && is_array && token_matches(type, "char")) {
            is_array = 0;
            is_pointer = 1;
        }

        // array
        if (is_array)
            printf(
                "%sfor (size_t i = 0; i < sizeof(src->%.*s) / sizeof(*(src->%.*s)); i++)\n",
                identation,
                name.len,
                name.text,
                name.len,
                name.text
            );

        printf("%s{\n", identation);

        // print char
        if (token_matches(type, "char")) {
            printf(
                "%stmp = snprintf(dest + written, n - written, \"%.*s: %%%s\\n\", src->%.*s%s);\n",
                identation,
                name.len,
                name.text,
                is_pointer ? "s" : "c",
                name.len,
                name.text,
                is_array ? "[i]" : ""
            );
            printf("%sif (tmp > 0) written += tmp;\n", identation);
        }
        // print int or enum or short
        else if (is_enum || token_matches(type, "int") || token_matches(type, "short")) {
            printf(
                "%stmp = snprintf(dest + written, n - written, \"%.*s: %%d\\n\", %ssrc->%.*s%s);\n",
                identation,
                name.len,
                name.text,
                is_pointer ? "*" : "",
                name.len,
                name.text,
                is_array ? "[i]" : ""
            );
            printf("%sif (tmp > 0) written += tmp;\n", identation);
        }
        // print float or double
        else if (token_matches(type, "float") || token_matches(type, "double")) {
            printf(
                "%stmp = snprintf(dest + written, n - written, \"%.*s: %%f\\n\", %ssrc->%.*s%s);\n",
                identation,
                name.len,
                name.text,
                is_pointer ? "*" : "",
                name.len,
                name.text,
                is_array ? "[i]" : ""
            );
            printf("%sif (tmp > 0) written += tmp;\n", identation);
        }
        // print size_t or long
        else if (token_matches(type, "size_t") || token_matches(type, "long")) {
            printf(
                "%stmp = snprintf(dest + written, n - written, \"%.*s: %%ld\\n\", %ssrc->%.*s%s);\n",
                identation,
                name.len,
                name.text,
                is_pointer ? "*" : "",
                name.len,
                name.text,
                is_array ? "[i]" : ""
            );
            printf("%sif (tmp > 0) written += tmp;\n", identation);
        }
        // if no c type was found, then try calling a "print_type" function
        else {
            printf(
                "%swritten += print_%.*s(dest + written, n - written, %ssrc->%.*s%s);\n",
                identation,
                type.len,
                type.text,
                is_pointer ? "" : "&",
                name.len,
                name.text,
                is_array ? "[i]" : ""
            );
        }
        printf("%s}\n", identation);
    }
    // add null terminator to buffer
    printf("%sdest[written] = 0;\n", identation);
    printf("%sreturn written;\n", identation);
    printf("}\n");
}

int main(int argc, char **argv)
{
    if (argc <= 1) {
        printf("usage example:\n");
        printf("    %s file.h file.c ...\n", argv[0]);
        return 0;
    }

    // add includes
    printf("#include <stddef.h>\n");
    printf("#include <stdio.h>\n");
    for (int i = 1; i < argc; i++) {
        struct tokenizer tokenizer = {0};
        tokenizer.file = read_file(argv[i]);
        if (!tokenizer.file) {
            printf("// file: %s was not able to be processed.\n", argv[i]);
            continue;
        }
        while (tokenizer.token.type != token_eof) {
            consume_token(&tokenizer);
            parse_generate_properties(&tokenizer);
        }
        free((void *) tokenizer.file);
    }
    return 0;
}
