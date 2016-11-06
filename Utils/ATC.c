#include <string.h>
#include <stdint.h>

#include "parser.h"

#include "ISD9160.h"
#include "Log.h"
#include "ATC.h"

#define LOG_TAG         "AT"

#define DEBUG_ATC       0

#if DEBUG_ATC
#define LOGD_ATC        LOGD
#else
#define LOGD_ATC        __LOGD
#endif

#define CMD_STR(str)    "~i~s*AT~s*+~s*"##str##"~s*"

enum {
    MUA_AT_CMD_MODE = 0,
    MUA_AT_CMD_PM,
    MUA_AT_CMD_CONF,
    MUA_AT_CMD_SYS,
    MUA_AT_CMD_RST,
    MUA_AT_CMD_SN,
    MUA_AT_CMD_NAME,
    MUA_AT_CMD_VOL,

    MUA_AT_CMD_TOTAL
};

const Keyword_t muaAtCmdList[] =
{
    { MUA_AT_CMD_MODE , CMD_STR("MODE~s*=~s*(~d+)")                                },
    { MUA_AT_CMD_PM   , CMD_STR("PM~s*=~s*(~d+)")                                  },
    { MUA_AT_CMD_CONF , CMD_STR("CONF~s*=~s*(~d+)~s*,~s*(~d+)~s*,~s*(~d+)")        },
    { MUA_AT_CMD_SYS  , CMD_STR("SYS")                                             },
    { MUA_AT_CMD_RST  , CMD_STR("RST")                                             },
    { MUA_AT_CMD_SN   , CMD_STR("SN~s*=~s*(~d+)~s*,~s*(~w+)")                      },
    { MUA_AT_CMD_NAME , CMD_STR("NAME~s*=~s*(~d+)~s*,~s*(~w+)")                    },
    { MUA_AT_CMD_VOL  , CMD_STR("VOL~s*=~s*(~d+),~s*(~d+)")                        },

    { 0xFF            , 0                                                          }
};

void ATC_Handler(uint8_t *buf)
{
    ParserMatch_t match;       //  pattern-matching results
    ParserToken_t token[9];    //  pattern-matching tokens
    char      tknBuf[32];
    uint16_t    tmpVal;
    const Keyword_t* kwd;
    Result_t result = RESULT_ERROR;

    for (kwd = muaAtCmdList; kwd->pattern != NULL; kwd++) {
        memset(token, 0, sizeof(token));
        memset(tknBuf, 0, sizeof(tknBuf));
        ParserInitMatch((char *)buf, &match);

        result = ParserMatchPattern(kwd->pattern, (char *)buf, &match, token);

        if (result != RESULT_OK) {
            // Skip if not match
            continue;
        }

        switch (kwd->val) {
            case MUA_AT_CMD_MODE:
                LOGD_ATC(LOG_TAG, "MODE\r\n");
                ParserTknToUInt(&token[0], &tmpVal) ;
                LOGD_ATC(LOG_TAG, "Val0 : %d\r\n", tmpVal);
                break;
            case MUA_AT_CMD_PM:
                LOGD_ATC(LOG_TAG, "PM\r\n");
                ParserTknToUInt(&token[0], &tmpVal) ;
                LOGD_ATC(LOG_TAG, "Val0 : %d\r\n", tmpVal);
                break;
            case MUA_AT_CMD_CONF:
                break;
            case MUA_AT_CMD_SYS:
                LOGD_ATC(LOG_TAG, "SYS\r\n");
                break;
            case MUA_AT_CMD_RST:
                LOG("RST\r\n");
                DrvSYS_Delay(10000);
                UNLOCKREG();
                DrvSYS_ResetChip();
                // Will never reach here as chip reset
                LOCKREG();
                break;
            case MUA_AT_CMD_SN:
                LOGD_ATC(LOG_TAG, "SN\r\n");
                break;
            case MUA_AT_CMD_NAME:
                LOGD_ATC(LOG_TAG, "NAME\r\n");
                break;
            case MUA_AT_CMD_VOL:
                LOGD_ATC(LOG_TAG, "VOL\r\n");
                break;
            default:
                break;
        }
        // Exit loop if match
        break;
    }
    if (!kwd->pattern) {
        // Not found match pattern
        LOGD_ATC(LOG_TAG, "Cmd Not Support\r\n");
    }
}

/* vim: set ts=4 sw=4 tw=0 list : */
