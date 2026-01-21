/** ********************************************************************************
 * @section RSP_Overview Overview
 * @file rsp.h
 * @brief Header file for RSP string manipulation and pattern matching functionalities.
 * @details
 * Typical use cases:
 * - Compiling patterns for string matching.
 * - Matching strings against compiled patterns.
 * *********************************************************************************
 * @section RSP_Strings  Strings Module
 * <RSP/rsp.h>
 ***********************************************************************************
 * @section RSP_Metadata Metadata
 * @author Estorc
 * @version v1.0
 * @copyright Copyright (c) 2025 Estorc MIT License.
 **********************************************************************************/
/*                             This file is part of
 *                                      RSP
 *                        (https://github.com/Estorc/RSP)
 ***********************************************************************************
 * Copyright (c) 2025 Estorc.
 * This file is licensed under the MIT License.
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 ***********************************************************************************/

#include <string.h>

/**
 * @brief Enumeration of token types used in pattern matching.
 * Each token type represents a specific element or operation in the pattern.
 * @note This enumeration is used internally by the RSP string module.
 */
enum rsp_token_type {
    RSP_TT_CHAR,                // Literal character
    RSP_TT_WILDCARD,            // .
    RSP_TT_ZERO_PLUS,           // *
    RSP_TT_ONE_PLUS,            // +
    RSP_TT_ONE_ZERO,            // ?
    RSP_TT_POSITIVE_LOOKAHEAD,  // !
    RSP_TT_NEGATIVE_LOOKAHEAD,  // ~
    RSP_TT_CHAR_CLASS,          // $a, $d, $w, etc.
    RSP_TT_ESCAPE,              // \x
    RSP_TT_RANGE,               // [a-z]
    RSP_TT_NEG_RANGE,           // [^...]
    RSP_TT_GROUP,               // ( ... )
    RSP_TT_END,                 // End of pattern
    RSP_TT_TERMINATOR           // Terminator token
};

/**
 * @brief Structure representing a token in the pattern.
 * Each token consists of a type and associated data.
 * @note This structure is used internally by the RSP string module.
 */
struct rsp_token {
    enum rsp_token_type type;
    void * data;
};

/**
 * @brief Structure representing a compiled pattern.
 * The pattern consists of an array of tokens.
 * @note This structure is used internally by the RSP string module.
 */
struct rsp_pattern {
    struct rsp_token * tokens;
};

/**
 * @brief Prints the compiled pattern for debugging purposes.
 * @param pattern The compiled rsp_pattern to be printed.
 */
void rsp_print(struct rsp_pattern *pattern);

/**
 * @brief Compiles a pattern string into a rsp_pattern structure.
 * @param pattern_ptr The pattern string to be compiled.
 * @return A pointer to the compiled rsp_pattern.
 * @note The returned rsp_pattern should be freed using rsp_free() when no longer needed.
 */
struct rsp_pattern * rsp_compile(const char * pattern_ptr);

/**
 * @brief Frees the memory allocated for a compiled rsp_pattern.
 * @param pattern The rsp_pattern to be freed.
 */
void rsp_free(struct rsp_pattern * pattern);

/**
 * @brief Matches a string against a compiled pattern.
 * @param str The string to be matched.
 * @param pattern The compiled rsp_pattern to match against.
 * @return A pointer to the position in the string after the match, or NULL if no match is found.
 */
const char * rsp_match(const char * str, struct rsp_pattern * pattern);

/**
 * @brief Compiles a pattern string and matches it against a given string.
 * @param str The string to be matched.
 * @param pattern The pattern string to be compiled and matched against.
 * @return A pointer to the position in the string after the match, or NULL if no match is found.
 * @note This function handles both compilation and matching, and frees the compiled pattern afterwards.
 */
const char * rsp_compile_and_match(const char * str, const char * pattern);