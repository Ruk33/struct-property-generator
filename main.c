#include <assert.h> // assert
#include <stdlib.h> // malloc, free, size_t
#include <stdio.h>  // printf, fopen, fclose, ftell, fseek, SEEK_END, SEEK_SET
#include <ctype.h>  // isspace, isalpha, isdigit
#include <string.h> // strncmp

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
    // yes i know this doesn't go here
    // but it's convenient
    int is_union;
    int is_enum;
};

struct type_alias {
    struct token parent;
    struct token alias;
};

struct tokenizer {
    const char *file;
    const char *cursor;
    struct token prev;
    struct token token;
    struct type_alias aliases[256];
    size_t aliases_count;
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

static void revert_to_prev_token(struct tokenizer *tokenizer)
{
    assert(tokenizer);
    assert(tokenizer->cursor);
    tokenizer->cursor = tokenizer->prev.text;
    // prev is wrong but i don't think we really need it
    // when reverting
    // tokenizer->prev = tokenizer->token;
    tokenizer->token = get_token(tokenizer->cursor);
    tokenizer->cursor = tokenizer->token.text + tokenizer->token.len;
}

// static struct token peek_token(struct tokenizer *tokenizer)
// {
//     assert(tokenizer);
//     assert(tokenizer->cursor);
//     return get_token(tokenizer->cursor);
// }

static int consume_token_type(struct tokenizer *tokenizer, enum token_type type)
{
    assert(tokenizer);
    if (tokenizer->token.type == type) {
        consume_token(tokenizer);
        return 1;
    }
    return 0;
}

static int consume_keyword(struct tokenizer *tokenizer, const char *keyword)
{
    assert(tokenizer);
    if (tokenizer->token.type == token_identifier && token_matches(tokenizer->token, keyword)) {
        consume_token(tokenizer);
        return 1;
    }
    return 0;
}

static struct token get_original_type(struct tokenizer *tokenizer, struct token alias)
{
    assert(tokenizer);
    for (size_t i = 0; i < tokenizer->aliases_count; i++) {
        if (tokenizer->aliases[i].alias.len != alias.len)
            continue;
        if (strncmp(tokenizer->aliases[i].alias.text, alias.text, alias.len) != 0)
            continue;
        if (tokenizer->aliases[i].parent.type == token_none)
            return alias;
        return get_original_type(tokenizer, tokenizer->aliases[i].parent);
    }
    return alias;
}

static void skip_union(struct tokenizer *tokenizer);

static void skip_struct_property(struct tokenizer *tokenizer)
{
    // type
    if (tokenizer->token.type != token_identifier)
        return;
    if (token_matches(tokenizer->token, "union"))
        return;
    if (token_matches(tokenizer->token, "struct"))
        return;
    if (token_matches(tokenizer->token, "union"))
        return;
    while (tokenizer->token.type != token_colon && tokenizer->token.type != token_eof)
        consume_token(tokenizer);
    // consume ;
    consume_token_type(tokenizer, token_colon);
}

static void skip_enum(struct tokenizer *tokenizer)
{
    assert(tokenizer);
    if (!consume_keyword(tokenizer, "enum"))
        return;
    // consume name
    consume_token_type(tokenizer, token_identifier);
    // {
    if (!consume_token_type(tokenizer, token_open_brace))
        return;
    while (tokenizer->token.type != token_close_brace && tokenizer->token.type != token_eof)
        consume_token(tokenizer);
    // }
    consume_token_type(tokenizer, token_close_brace);
    // consume ;
    consume_token_type(tokenizer, token_colon);
}

static void skip_struct(struct tokenizer *tokenizer)
{
    assert(tokenizer);
    if (!consume_keyword(tokenizer, "struct"))
        return;
    // consume name
    consume_token_type(tokenizer, token_identifier);
    // {
    if (!consume_token_type(tokenizer, token_open_brace))
        return;
    while (tokenizer->token.type != token_close_brace && tokenizer->token.type != token_eof) {
        skip_union(tokenizer);
        skip_struct(tokenizer);
        skip_enum(tokenizer);
        skip_struct_property(tokenizer);
    }
    // }
    consume_token_type(tokenizer, token_close_brace);
    // consume property name
    // consume_token_type(tokenizer, token_identifier);
    // consume ;
    consume_token_type(tokenizer, token_colon);
}

static void skip_union(struct tokenizer *tokenizer)
{
    assert(tokenizer);
    if (!consume_keyword(tokenizer, "union"))
        return;
    // consume name
    consume_token_type(tokenizer, token_identifier);
    // {
    if (!consume_token_type(tokenizer, token_open_brace))
        return;
    while (tokenizer->token.type != token_close_brace && tokenizer->token.type != token_eof) {
        // consume_token(tokenizer);
        int is_union = token_matches(tokenizer->token, "union");
        int is_struct = token_matches(tokenizer->token, "struct");
        int is_enum = token_matches(tokenizer->token, "enum");
        if (is_union || is_struct ||is_enum) {
            skip_union(tokenizer);
            skip_struct(tokenizer);
            skip_enum(tokenizer);
            // skip property's name
            consume_token_type(tokenizer, token_identifier);
            // ;
            consume_token_type(tokenizer, token_colon);
        }
        skip_struct_property(tokenizer);
    }
    // }
    consume_token_type(tokenizer, token_close_brace);
    // consume property name
    // consume_token_type(tokenizer, token_identifier);
    // consume ;
    consume_token_type(tokenizer, token_colon);
}

static void parse_typedef(struct tokenizer *tokenizer)
{
    assert(tokenizer);
    if (!consume_keyword(tokenizer, "typedef"))
        return;
    // generate_properties for inline structs
    consume_keyword(tokenizer, "generate_properties");
    // union or struct
    int is_union = consume_keyword(tokenizer, "union");
    int is_struct = consume_keyword(tokenizer, "struct");
    int is_enum = consume_keyword(tokenizer, "enum");
    // todo: check if the struct has a name or if it's anon
    tokenizer->aliases[tokenizer->aliases_count].parent = tokenizer->token;
    tokenizer->aliases[tokenizer->aliases_count].parent.is_union = is_union;
    tokenizer->aliases[tokenizer->aliases_count].parent.is_enum = is_enum;    
    if (is_union || is_struct || is_enum) {
        revert_to_prev_token(tokenizer);
        skip_struct(tokenizer);
        skip_union(tokenizer);
        skip_enum(tokenizer);
    } else {
        // consume type
        consume_token_type(tokenizer, token_identifier);
    }
    // if (tokenizer->aliases[tokenizer->aliases_count].parent.type != token_identifier) {
    //     tokenizer->aliases[tokenizer->aliases_count].parent.text = tokenizer->token.text;
    //     tokenizer->aliases[tokenizer->aliases_count].parent.len = tokenizer->token.len;
    // }
    tokenizer->aliases[tokenizer->aliases_count].alias = tokenizer->token;
    tokenizer->aliases_count++;
    // typedef name
    consume_token_type(tokenizer, token_identifier);
    // ;
    // consume_token_type(tokenizer, token_colon);
}

static void parse_generate_properties(struct tokenizer *tokenizer)
{
    assert(tokenizer);
    // we only care about structs prefixed with "generate_properties"
    if (!consume_keyword(tokenizer, "generate_properties"))
        return;
    if (token_matches(tokenizer->prev, "define"))
        return;
    // consume struct keyword
    if (!consume_keyword(tokenizer, "struct"))
        return;
    printf(
        "int print_%.*s(char *dest, int n, struct %.*s *src)\n",
        tokenizer->token.len,
        tokenizer->token.text,
        tokenizer->token.len,
        tokenizer->token.text
    );
    // consume name
    consume_token_type(tokenizer, token_identifier);
    char *identation = "    ";
    printf("{\n");
    printf("%sif (!dest || !src) return 0;\n", identation);
    printf("%sint written = 0;\n", identation);
    printf("%sint tmp = 0;\n", identation);
    // consume {
    consume_token_type(tokenizer, token_open_brace);
    // consume properties
    while (tokenizer->token.type != token_close_brace && tokenizer->token.type != token_eof) {
        // skip union if detected
        if (token_matches(tokenizer->token, "union")) {
            skip_union(tokenizer);
            // skip the property's name
            consume_token_type(tokenizer, token_identifier);
            consume_token_type(tokenizer, token_colon);
        }
        // const
        consume_keyword(tokenizer, "const");
        // struct
        consume_keyword(tokenizer, "struct");
        // enum
        int is_enum = consume_keyword(tokenizer, "enum");
        // unsigned
        consume_keyword(tokenizer, "unsigned");
        // signed
        consume_keyword(tokenizer, "signed");
        struct token type = get_original_type(tokenizer, tokenizer->token);
        consume_token_type(tokenizer, token_identifier);
        // pointer *
        int is_pointer = consume_token_type(tokenizer, token_star);
        struct token name = tokenizer->token;
        consume_token_type(tokenizer, token_identifier);
        // check if it's an array
        int is_array = consume_token_type(tokenizer, token_open_brackets);
        // find colon ;
        while (tokenizer->token.type != token_colon && tokenizer->token.type != token_eof)
            consume_token(tokenizer);
        consume_token_type(tokenizer, token_colon);

        // printing code
        int is_union = type.is_union;
        if (is_union)
            printf("%s/*\n", identation);
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
        printf("%stmp = 0;\n", identation);
        // check for null pointers
        if (is_pointer)
            printf("%sif (src->%.*s%s)\n%s", identation, name.len, name.text, is_array ? "[i]" : "", identation);
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
        }
        // print int or enum or short
        else if (is_enum || type.is_enum || token_matches(type, "int") || token_matches(type, "short")) {
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
        }
        // if no c type was found, then try calling a "print_type" function
        else {
            printf("%stmp = snprintf(dest + written, n - written, \"%.*s.\");\n", identation, name.len, name.text);
            printf("%sif (tmp > 0) written += tmp;\n", identation);
            printf(
                "%stmp = print_%.*s(dest + written, n - written, %ssrc->%.*s%s);\n",
                identation,
                type.len,
                type.text,
                is_pointer ? "" : "&",
                name.len,
                name.text,
                is_array ? "[i]" : ""
            );
        }
        // check for null pointers
        if (is_pointer) {
            printf("%selse\n%s", identation, identation);
            printf(
                "%stmp = snprintf(dest + written, n - written, \"%.*s: NULL\\n\");\n",
                identation,
                name.len,
                name.text
            );
        }
        printf("%sif (tmp > 0) written += tmp;\n", identation);
        printf("%s}\n", identation);
        if (is_union)
            printf("%s*/\n", identation);
    }
    // }
    consume_token_type(tokenizer, token_close_brace);
    // don't consume it, let the main function do it.
    // ;
    // consume_token_type(tokenizer, token_colon);
    // add null terminator to buffer
    printf("%sdest[written] = 0;\n", identation);
    printf("%sreturn written;\n", identation);
    printf("}\n");
}

