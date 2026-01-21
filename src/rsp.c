#include <RSP/rsp.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>
#include <ctype.h>

#define TOKEN_NULL ((struct rsp_token){ .type = RSP_TT_TERMINATOR, .data = NULL })

const enum rsp_token_type rsp_right_unary_operators[] = {
    RSP_TT_ZERO_PLUS,
    RSP_TT_ONE_PLUS,
    RSP_TT_ONE_ZERO,
    RSP_TT_POSITIVE_LOOKAHEAD,
    RSP_TT_NEGATIVE_LOOKAHEAD
};

static bool rsp_token_exists(struct rsp_token token) {
    return token.type != RSP_TT_TERMINATOR;
}

static void rsp_get_token(const char ** pattern_ptr, struct rsp_token * token) {
    const char * pattern = *pattern_ptr;
    if (*pattern == '\0') {
        token->type = RSP_TT_END;
        token->data = NULL;
        return;
    }
    if (*pattern == '?') {
        token->type = RSP_TT_ONE_ZERO;
        token->data = NULL;
        (*pattern_ptr)++;
        return;
    }
    if (*pattern == '!') {
        token->type = RSP_TT_POSITIVE_LOOKAHEAD;
        token->data = NULL;
        (*pattern_ptr)++;
        return;
    }
    if (*pattern == '~') {
        token->type = RSP_TT_NEGATIVE_LOOKAHEAD;
        token->data = NULL;
        (*pattern_ptr)++;
        return;
    }
    if (*pattern == '[') {
        token->type = RSP_TT_RANGE;
        if (*(pattern + 1) == '^') {
            token->type = RSP_TT_NEG_RANGE;
            pattern++;
        }
        token->data = rsp_compile(*pattern_ptr + (token->type == RSP_TT_NEG_RANGE ? 2 : 1));
        int depth = 1;
        while (**pattern_ptr && (**pattern_ptr != ']' || depth > 0)) {
            (*pattern_ptr)++;
            if (**pattern_ptr == '[') depth++;
            if (**pattern_ptr == ']') depth--;
        }
        if (**pattern_ptr == ']') {
            (*pattern_ptr)++;
        }
        return;
    }
    if (*pattern == '(') {
        token->type = RSP_TT_GROUP;
        token->data = rsp_compile(*pattern_ptr + 1);
        int depth = 1;
        while (**pattern_ptr && (**pattern_ptr != ')' || depth > 0)) {
            (*pattern_ptr)++;
            if (**pattern_ptr == '(') depth++;
            if (**pattern_ptr == ')') depth--;
        }
        if (**pattern_ptr == ')') {
            (*pattern_ptr)++;
        }
        return;
    }
    if (*pattern == '*') {
        token->type = RSP_TT_ZERO_PLUS;
        token->data = NULL;
        (*pattern_ptr)++;
        return;
    }
    if (*pattern == '$') {
        token->type = RSP_TT_CHAR_CLASS;
        token->data = (void *)(pattern + 1);
        (*pattern_ptr) += 2;
        return;
    }
    if (*pattern == '\\') {
        token->type = RSP_TT_ESCAPE;
        token->data = (void *)(pattern + 1);
        (*pattern_ptr) += 2;
        return;
    }
    if (*pattern == '+') {
        token->type = RSP_TT_ONE_PLUS;
        token->data = NULL;
        (*pattern_ptr)++;
        return;
    }
    if (*pattern == '.') {
        token->type = RSP_TT_WILDCARD;
        token->data = NULL;
        (*pattern_ptr)++;
        return;
    }
    token->type = RSP_TT_CHAR;
    token->data = (void *)pattern;
    (*pattern_ptr)++;
}

static bool rsp_is_unary_right_operator(enum rsp_token_type type) {
    for (size_t i = 0; i < sizeof(rsp_right_unary_operators) / sizeof(enum rsp_token_type); i++) {
        if (type == rsp_right_unary_operators[i]) {
            return true;
        }
    }
    return false;
}

static bool rsp_is_node_type(enum rsp_token_type type) {
    return type == RSP_TT_RANGE || type == RSP_TT_NEG_RANGE || type == RSP_TT_GROUP || rsp_is_unary_right_operator(type);
}

