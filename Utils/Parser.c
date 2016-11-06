#include <ctype.h>
#include <string.h>
#include "mua_types.h"
#include "parser.h"

//-----------------------------------------------------------------------------
//  String parsing functions.
//-----------------------------------------------------------------------------

#define NOLIMIT     (-1)

//
//  meta-function match
//
typedef bool( * ParserMetaFct_t )( char c, ... ) ;


//-----------------------------------------------------------------------------
//                   P R I V A T E   F U N C T I O N S
//-----------------------------------------------------------------------------
//
//  Match an alphabetic character
//
static bool Meta_a( char c )
{
    return isalpha( c ) ;
}

//
//  Match a specific character
//
static bool Meta_c( char c, char c2 )
{
    return ( c == c2 ) ;
}

//
//  Match a digit character
//
static bool Meta_d( char c )
{
    return isdigit( c ) ;
}

//
//  Match character in list
//
static bool Meta_l( char c, ParserToken_t* token )
{
    uint16  i = 0;

    while( i < token->tknLen-2 ) {
        if( '-' == token->tknPtr[i+1] ) {
            char c0 = token->tknPtr[i] ;
            char c1 = token->tknPtr[i+2] ;
            if((c0 <= c) &&(c <= c1)) {
                return TRUE ;
            }
            i+=3 ;
        }
        else {
            if( c == token->tknPtr[i] ) {
                return TRUE ;
            }
            i+=1;
        }
    }

    while( i < token->tknLen ) {
        if( c == token->tknPtr[i] ) {
            return TRUE ;
        }
        i+=1;
    }

    return FALSE ;
}

//
//  Match a space character
//
static bool Meta_s( char c )
{
    return isspace( c ) ;
}

//
//  Match a word character
//
static bool Meta_w( char c )
{
    return ( isalnum( c ) || c == '_' || c == '.' ) ;
}

//
//  Match a meta field
//
static const char* MatchMeta(
    const char*     str,
    bool            compareType,
    ParserMetaFct_t func,
    uint16          min,
    uint16          max,
    void*           auxParm )
{
    uint16          i ;

    //
    //  note: xor(^) is bitwise, not logical; do not trust
    //  boolean TRUE to be consistent bit pattern!
    //

    for( i=0; i<min; i++, str++ ) {
        bool b1 = func( *str, auxParm )?1:0 ;
        bool b2 = compareType?1:0 ;
        if ( b1 ^ b2 ) return 0 ;
    }

    for( ; i<max; i++, str++ ) {
        bool b1 = func( *str, auxParm )?1:0 ;
        bool b2 = compareType?1:0 ;
        if( b1 ^ b2 ) return str ;
    }

    return str ;
}

//
//  Interpret a 'count' field
//
static Result_t GetCountField( ParserMatch_t* match, uint16* min, uint16* max )
{
    char            cf ;
    ParserToken_t   tkn [2] ;
    ParserMatch_t   tmpMatch ;

    cf = *match->pPtrn++ ;

    switch( cf ) {

    case '*':                   // zero or more
        *min = 0 ;
        *max = (uint16)NOLIMIT ;
        break ;

    case '?':                   // one or more
        *min = 0 ;
        *max = 1 ;
        break ;

    case '+':                   // zero or one
        *min = 1 ;
        *max = (uint16)NOLIMIT ;
        break ;

    case '{':


        //
        //  use the pattern-matching function to match the pattern
        //

        if( RESULT_OK == ParserMatchPattern( "(~d+)}", match->pPtrn, &tmpMatch, tkn ) ) {
            match->pPtrn += tmpMatch.pLine - match->pPtrn ;
            if( RESULT_OK != ParserTknToUInt( &tkn[0], min ) ) {
                return RESULT_ERROR ;
            }
            *max = *min ;
            break ;
        }

        if( RESULT_OK == ParserMatchPattern( "(~d+),}", match->pPtrn, &tmpMatch, tkn ) ) {
            match->pPtrn += tmpMatch.pLine - match->pPtrn ;
            if( RESULT_OK != ParserTknToUInt( &tkn[0], min ) ) {
                return RESULT_ERROR ;
            }
            *max = (uint16)NOLIMIT ;
            break ;
        }

        if( RESULT_OK == ParserMatchPattern( ",(~d+)}", match->pPtrn, &tmpMatch, tkn ) ) {
            match->pPtrn += tmpMatch.pLine - match->pPtrn ;
            if( RESULT_OK != ParserTknToUInt( &tkn[0], max ) ) {
                return RESULT_ERROR ;
            }
            *min = 0 ;
            break ;
        }

        if( RESULT_OK == ParserMatchPattern( "(~d+),(~d+)}", match->pPtrn, &tmpMatch, tkn ) ) {
            match->pPtrn += tmpMatch.pLine - match->pPtrn ;
            if( RESULT_OK != ParserTknToUInt( &tkn[0], min ) ) {
                return RESULT_ERROR ;
            }
            if( RESULT_OK != ParserTknToUInt( &tkn[1], max ) ) {
                return RESULT_ERROR ;
            }
            break ;
        }

        return RESULT_ERROR ;

    default:                    //  no repeat field
        *min = *max = 1 ;
        --(match->pPtrn) ;
        break ;
    }

    return RESULT_OK ;
}

