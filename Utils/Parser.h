/** \file parser.h
 *
 *  These functions perform pattern-matching operations on strings.
 *  They are primarily used by the AT Command Handler to parse AT command strings, but
 *  are also available for general-purpose string parsing.  .
 *
 *      Pattern matching is loosely equivalent to regular expression parsing.
 *      A subject string is compared to a pattern,
 *      character-by-character, from
 *      left to right.  The pattern may contain individual characters to match,
 *      and/or "meta-fields" that specify groups of characters:
 *
 *      -   ~a      - match alphabetic characters
 *      -   ~A      - match non-alphabetic characters
 *      -   ~cx     - match character 'x'
 *      -   ~Cx     - match not character 'x'
 *      -   ~d      - match decimal characters
 *      -   ~D      - match non-decimal characters
 *      -   ~l      - match characters in list /.../ (any delimiter '/')
 *      -   ~L      - match characters not in list /.../ (any delimiter '/')
 *      -   ~s      - match space characters
 *      -   ~S      - match non-space characters
 *      -   ~w      - match word characters( alphanumeric, '_' or '.' )
 *      -   ~W      - match non-word characters
 *      -    $      - match end of line
 *
 *      Special fields:
 *      -   ~i      - case-insensitive search.  If omitted, case-insensitive
 *                    search is applied.  Must appear at beginning of pattern.
 *      -   ~I      - case-sensitive search.  If omitted, case-insensitive
 *                    search is applied.  Must appear at beginning of pattern.
 *
 *      A meta-field may be optionally followed by a repetition-count :
 *      expression:
 *
 *      -   +       - match one or more character
 *      -   *       - match zero or more characters
 *      -   ?       - match zero or one character
 *      -   {n,m}   - match from 'n' to 'm' characters
 *      -   {,m}    - match up to 'm' characters
 *      -   {n,}    - match 'n' or more characters
 *
 *
 *      Tokens enable substrings to be extracted from the test string.
 *      Tokens are identified using
 *
 *      -   (       - start token
 *      -   )       - end token
 */

#ifndef __PARSER_H__
#define __PARSER_H__

/** Result Codes */
typedef enum {
    RESULT_OK = 0,                      ///< 0  Result OK, operation successful
    RESULT_ERROR,                       ///< 1  Result Error
    RESULT_DONE,                        ///< 2  Result Done,operation completed
    RESULT_UNKNOWN                      ///< 3  Result Unknown
} Result_t; ///< Result Codes

/** Describes a token.  A token is a substring extracted from a string using one of
  * the pattern-matching functions of this API.  A token is not null-terminated; instead
  * it contains a pointer to the start of the string, and a length in bytes.  The
  * token extraction and conversion routines in this API can be used to manipulate
  * tokens
  */
typedef struct {
    const char*             strBuf ;    /**< pointer to start of string buffer */
    const char*             tknPtr ;    /**< pointer to start of token string */
    uint16                  tknLen ;    /**< length of token string */
}   ParserToken_t ;

/** Status of the parser API pattern matching functions.
 */
typedef struct {
    const char*             pPtrn ;     /**< pointer to the pattern specification */
    const char*             pLine ;     /**< pointer to the line to test */
}   ParserMatch_t ;

/** Copy a token to a null-terminated string */
Result_t ParserTknToStr (
    const ParserToken_t*    tkn,            /**<    the token */
    char*                   str,            /**<    string buffer */
    uint16                  maxChr          /**<    max number of chars in string buffer */
) ;

/** Convert a token to a signed integer.  \return RESULT_ERROR if not a valid integer; else RESULT_OK
 */
Result_t ParserStrToInt (
    const char*             str,            /**<    the string */
    int16*                  retVal          /**<    the integer, returned */
) ;

/** Convert a token to a uint8.  \return RESULT_ERROR if not a valid uint8; else RESULT_OK
 */
Result_t ParserStrToUInt8(
    const char*             str,            /**<    the string */
    uint8*                  retVal          /**<    the uint8, returned */
) ;

/** Convert a token to an unsigned integer.  \return ERROR if not a valid integer
 */
Result_t ParserTknToUInt (
    ParserToken_t*          tkn,
    uint16*                 retVal          /**<    the integer, returned */
) ;

/** Concatenate tokens into a null-terminated string.  Returns ERROR if the number
 *  of characters copied to the output string, including a NULL terminator,
 *  would exceed its size.
 */
Result_t ParserCatTkns(
    ParserToken_t*          tkn,            /**<    an array of one or more tokens */
    uint8                   n_tkn,          /**<    number of tokens in array */
    uint8*                  strOut,         /**<    output string buffer */
    uint16                  maxChr          /**<    max chars to copy including NULL */
) ;

/** Match a pattern.  This function is most convenient when performing a single
 *  pattern match.  Use ParserMatch() when making multiple matches on a single
 *  string
 */
Result_t ParserMatchPattern (
    const char*             pPtrn,      /**<    pointer to the pattern specification */
    const char*             pLine,      /**<    pointer to the line to test or NULL */
    ParserMatch_t*          match,      /**<    match results */
    ParserToken_t*          token       /**<    list of extracted tokens */
) ;

/** Initialize pattern matching.  This function is called before making calls to
 *  ParserMatch.  It initializes the ParserMatch_t structure.
 */
void ParserInitMatch(
    const char*             pLine,      /**<    pointer to start of string */
    ParserMatch_t*          match       /**<    match structure, initialized */
) ;

/** Match pattern.  This function is most convenient when performing multiple matches
 *  on a string.  Use ParserMatchPattern for one-time matches.
 */
Result_t ParserMatch (
    const char*             pPtrn,      /**<    pointer to the pattern specification */
    ParserMatch_t*          match,      /**<    match results */
    ParserToken_t*          token       /**<    list of extracted tokens */
) ;

#endif // __PARSER_H__