static void rsp_apply_right_unary_operators(struct rsp_pattern * pattern) {
    for (size_t i = 0; rsp_token_exists(pattern->tokens[i]); i++) {
        if (rsp_is_unary_right_operator(pattern->tokens[i].type)) {
            struct rsp_token temp = pattern->tokens[i - 1];
            pattern->tokens[i - 1] = pattern->tokens[i];
            pattern->tokens[i - 1].data = malloc(sizeof(struct rsp_pattern));
            *(struct rsp_pattern *)pattern->tokens[i - 1].data = (struct rsp_pattern) { 
                .tokens = malloc(sizeof(struct rsp_token) * 2)
            };
            (*(struct rsp_pattern *)pattern->tokens[i - 1].data).tokens[0] = temp;
            (*(struct rsp_pattern *)pattern->tokens[i - 1].data).tokens[1] = TOKEN_NULL;
            // Shift left the rest
            size_t k;
            for (k = i; rsp_token_exists(pattern->tokens[k]); k++) {
                pattern->tokens[k] = pattern->tokens[k + 1];
            }
            pattern->tokens[k] = TOKEN_NULL;
        }
    }
}

static void rsp_apply_escapes(struct rsp_pattern * pattern) {
    for (size_t i = 0; rsp_token_exists(pattern->tokens[i]); i++) {
        if (pattern->tokens[i].type == RSP_TT_ESCAPE) {
            pattern->tokens[i].type = RSP_TT_CHAR;
        }
    }
}

struct rsp_pattern * rsp_compile(const char * pattern_ptr) {
    struct rsp_pattern *pattern = malloc(sizeof(struct rsp_pattern));
    pattern->tokens = NULL;
    size_t token_count = 0;
    size_t pattern_size = 1;
    while (*pattern_ptr && *pattern_ptr != ']' && *pattern_ptr != ')') {
        if (token_count + 1 >= pattern_size) pattern_size <<= 1;
        pattern->tokens = realloc(pattern->tokens, sizeof(struct rsp_token) * (pattern_size));
        rsp_get_token(&pattern_ptr, &pattern->tokens[token_count]);
        token_count++;
    }
    pattern->tokens = realloc(pattern->tokens, sizeof(struct rsp_token) * (token_count + 1));
    pattern->tokens[token_count] = TOKEN_NULL;
    rsp_apply_escapes(pattern);
    rsp_apply_right_unary_operators(pattern);
    return pattern;
}

void rsp_free(struct rsp_pattern *pattern) {
    for (size_t i = 0; rsp_token_exists(pattern->tokens[i]); i++) {
        struct rsp_token token = pattern->tokens[i];
        if (rsp_is_node_type(token.type)) {
            rsp_free((struct rsp_pattern *)token.data);
            free(token.data);
        }
    }
    free(pattern->tokens);
    pattern->tokens = NULL;
}

