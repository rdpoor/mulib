/**
 * @file mu_json.h
 *
 * @brief mu_json is a compact, efficient JSON parser.
 * 
 * mu_json takes a JSON formatted string and parses it into its constituent 
 * tokens.  Once parsed, mu_json provides functions to navigate among the
 * resulting tokens via parent, child, next_sibling, prev_sibling, etc.
 * 
 * Parsing is done entirely in place: strings are not copied.  The user provies
 * token storage, so mu_json never calls malloc() or free().
 * 
 * Thanks to:
 *   Douglas Crockford for creating JSON in the first place, and for the compact
 *   JSON_checker.c design on which much of mu_json is based.
 *   Serge Zaitsev for creating JSMN, an efficient copy-free JSON parser, which
 *   informed the "sliced token" approach used by mu_json.
 * 
 * Unlike many parsers, mu_json does not try to interpret the parsed values.
 * Rather, it simply slices each token into a substring and associates a JSON
 * type to it.
 * 
 * Here's a simple example:
 * 
 * @code
 * 
 * 
 * 
 * TODO:
 * * Start numbers as INTEGER type, promote to NUMBER type only as needed.
 * * Extend `finish_token()` to check that the token type being finished 
 *   matches the expected type, and write unit test to verify.
 * * Create mu_json_parser_info_t structure, pass it into the parsing functions.
 *   If non-null, when the parsing function returns, it will contain extra info
 *   such as the char position where parsing failed, etc.
 */

#ifndef _MU_JSON_H_
#define _MU_JSON_H_

// *****************************************************************************
// Includes

#include "mu_str.h"
#include <stddef.h>
#include <stdint.h>

// *****************************************************************************
// C++ Compatibility