//
//  Match an escaped pattern character
//
static Result_t MatchEscape(
    bool            caseSensitive,
    ParserMatch_t*  match )
{
    bool            compareType ;
    char            metaChar ;
    ParserMetaFct_t func ;
    uint16          min ;
    uint16          max ;
    char            c2 ;
    void*           auxParm = NULL;
    char            listPtrn [10] ;
    ParserToken_t   token ;
    ParserMatch_t   ptrnMatch ;

    //
    //  get and validate the meta char
    //
    metaChar = *match->pPtrn++ ;

    //
    //  peak ahead for add'l required chars
    //

    compareType = islower( metaChar ) ;
    switch( tolower( metaChar ) ) {

    case 'a':
        func =(ParserMetaFct_t) Meta_a ;
        break ;

    case 'c':
        func =(ParserMetaFct_t) Meta_c ;
        c2 = *match->pPtrn++ ;

        if( !c2 ) {
            return RESULT_ERROR ;
        }

        if( caseSensitive ) {
            metaChar = toupper( metaChar ) ;
            c2       = toupper( c2 ) ;
        }

        auxParm =(void*) c2 ;
        break ;

    case 'd':
        func =(ParserMetaFct_t) Meta_d ;
        break ;

    case 'l':
        func =(ParserMetaFct_t) Meta_l ;
        c2 = *match->pPtrn++ ;      //  c2 is the list delimiter

        if( !c2 ) {
            return RESULT_ERROR ;
        }

        //
        //  Trick: build a pattern using the first character following ~l(~L) as
        //  the beginning and ending delimiter.  The original pattern itself will
        //  be pattern-matched against this new pattern to extract the character
        //  list.
        //
        strcpy( listPtrn, "(~C/+)~c/" ) ;
        listPtrn[3]  = c2 ;
        listPtrn[8]  = c2 ;

        if( RESULT_OK != ParserMatchPattern( listPtrn, match->pPtrn, &ptrnMatch, &token ) ) {
            return RESULT_ERROR ;
        }

        //
        //  Adjust the original pattern by the length of the list, including the
        //  terminal delimiter.
        //
        match->pPtrn += token.tknLen+1 ;

        if( caseSensitive ) {
            metaChar = toupper( metaChar ) ;
            c2       = toupper( c2 ) ;
        }

        auxParm =(void*) &token ;

        break ;

    case 's':
        func =(ParserMetaFct_t) Meta_s ;
        break ;

    case 'w':
        func =(ParserMetaFct_t) Meta_w ;
        break ;

    default:
        return RESULT_ERROR ;
    }

    //
    //  get the count field if present
    //
    if( RESULT_OK != GetCountField( match, &min, &max ) ) {
        return RESULT_ERROR ;
    }

    //
    //  scan the line
    //

    if( max > 0 ) {
        match->pLine = MatchMeta( match->pLine, compareType, func, min, max, auxParm ) ;

        if( !match->pLine ) {
            return RESULT_ERROR ;
        }
    }

    return RESULT_OK ;
}

static bool CaseSensitiveSearch( ParserMatch_t* match )
{
    switch( match->pPtrn[0] ) {
    case '~':
        switch( match->pPtrn[1] ) {
        case 'i':
            match->pPtrn+=2 ;
            return FALSE ;
        case 'I':
            match->pPtrn+=2 ;
            return TRUE ;
        }
    }
    return FALSE ;
}