void rsp_print(struct rsp_pattern *pattern) {
    for (size_t i = 0; rsp_token_exists(pattern->tokens[i]); i++) {
        struct rsp_token token = pattern->tokens[i];
        switch (token.type) {
            case RSP_TT_CHAR:
                printf("CHAR(%c) ", *(char *)token.data);
                break;
            case RSP_TT_WILDCARD:
                printf("WILDCARD ");
                break;
            case RSP_TT_ZERO_PLUS:
                printf("ZERO_PLUS ( ");
                if (token.data) rsp_print((struct rsp_pattern *)token.data);
                printf(") ");
                break;
            case RSP_TT_ONE_PLUS:
                printf("ONE_PLUS ( ");
                if (token.data) rsp_print((struct rsp_pattern *)token.data);
                printf(") ");
                break;
            case RSP_TT_ONE_ZERO:
                printf("ONE_ZERO ( ");
                if (token.data) rsp_print((struct rsp_pattern *)token.data);
                printf(") ");
                break;
            case RSP_TT_POSITIVE_LOOKAHEAD:
                printf("POSITIVE_LOOKAHEAD ( ");
                if (token.data) rsp_print((struct rsp_pattern *)token.data);
                printf(") ");
                break;
            case RSP_TT_NEGATIVE_LOOKAHEAD:
                printf("NEGATIVE_LOOKAHEAD ( ");
                if (token.data) rsp_print((struct rsp_pattern *)token.data);
                printf(") ");
                break;
            case RSP_TT_CHAR_CLASS:
                printf("CHAR_CLASS(%c) ", *(char *)token.data);
                break;
            case RSP_TT_RANGE:
                printf("RANGE[ ");
                if (token.data) rsp_print((struct rsp_pattern *)token.data);
                printf("] ");
                break;
            case RSP_TT_NEG_RANGE:
                printf("NEG_RANGE[ ");
                if (token.data) rsp_print((struct rsp_pattern *)token.data);
                printf("] ");
                break;
            case RSP_TT_GROUP:
                printf("GROUP( ");
                if (token.data) rsp_print((struct rsp_pattern *)token.data);
                printf(") ");
                break;
            case RSP_TT_ESCAPE:
                printf("ESCAPE(%c) ", *(char *)token.data);
                break;
            case RSP_TT_END:
                printf("END ");
                break;
            case RSP_TT_TERMINATOR:
                printf("TERMINATOR ");
                break;
            default:
                printf("UNKNOWN ");
                break;
        }
    }
}

static bool rsp_match_char_class(char c, const char * class_ptr) {
    switch (*class_ptr) {
        case 'a':
            return isalpha((unsigned char)c);
        case 'd':
            return isdigit((unsigned char)c);
        case 'w':
            return isalnum((unsigned char)c) || c == '_';
        case '_':
            return c == '_';
        default:
            return c == *class_ptr;
    }
}

enum rsp_pattern_match_result {
    RSP_PMR_NO_MATCH,
    RSP_PMR_MATCH,
    RSP_PMR_INDETERMINATE
};