#ifdef __cplusplus
extern "C" {
#endif

// *****************************************************************************
// Public types and definitions

/**
 * @brief Enumeration of error codes returned by mu_json functions.
 */
typedef enum {
    MU_JSON_ERR_NONE = 0,        /**< No error */
    MU_JSON_ERR_BAD_FORMAT = -1, /**< Illegal JSON format */
    MU_JSON_ERR_NO_TOKENS = -2,  /**< Not enough tokens provided */
    MU_JSON_ERR_INCOMPLETE = -3  /**< JSON ended with unterminated form */
} mu_json_err_t;

/**
 * @brief Enumeration of token flags used by mu_json.
 */
typedef enum {
    MU_JSON_TOKEN_FLAG_IS_FIRST = 1,  /**< Token is first in token list */
    MU_JSON_TOKEN_FLAG_IS_LAST = 2,   /**< Token is last in token list */
    MU_JSON_TOKEN_FLAG_IS_SEALED = 4  /**< Token end has been found */
} mu_json_token_flags_t;

#define DEFINE_MU_JSON_TOKEN_TYPES(M)                                          \
    M(MU_JSON_TOKEN_TYPE_UNKNOWN) /* ?       */                                \
    M(MU_JSON_TOKEN_TYPE_ARRAY)   /* [ ... ] */                                \
    M(MU_JSON_TOKEN_TYPE_OBJECT)  /* { ... } */                                \
    M(MU_JSON_TOKEN_TYPE_STRING)  /* "..."   */                                \
    M(MU_JSON_TOKEN_TYPE_NUMBER)  /* 123.45  */                                \
    M(MU_JSON_TOKEN_TYPE_INTEGER) /* 12345 (specialized number) */             \
    M(MU_JSON_TOKEN_TYPE_TRUE)    /* true    */                                \
    M(MU_JSON_TOKEN_TYPE_FALSE)   /* false   */                                \
    M(MU_JSON_TOKEN_TYPE_NULL)    /* null    */

/**
 * @brief Enumeration of JSON token types.
 */
#define EXPAND_MU_JSON_TOKEN_TYPE_ENUMS(_name) _name,
typedef enum {
    DEFINE_MU_JSON_TOKEN_TYPES(EXPAND_MU_JSON_TOKEN_TYPE_ENUMS)
} mu_json_token_type_t;

/**
 * @brief Structure representing a JSON token.
 */
typedef struct {
    mu_str_t json; /**< Slice of the original JSON string */
    uint8_t type;  /**< mu_json_token_type cast to uint8_t */
    uint8_t flags; /**< mu_json_token_flags_t cast to uint8_t */
    int16_t depth; /**< 0 = toplevel, n+1 = child of n... */
} mu_json_token_t;

// *****************************************************************************
// Public declarations

/**
 * @brief Parse a JSON-formatted string provided as a null-terminated C string.
 *
 * Parse the JSON-formatted string `json` into a series of tokens stored in 
 * user-supplied `token_store` containing `max_tokens`. The JSON string
 * is expected to be null-terminated.
 *
 * @param token_store A user-supplied array of tokens for receiving the parsed
 *        results.
 * @param max_tokens Number of tokens in `token_store`.
 * @param json The JSON-formatted string to be parsed, provided as a 
 *        null-terminated C string.
 * 
 * @return Returns the number of parsed tokens if parsing is successful, or a
 *         negative error code if an error occurs.
 */
int mu_json_parse_c_str(mu_json_token_t *token_store, size_t max_tokens,
                        const char *json);

/**
 * @brief Parse a JSON-formatted string provided as a mu_str_t object.
 *
 * Parse the JSON-formatted string in mu_json` into a series of tokens stored in 
 * user-supplied `token_store` containing `max_tokens` elements.
 *
 * @param token_store A user-supplied array of tokens for receiving the parsed
 *        results.
 * @param max_tokens Number of tokens in `token_store`.
 * @param mu_json Pointer to a mu_str_t object containing the JSON-formatted 
 *        string to be parsed.
 * 
 * @return Returns the number of parsed tokens if parsing is successful, or a
 *         negative error code if an error occurs.
 */
int mu_json_parse_mu_str(mu_json_token_t *token_store, size_t max_tokens,
                         mu_str_t *mu_json);

/**
 * @brief Parse a JSON-formatted string stored in a buffer.
 *
 * Parse the JSON-formatted string in mu_json` into a series of tokens stored in 
 * user-supplied `token_store` containing `max_tokens` elements.
 * Psrse the JSON-formatted buffer `buf` of length `buflen` into a series of
 * tokens stored in the user-supplied `token_store` containing `max_tokens`
 * elements.  
 * 
 * @param token_store A user-supplied array of tokens for receiving the parsed
 *        results.
 * @param max_tokens Number of tokens in `token_store`.
 * @param buf Pointer to a uint8_t array containing the JSON-formatted buffer.
 * @param buflen Length of the JSON-formatted buffer `buf`.
 * 
 * @return Returns the number of parsed tokens if parsing is successful, or a
 *         negative error code if an error occurs.
 */
int mu_json_parse_buffer(mu_json_token_t *token_store, size_t max_tokens,
                         const uint8_t *buf, size_t buflen);

/**
 * @brief Retrieve the mu_str_t slice associated with a JSON token.
 *
 * Return a pointer to the mu_str_t slice associated with the
 * JSON token `token`.
 *
 * @param token Pointer to the JSON token.
 * 
 * @return Pointer to the mu_str_t slice associated with the JSON token.
 */
mu_str_t *mu_json_token_mu_str(mu_json_token_t *token);

/**
 * @brief Retrieve the JSON type of a JSON token.
 *
 * Return the JSON type of the JSON token `token`.
 *
 * @param token Pointer to the JSON token.
 * 
 * @return JSON type of the JSON token.
 */
mu_json_token_type_t mu_json_token_type(mu_json_token_t *token);

/**
 * @brief Retrieve the depth of a JSON token in the JSON hierarchy.
 *
 * Return the depth of the JSON token `token` in the JSON hierarchy.
 * The depth is 0 for the top-level tokens, and increases for nested tokens.
 *
 * @param token Pointer to the JSON token.
 * 
 * @return Depth of the JSON token.
 */
int mu_json_token_depth(mu_json_token_t *token);

/**
 * @brief Check if a JSON token is the first token in its token list.
 *
 * This function checks if the JSON token `token` is the first token in its token list.
 *
 * @param token Pointer to the JSON token.
 * 
 * @return true if the token is the first token in its token list, false otherwise.
 */
bool mu_json_token_is_first(mu_json_token_t *token);

/**
 * @brief Checks if a JSON token is the last token in its token list.
 *
 * This function checks if the JSON token `token` is the last token in its token list.
 *
 * @param token Pointer to the JSON token.
 * 
 * @return true if the token is the last token in its token list, false otherwise.
 */
bool mu_json_token_is_last(mu_json_token_t *token);

/**
 * @brief Retrieves the previous JSON token in its token list.
 *
 * Return a pointer to the previous sibling of the JSON token `token`.
 *
 * @param token Pointer to the JSON token.
 * 
 * @return Pointer to the previous sibling JSON token, or NULL if no previous sibling exists.
 */
mu_json_token_t *mu_json_token_prev(mu_json_token_t *token);

/**
 * @brief Retrieves the next JSON token in its token list.
 *
 * Return a pointer to the next sibling of the JSON token `token`.
 *
 * @param token Pointer to the JSON token.
 * 
 * @return Pointer to the next sibling JSON token, or NULL if no next sibling exists.
 */
mu_json_token_t *mu_json_token_next(mu_json_token_t *token);

/**
 * @brief Retrieves the root token of a JSON token.
 *
 * Return a pointer to the root token of the JSON token `token`.
 *
 * @param token Pointer to the JSON token.
 * 
 * @return Pointer to the root JSON token.
 */
mu_json_token_t *mu_json_token_root(mu_json_token_t *token);

/**
 * @brief Retrieves the parent token of a JSON token.
 *
 * Return a pointer to the parent token of the JSON token `token`.
 *
 * @param token Pointer to the JSON token.
 * 
 * @return Pointer to the parent JSON token, or NULL if `token` is the root token.
 */
mu_json_token_t *mu_json_token_parent(mu_json_token_t *token);

/**
 * @brief Retrieves the first child token of a JSON token.
 *
 * Return a pointer to the first child token of the JSON token `token`.
 *
 * @param token Pointer to the JSON token.
 * 
 * @return Pointer to the first child JSON token, or NULL if `token` has no children.
 */
mu_json_token_t *mu_json_token_child(mu_json_token_t *token);

/**
 * @brief Retrieves the previous sibling token of a JSON token.
 *
 * Return a pointer to the previous sibling token of the JSON token `token`.
 *
 * @param token Pointer to the JSON token.
 * @return Pointer to the previous sibling JSON token, or NULL if no previous sibling exists.
 */
mu_json_token_t *mu_json_token_prev_sibling(mu_json_token_t *token);

/**
 * @brief Retrieves the next sibling token of a JSON token.
 *
 * Return a pointer to the next sibling token of the JSON token `token`.
 *
 * @param token Pointer to the JSON token.
 * @return Pointer to the next sibling JSON token, or NULL if no next sibling exists.
 */
mu_json_token_t *mu_json_token_next_sibling(mu_json_token_t *token);

#ifdef __cplusplus
}
#endif

#endif /* #ifndef _MU_JSON_H_ */