//-----------------------------------------------------------------------------
//                   P U B L I C   F U N C T I O N S
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
//
//  ParserMatchPattern
//
//  Description:
//
//      A test string is compared to a pattern, character-by-character, from
//      left to right.  The pattern may contain individual characters to match,
//      and/or "meta-fields" that specify groups of characters:
//
//          ~a      - match alphabetic characters
//          ~A      - match non-alphabetic characters
//          ~cx     - match character 'x'
//          ~Cx     - match not character 'x'
//          ~d      - match decimal characters
//          ~D      - match non-decimal characters
//          ~l      - match characters in list /.../ (any delimiter '/')
//          ~L      - match characters not in list /.../ (any delimiter '/')
//          ~s      - match space characters
//          ~S      - match non-space characters
//          ~w      - match word characters( alphanumeric, '_' or '.' )
//          ~W      - match non-word characters
//           $      - match end of line
//
//      Special fields:
//          ~i      - case-insensitive search.  If omitted, case-insensitive
//                    search is applied.  Must appear at beginning of pattern.
//          ~I      - case-sensitive search.  If omitted, case-insensitive
//                    search is applied.  Must appear at beginning of pattern.
//
//      A meta-field may be optionally followed by a repetition-count :
//      expression:
//
//          +       - match one or more character
//          *       - match zero or more characters
//          ?       - match zero or one character
//          {n,m}   - match from 'n' to 'm' characters
//          {,m}    - match up to 'm' characters
//          {n,}    - match 'n' or more characters
//
//
//      Tokens enable substrings to be extracted from the test string.
//      Tokens are identified using
//
//          (       - start token
//          )       - end token
//
//  Returns:
//
//      RESULT_OK           -   pattern matched
//      RESULT_ERROR        -   pattern did not match,
//                              or error in pattern specification
//-----------------------------------------------------------------------------

static Result_t MatchPattern(
    const char*         pPtrn,          //  pointer to the pattern specification
    const char*         pLine,          //  pointer to the line to test
    ParserMatch_t*      match,          //  match result
    ParserToken_t*      token )
{
    char            ptrnChar ;

    uint16          tknState ;
    const char*     tknPtr ;
    uint16          tknLen ;
    uint16          nTkn ;
    ParserMatch_t   tmpMatch ;
    bool            caseSensitive ;

    if (pPtrn == NULL) {
        return RESULT_ERROR;
    }

    //
    //  if pLine!= 0 use this a pointer to the test line, otherwise use the
    //  stored line pointer in 'match'; this allows subsequent calls while
    //  scanning a line, without having to externally keep track of the
    //  last position of the test line
    //
    if( pLine ) {
        tmpMatch.pLine = pLine ;
    }
    else {
        tmpMatch.pLine = match->pLine ;
    }

    tmpMatch.pPtrn = pPtrn ;

    tknState = 0 ;
    nTkn = 0 ;

    caseSensitive = CaseSensitiveSearch( &tmpMatch ) ;

    for( ;; ) {
        ptrnChar = *tmpMatch.pPtrn++ ;
        if( !ptrnChar ) break ;

        //
        //  pattern char is 'escape'
        //
        if( '~' == ptrnChar ) {
            if( RESULT_OK != MatchEscape( caseSensitive, &tmpMatch ) ) {
                return RESULT_ERROR ;
            }
        }

        //
        //  if left paren starts token extraction
        //
        else if( '(' == ptrnChar ) {
            if( tknState ) {
                return RESULT_ERROR ;
            }

            tknState = 1 ;
            tknPtr = tmpMatch.pLine ;
        }

        //
        //  if left paren starts token extraction
        //
        else if( ')' == ptrnChar ) {

            if( !tknState ) {
                return RESULT_ERROR ;
            }

            tknState = 0 ;
            tknLen = tmpMatch.pLine - tknPtr ;

            token->tknPtr = tknPtr ;
            token->tknLen = tknLen ;
            ++token ;
            nTkn +=1 ;
        }

        //
        //  dollar sign matches end of line
        //
        else if( '$' == ptrnChar ) {
            if( *tmpMatch.pLine ) {
                return RESULT_ERROR ;
            }
        }

        //
        //  pattern char is not 'escape' -- do simple compare
        //
        else {
            char lineChar = *tmpMatch.pLine++ ;

            if( !lineChar ) {
                return RESULT_ERROR ;
            }

            if( caseSensitive ) {
                if( ptrnChar != lineChar ) {
                    return RESULT_ERROR ;
                }
            }
            else {
                if( toupper( ptrnChar )  != toupper( lineChar ) ) {
                    return RESULT_ERROR ;
                }
            }
        }
    }

    *match = tmpMatch ;

    return RESULT_OK ;
}

