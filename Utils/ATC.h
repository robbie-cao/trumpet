#ifndef __ATC_H__
#define __ATC_H__

typedef struct {
    uint8_t     val;
    const char* pattern;
} Keyword_t;

void ATC_Handler(uint8_t *buf);

#endif /* __ATC_H__ */

/* vim: set ts=4 sw=4 tw=0 list : */