int main(int argc, char **argv)
{
    static struct tokenizer tokenizer = {0};

    if (argc <= 1) {
        printf("usage example:\n");
        printf("    %s file.h file.c ...\n", argv[0]);
        return 0;
    }

    // add includes
    printf("#include <stddef.h>\n");
    printf("#include <stdio.h>\n");
    for (int i = 1; i < argc; i++) {
        tokenizer.file = read_file(argv[i]);
        tokenizer.cursor = 0;
        tokenizer.token = (struct token) {0};
        tokenizer.prev = (struct token) {0};
        if (!tokenizer.file) {
            printf("// file: %s was not able to be processed.\n", argv[i]);
            continue;
        }
        while (tokenizer.token.type != token_eof) {
            consume_token(&tokenizer);
            parse_typedef(&tokenizer);
        }
        free((void *) tokenizer.file);
    }
    // for (size_t i = 0; i < tokenizer.aliases_count; i++) {
    //     printf("parent\n");
    //     print_token(tokenizer.aliases[i].parent);
    //     printf("alias\n");
    //     print_token(tokenizer.aliases[i].alias);
    //     printf("\n\n");
    // }
    for (int i = 1; i < argc; i++) {
        tokenizer.file = read_file(argv[i]);
        tokenizer.cursor = 0;
        tokenizer.token = (struct token) {0};
        tokenizer.prev = (struct token) {0};
        // reset tokenizer
        tokenizer.cursor = 0;
        while (tokenizer.token.type != token_eof) {
            consume_token(&tokenizer);
            parse_generate_properties(&tokenizer);
        }
        free((void *) tokenizer.file);
    }
    return 0;
}