//
//  Convert a token to uint16, with error check
//
Result_t ParserTknToUInt( ParserToken_t* tkn, uint16* retVal )
{
    uint16              v = 0 ;
    const char*         s ;

    s = tkn->tknPtr ;

    if( tkn->tknPtr ) {
        if( tkn->tknLen ) {
            uint16 i ;
            for( i=0; i<tkn->tknLen; i++ ) {
                char c = *s++ ;
                if( !c ) break ;
                if( c >= '0' && c <= '9' ) {
                    v *= 10 ;
                    v += c - '0' ;
                }
                else {
                    return RESULT_ERROR ;
                }
            }
        }
    }
    *retVal = v ;
    return RESULT_OK ;
}

//
//  Convert a null-terminated string to int16, with error check
//
Result_t ParserStrToInt( const char* str, int16* retVal )
{
    int16 v = 0 ;
    char c;

    if(!*str) {
        return RESULT_ERROR ;
    }

    for( ;; ) {
        c = *str++ ;
        if( !c ) break ;
        if( c >= '0' && c <= '9' ) {
            v *= 10 ;
            v += c - '0' ;
        }
        else {
            return RESULT_ERROR ;
        }
    }

    *retVal = v ;

    return RESULT_OK ;
}

//
//  Convert a null-terminated string to uint8, with error check
//
Result_t ParserStrToUInt8( const char* str, uint8* retVal )
{
    uint16 v = 0 ;
    char c;

    if(!*str) {
        return RESULT_ERROR ;
    }

    for( ;; ) {
        c = *str++ ;
        if( !c ) break ;
        if( c >= '0' && c <= '9' ) {
            v *= 10 ;
            v += c - '0' ;
        }
        else {
            return RESULT_ERROR ;
        }
    }

    if( v > 0xff ) {
        return RESULT_ERROR ;
    }

    *retVal = (uint8)v ;

    return RESULT_OK ;
}

//
//  Extract string from token
//
Result_t ParserTknToStr(
    const ParserToken_t*    tkn,
    char*                   str,
    uint16                  maxChr )
{
    char*                   b ;
    const char*             s ;

    if( tkn->tknLen >= maxChr ) {
        return RESULT_ERROR ;
    }

    *str = 0 ;
    b = str ;
    s = tkn->tknPtr ;

    if( tkn->tknPtr ) {
        if( tkn->tknLen ) {
            uint16 i ;
            for( i=0; i<tkn->tknLen; i++ ) {
                *b++ = *s++ ;
            }
            *b = 0 ;
        }
    }
    return RESULT_OK ;
}

//
//  Concatenate tokens to string
//
Result_t ParserCatTkns(
    ParserToken_t*      tkn,
    uint8           n_tkn,
    uint8*          strOut,
    uint16          maxChr )

{
    uint8           n ;

    *strOut = 0 ;

    for( n=0; n<n_tkn; n++ ) {

        uint16 l_strOut = strlen( (char*)strOut ) ;

        if( RESULT_OK != ParserTknToStr ( tkn++, (char*)strOut+l_strOut, (uint16)(maxChr-l_strOut) ) ) {
            return RESULT_ERROR ;
        }
    }

    return RESULT_OK ;
}

void ParserInitMatch(
    const char*         pLine,
    ParserMatch_t*      match)
{
    match->pLine         = pLine ;
    match->pPtrn         = 0 ;
}

Result_t ParserMatch(
    const char*         pPtrn,          //  pointer to the pattern specification
    ParserMatch_t*      match,          //  match result
    ParserToken_t*      token )
{
    return MatchPattern( pPtrn, 0, match, token ) ;
}

Result_t ParserMatchPattern(
    const char*         pPtrn,          //  pointer to the pattern specification
    const char*         pLine,          //  pointer to the line to test
    ParserMatch_t*      match,          //  match result
    ParserToken_t*      token )
{
    return MatchPattern( pPtrn, pLine, match, token ) ;
}