static enum rsp_pattern_match_result rsp_match_token(const char ** str_ptr, struct rsp_token * token, size_t repeat_count) {
    const char * str = *str_ptr;
    if (rsp_token_exists(*token) == false) {
        return RSP_PMR_NO_MATCH;
    }
    switch (token->type) {
        case RSP_TT_CHAR:
            if (*str == *(char *)token->data) {
                (*str_ptr)++;
                return RSP_PMR_MATCH;
            }
            break;
        case RSP_TT_WILDCARD:
            if (*str != '\0') {
                (*str_ptr)++;
                return RSP_PMR_MATCH;
            }
            break;
        case RSP_TT_CHAR_CLASS:
            if (rsp_match_char_class(*str, (const char *)token->data)) {
                (*str_ptr)++;
                return RSP_PMR_MATCH;
            }
            break;
        case RSP_TT_RANGE:
        case RSP_TT_NEG_RANGE: {
            bool match = false;
            for (size_t i = 0; rsp_token_exists(((struct rsp_pattern *)token->data)->tokens[i]); i++) {
                struct rsp_token *range_token = &((struct rsp_pattern *)token->data)->tokens[i];
                if (i > 0 && range_token->type == RSP_TT_CHAR && *(char *)range_token->data == '-' && 
                    ((struct rsp_pattern *)token->data)->tokens[i + 1].type == RSP_TT_CHAR) {
                    char start = *((char *)((struct rsp_pattern *)token->data)->tokens[i - 1].data);
                    char end = *((char *)((struct rsp_pattern *)token->data)->tokens[i + 1].data);
                    if (*str >= start && *str <= end) {
                        match = true;
                        break;
                    } else {
                        match = false;
                        continue;
                    }
                } else {
                    enum rsp_pattern_match_result result = rsp_match_token(&str, range_token, 0);
                    if (result == RSP_PMR_MATCH) {
                        match = true;
                        break;
                    } else {
                        match = false;
                    }
                }
            }
            if ((token->type == RSP_TT_RANGE && match) || (token->type == RSP_TT_NEG_RANGE && !match)) {
                (*str_ptr)++;
                return RSP_PMR_MATCH;
            }
            break;
        }
        case RSP_TT_GROUP: {
            struct rsp_pattern sub_pattern = *(struct rsp_pattern *)token->data;
            const char * current_str = str;
            repeat_count = 0;
            for (size_t i = 0; rsp_token_exists(sub_pattern.tokens[i]); i++) {
                enum rsp_pattern_match_result result = rsp_match_token(&current_str, &sub_pattern.tokens[i], repeat_count);
                switch (result) {
                    case RSP_PMR_NO_MATCH:
                        return RSP_PMR_NO_MATCH;
                    case RSP_PMR_INDETERMINATE:
                        repeat_count++;
                        i--;
                        break;
                    case RSP_PMR_MATCH:
                        repeat_count = 0;
                        break;
                }
            }
            *str_ptr = current_str;
            return RSP_PMR_MATCH;
        }
        case RSP_TT_ZERO_PLUS:
        case RSP_TT_ONE_PLUS: {
            struct rsp_pattern sub_pattern = *(struct rsp_pattern *)token->data;
            const char * current_str = str;
            if ((token->type == RSP_TT_ONE_PLUS && repeat_count >= 1) || (token->type == RSP_TT_ZERO_PLUS)) {
                if (*current_str == '\0') {
                    return RSP_PMR_MATCH;
                }
                if (rsp_match_token(&current_str, token + 1, repeat_count) != RSP_PMR_NO_MATCH) {
                    return RSP_PMR_MATCH;
                }
            }
            for (size_t i = 0; rsp_token_exists(sub_pattern.tokens[i]); i++) {
                if (rsp_match_token(&str, &sub_pattern.tokens[i], repeat_count) == RSP_PMR_NO_MATCH) {
                    return RSP_PMR_NO_MATCH;
                }
            }
            (*str_ptr) = str;
            return RSP_PMR_INDETERMINATE;
        }
        case RSP_TT_ONE_ZERO: {
            struct rsp_pattern sub_pattern = *(struct rsp_pattern *)token->data;
            bool matched = true;
            for (size_t i = 0; rsp_token_exists(sub_pattern.tokens[i]); i++) {
                if (rsp_match_token(&str, &sub_pattern.tokens[i], repeat_count) == RSP_PMR_NO_MATCH) {
                    matched = false;
                    break;
                }
            }
            if (matched) {
                (*str_ptr) = str;
                return RSP_PMR_MATCH;
            }
            return RSP_PMR_MATCH;
        }
        case RSP_TT_POSITIVE_LOOKAHEAD:
        case RSP_TT_NEGATIVE_LOOKAHEAD: {
            struct rsp_pattern sub_pattern = *(struct rsp_pattern *)token->data;
            for (size_t i = 0; rsp_token_exists(sub_pattern.tokens[i]); i++) {
                if ((rsp_match_token(&str, &sub_pattern.tokens[i], repeat_count) == RSP_PMR_NO_MATCH) == (token->type == RSP_TT_NEGATIVE_LOOKAHEAD)) {
                    return RSP_PMR_MATCH;
                }
            }
            break;
        }
        default:
            printf("Unknown token type %d\n", token->type);
            break;
    }
    return RSP_PMR_NO_MATCH;
}

static const char * _rsp_match(const char * str, const struct rsp_pattern *pattern) {
    size_t repeat_count = 0;
    for (size_t i = 0; rsp_token_exists(pattern->tokens[i]); i++) {
        enum rsp_pattern_match_result result = rsp_match_token(&str, &pattern->tokens[i], repeat_count);
        switch (result) {
            case RSP_PMR_NO_MATCH:
                return NULL;
            case RSP_PMR_INDETERMINATE:
                repeat_count++;
                i--;
                break;
            case RSP_PMR_MATCH:
                repeat_count = 0;
                break;
        }
    }
    return str;
}

const char * rsp_match(const char * str, struct rsp_pattern *pattern) {
    const char * result = _rsp_match(str, pattern);
    return result;
}

const char * rsp_compile_and_match(const char * str, const char * pattern) {
    struct rsp_pattern * pat = rsp_compile(pattern);
    const char * result = _rsp_match(str, pat);
    rsp_free(pat);
    return result;
}