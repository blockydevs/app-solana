#pragma once

#include <stdbool.h>
#include <stdint.h>
#include "parser.h"
#include "../util.h"

#if EXTENDED_MEMORY
//Arbitrary number - we don't expect more than 10 resources
// It is impossible to predict precisely how many user will use
#define RESOURCES_MAX_LENGTH 10
#define SIWS_FIELDS_COUNT 12
#else
// Reduce memory usage on Nano S
//@FIXME this is a hack - we should use a better solution
#define RESOURCES_MAX_LENGTH 2
#define SIWS_FIELDS_COUNT 2
#endif



typedef const char* DateTimeString;
typedef const char* UriString;

typedef struct SiwsInternalChangelist {
    char delimiter_replaced;
    uint8_t* parser_offset;
} SiwsInternalChangelist;

typedef struct SiwsInternalChangelistWrapper{
    uint8_t number_of_changes;
    SiwsInternalChangelist changelist[RESOURCES_MAX_LENGTH + SIWS_FIELDS_COUNT];
} SiwsInternalChangelistWrapper;


extern SiwsInternalChangelistWrapper G_changelist_wrapper;


//@TODO create better types for these fields
//For now we assume that every field is string-like
typedef struct SiwsMessage {
    const char* domain;
    const char* address;
    const char* statement;
    UriString uri;
    const char* version;
    const char* chain_id;
    const char* nonce;
    DateTimeString issued_at;
    DateTimeString expiration_time;
    DateTimeString not_before;
    const char* request_id;
    //List of uri strings
    UriString resources[RESOURCES_MAX_LENGTH];
} SiwsMessage;

int parse_siws_message(Parser* parser, SiwsMessage* message);
void siws_changelist_rollback();
