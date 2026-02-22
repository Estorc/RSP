#include <RSP/rsp.h>
#include <stdlib.h>
#include <assert.h>
#include <stdio.h>

#define OK(s,p)  do{ \
    const char *r = rsp_compile_and_match(s,p); \
    printf("[OK]  \"%s\" =~ \"%s\" -> %s\n", s,p, r?"MATCH":"FAIL"); \
    printf("Result: %s\n\n", r?r:"<null>"); \
}while(0)

#define MUST_MATCH(s,p)  do{ \
    const char *r = rsp_compile_and_match(s,p); \
    assert(r && "Expected MATCH but got FAIL"); \
    OK(s,p); \
}while(0)

#define MUST_FAIL(s,p)  do{ \
    const char *r = rsp_compile_and_match(s,p); \
    assert(!r && "Expected FAIL but got MATCH"); \
    OK(s,p); \
}while(0)

int main() {

    MUST_MATCH("abc", "abc");
    MUST_FAIL("abc", "abd");

    MUST_MATCH("aaaa", "a*");
    MUST_MATCH("abbbb", "ab*");
    MUST_FAIL("abbbb", "ab*c");

    MUST_MATCH("aaaa", "a+");
    MUST_FAIL("", "a+");

    MUST_MATCH("a", ".");
    MUST_FAIL("", ".");

    MUST_MATCH("*", "\\*");
    MUST_MATCH("$", "\\$");
    MUST_MATCH("[", "\\[");

    MUST_MATCH("a", "$a");
    MUST_FAIL("1", "$a");
    MUST_MATCH("9", "$d");
    MUST_FAIL("x", "$d");

    MUST_MATCH("a", "[abc]");
    MUST_FAIL("d", "[abc]");
    MUST_MATCH("5", "[$d]");
    MUST_FAIL("5", "[^$d]");

    MUST_MATCH("a", "[^bcd]");

    MUST_MATCH("aaaaaaaaaaaaaaaaaaaaX", "a*a*a*a*a*a*a*a*X");

    MUST_MATCH("h", "[$a_][$w]*");

    MUST_MATCH("\"This is a string\"", "\".*\"");

    MUST_MATCH("a a", "a[^$w_]a");

    MUST_MATCH("test a", "[$a_][$w_]*[^$w_]!");

    MUST_MATCH("abc", "abc");
    MUST_FAIL("abc", "abd");
    MUST_MATCH("", "");
    MUST_MATCH("a", "");

    // Kleene star (*)
    MUST_MATCH("aaaa", "a*");
    MUST_MATCH("", "a*");
    MUST_MATCH("abbbb", "ab*");
    MUST_FAIL("abbbb", "ab*c");
    MUST_MATCH("xyz", "x*y*z*");

    // Plus (+)
    MUST_MATCH("aaaa", "a+");
    MUST_FAIL("", "a+");
    MUST_MATCH("a", "a+");
    MUST_FAIL("b", "a+");

    // Dot (.)
    MUST_MATCH("a", ".");
    MUST_FAIL("", ".");
    MUST_MATCH("x", ".");
    MUST_MATCH("1", ".");

    // Escapes
    MUST_MATCH("*", "\\*");
    MUST_MATCH("$", "\\$");
    MUST_MATCH("[", "\\[");
    MUST_MATCH("]", "\\]");
    MUST_MATCH("\\", "\\\\");
    MUST_MATCH("+", "\\+");

    // Character classes ($a, $d, etc)
    MUST_MATCH("a", "$a");
    MUST_FAIL("1", "$a");
    MUST_MATCH("Z", "$a");
    MUST_MATCH("9", "$d");
    MUST_FAIL("x", "$d");
    MUST_MATCH("0", "$d");

    // Character sets []
    MUST_MATCH("a", "[abc]");
    MUST_FAIL("d", "[abc]");
    MUST_MATCH("5", "[$d]");
    MUST_FAIL("5", "[^$d]");
    MUST_MATCH("a", "[^bcd]");
    MUST_MATCH("x", "[a-z]");
    MUST_FAIL("A", "[a-z]");

    // Negated character sets [^]
    MUST_MATCH("x", "[^abc]");
    MUST_FAIL("a", "[^abc]");

    // Complex patterns
    MUST_MATCH("aaaaaaaaaaaaaaaaaaaaX", "a*a*a*a*a*a*a*a*X");
    MUST_MATCH("h", "[$a_][$w]*");
    MUST_MATCH("_var123", "[$a_][$w]*");


    const char * number_pattern = "$d+$d~\\.?$d*$d~[fF]?";
    // Float-like patterns
    MUST_MATCH("51.23", number_pattern);
    MUST_MATCH("51", number_pattern);
    MUST_MATCH("52.", number_pattern);
    MUST_MATCH("51.23.5", number_pattern);
    MUST_FAIL(".23", number_pattern);
    MUST_MATCH("2.23F", number_pattern);
    MUST_MATCH("1.23f", number_pattern);

    // String patterns
    MUST_MATCH("\"This is a string\"", "\".*\"");
    MUST_MATCH("\"\"", "\".*\"");
    MUST_FAIL("\"unterminated", "\".*\"");

    // Word boundary patterns
    MUST_MATCH("a a", "a[^$w_]a");
    MUST_FAIL("aa", "a[^$w_]a");

    // Identifier patterns
    MUST_MATCH("test a", "[$a_][$w_]*[$w_]~");
    MUST_MATCH("_test ", "[$a_][$w_]*[$w_]~");
    MUST_FAIL("123test ", "[$a_][$w_]*[$w_]~");
    MUST_MATCH("var_name1 = 5;", "[$a_][$w_]*[$w_]~");

    // Necessary matches ignored
    MUST_MATCH("test", "test!");
    MUST_FAIL("tes", "test!");

    // Edge cases
    MUST_MATCH("aaa", "a.a");
    MUST_MATCH("aba", "a.a");
    MUST_MATCH("abc", "a.*c");
    MUST_MATCH("ac", "a.*c");

    MUST_MATCH("", "a?");
    MUST_FAIL("", "a+");
    MUST_MATCH("a", "a?");
    MUST_MATCH("", ".*");
    MUST_FAIL("abc", "a$");

    MUST_MATCH("aaaaaaaaaaaaaaaaab", "a*a*a*a*a*a*a*a*b");
    MUST_FAIL("aaaaaaaaaaaaaaaaab", "a*a*a*a*a*a*a*a*c");

    MUST_MATCH("ab", "a?b");
    MUST_MATCH("b", "a?b");
    MUST_MATCH("abbb", "ab+");
    MUST_FAIL("a", "ab+");

    MUST_MATCH("/* This is a comment \n"
                "* Ok\n"
                "*/ /* test 2 */", "(/\\*).*(\\*/)");

    const char * pattern = "\"(.*[\\\\\"]!(\\\\\")?\\\\?\\\\?)*\"";

    struct rsp_pattern *compiled_pattern = rsp_compile(pattern);
    printf("Compiled pattern: \n");
    rsp_print(compiled_pattern);
    printf("\n");
    rsp_free(compiled_pattern);
    MUST_MATCH("\"This is\\\\\\\" a \\\"text\\\" + \"And more text\"", pattern);
    MUST_MATCH("\"This is a text\" + \"And more text\"", pattern);
    MUST_MATCH("\"This is a text + \"And more text\"", pattern);
    MUST_MATCH("\"This is\\\" multiline\n"
                "te\\\"xt\\\"\" + \"And more text\"", pattern);

    MUST_MATCH("aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaX", "a*a*a*a*a*a*a*a*a*a*a*a*a*a*a*a*X");
    MUST_MATCH("((((((((((nested)))))))))", "(\\()*\\(~.*(\\))*");
    MUST_MATCH("***$$$[[[\\\\\\\\", "([\\*\\$\\[\\\\])*");
    MUST_MATCH("test_var_123_abc_xyz_final", "[$a_][$w_]*[$a_][$w_]*[$d][$w_]*[$a_][$w_]*[$a_][$w_]*[$a_][$w_]*");

    MUST_MATCH(" => test_var_123_abc_xyz_final", ".*([$a_][$w_]*[$a_][$w_]*[$d][$w_]*[$a_][$w_]*[$a_][$w_]*[$a_][$w_]*)!");

    const char * test = "simple_test dfegr gerg 125.6f DFEF";
    const char * test_result = rsp_compile_and_match(test, ".*($d+$d~\\.?$d*$d~[fF]?)!");

    const char * test2 = rsp_compile_and_match(test_result, number_pattern);

    char * result = malloc(test2 - test_result + 1);
    strncpy_s(result, test2 - test_result + 1, test_result, test2 - test_result);
    result[test2 - test_result] = '\0';

    printf("Final extracted number: \"%s\"\n", result);

    free(result);

    MUST_MATCH("", "");

    printf("\nAll torture tests passed (if you reached here alive)!\n");

    return 0;
}