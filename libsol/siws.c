#include "include/sol/siws.h"
#include <string.h>
#include "util.h"
#include <ctype.h>

#define VALID_CHAIN_IDS_COUNT 7
#define VALID_CHAIN_IDS_MAX_LENGTH 15

SiwsInternalChangelistWrapper G_changelist_wrapper;

//FIXME - change to list of pointers
const char valid_chain_ids[7][15] = {
    "mainnet",
    "testnet",
    "devnet",
    "localnet",
    "solana:mainnet",
    "solana:testnet",
    "solana:devnet"
};

/**
 * Append new modification to the changelist
 */
void siws_changelist_append(char delimiter_used, char* delimiter_ptr){
    SiwsInternalChangelistWrapper *changelist = &G_changelist_wrapper;
    SiwsInternalChangelist* changeListItem = &changelist->changelist[changelist->number_of_changes++];
    changeListItem->delimiter_replaced = delimiter_used;
    changeListItem->parser_offset = (uint8_t*) delimiter_ptr;
}

static int parse_to_char(Parser* parser, char delimiter_char, const char** value){
    //Check if message contains at least two characters (message part + one space)
    BAIL_IF(check_buffer_length(parser, 2));
    //Do not search further than buffer_length
    char* delimiter_char_location = memchr(parser->buffer, delimiter_char, parser->buffer_length);

    //Value is set even if delimiter char is not found - it will be set to the end of the buffer
    *value = (const char*) parser->buffer;

    //Delimiter char not found till the end of the buffer
    if(delimiter_char_location != NULL){
        *delimiter_char_location = '\0';//Mark string end
        siws_changelist_append(delimiter_char, delimiter_char_location);
        unsigned int advance_length = strlen(*value) + 1;
        advance(parser, advance_length);
    }else{
        //Advance to the end of the buffer
        advance(parser, parser->buffer_length);
    }

    return 0;
}

/**
 * Rollbacks all changes contained in the changelist wrapper
 */
void siws_changelist_rollback(){
    SiwsInternalChangelistWrapper *changelist = &G_changelist_wrapper;
    for(int i = 0; i < changelist->number_of_changes; ++i){
        SiwsInternalChangelist* changelist_item = &changelist->changelist[i];
        *changelist_item->parser_offset = changelist_item->delimiter_replaced;
    }
}

/**
 * Parse string up to the first space
 */
int parse_to_space(Parser* parser, const char** value){
    return parse_to_char(parser, ' ', value);
}

/**
 * Parse string up to the first new line
 */
int parse_to_new_line(Parser* parser, const char** value){
    return parse_to_char(parser, '\n', value);
}

/**
 * Move parser pointer to the first occurrence of the new line char or to the end of the buffer
 * @param check_if_empty - if true, function will return 1 if buffer is not empty
 */
int skip_line(Parser* parser, bool check_if_empty){
    BAIL_IF(check_buffer_length(parser, 2));

    char* lf_char = memchr(parser->buffer, '\n', parser->buffer_length);
    if(lf_char == NULL){
        return 1;
    }

    //Set is as zero to be able to use strlen to calculate pointer advance length
    *lf_char = '\0';
    siws_changelist_append('\n', lf_char);

    size_t buffer_length = strlen((const char*) parser->buffer);

    if(check_if_empty && buffer_length > 0){
        return 1;
    }

    size_t advance_length = buffer_length + 1;
    advance(parser, advance_length);

    return 0;
}

static int compare_set_named_value(const char* line_values, const char* field_name, const char** value){
    size_t field_name_length = strlen(field_name);

    //Check if line starts with given field name
    if(strlen(line_values) >= field_name_length && strncmp(line_values, field_name, field_name_length) == 0){

        if(line_values[field_name_length] != ':'){
            //Values must be separated with ":"
            return 1;
        }

        if(line_values[field_name_length + 1] != ' '){
            //Space must be present
            return 1;
        }

        //Skip field name and colon
        line_values += field_name_length + 2;
        *value = line_values;
        return 0;
    }

    //Value not found, ignore it. Advanced fields are not required
    return 0;
}

static int set_resource_values(Parser* parser, const char* line_values, UriString* uri_resources){
    const char* resources_field_name = "Resources";
    uint8_t resource_index = 0;

    if(strncmp(line_values, resources_field_name, strlen(resources_field_name)) == 0){
        const char* line_value = NULL;
        bool first_line = true;

        while(resource_index < RESOURCES_MAX_LENGTH){
            if(parser->buffer_length > 2 && strncmp((const char*) parser->buffer, "- ", 2) == 0 && parse_to_new_line(parser, &line_value) == 0){
                // Skip "- "
                line_value += 2;

                uri_resources[resource_index] = line_value;
            }else{
                if(first_line){
                    //Next line after "Resources:" must start with "- " prefix otherwise message is invalid
                    return 1;
                }else{
                    //End of resources - exit loop
                    return 0;
                }
            }
            first_line = false;
            resource_index++;
        }

    }


    return 0;
}

/**
 * Check if string contains only alphanumeric characters
 */
bool is_alphanumeric(const char* message, size_t max_message_size){
    for (size_t char_index = 0; char_index < max_message_size; ++char_index) {
        if(message[char_index] == '\0'){
            //End of string, no need to parse it further
            break;
        }
        if(!isalnum( (int) message[char_index])){
            return false;
        }
    }
    return true;
}

bool validate_chain_id(const char* chain_id){
    for (size_t i = 0; i < VALID_CHAIN_IDS_COUNT; ++i) {
        //Compare at most 15 bytes - chain id is never longer
        if(strncmp(chain_id, (const char*) &valid_chain_ids[i], VALID_CHAIN_IDS_MAX_LENGTH) == 0){
            return true;
        }
    }
    return false;
}

int validate_siws_message(SiwsMessage* message, size_t max_message_size){

    //Domain & address are required
    if(message->domain == NULL || message->address == NULL){
        return 1;
    }


    //Optional field
    //Check if version is equal to '1'
    //Currently this is the only supported version
    if(message->version != NULL && strcmp(message->version, "1") != 0){
        return 1;
    }

    //Optional field
    //Nonce has to be at least 8 bytes and be alphanumeric
    if(message->nonce != NULL && (strnlen(message->nonce, max_message_size) < 8 || !is_alphanumeric(message->nonce, max_message_size))){
        return 1;
    }

    //Optional field
    //Statement has to be at least 1 byte long
    if(message->statement != NULL && strnlen(message->statement, max_message_size) == 0){
        return 1;
    }

    //Optional field
    //Address has to be between 32 and 44 bytes long and be alphanumeric
    if(message->address == NULL
        || strnlen(message->address, max_message_size) < 32
        || strnlen(message->address, max_message_size) > 44
        || !is_alphanumeric(message->address, max_message_size)
        ){
            return 1;
    }

    //Optional field
    //Chain id has to be one of the predefined values
    if(message->chain_id != NULL && !validate_chain_id(message->chain_id)){
        return 1;
    }


    return 0;
}

/**
 * Search for field with given name and try to parse it's value
 * Field name is case sensitive
 */
int parse_advanced_fields(Parser* parser, SiwsMessage* message){
    //Iterate over all lines
    const char* line_value = NULL;

    while (parse_to_new_line(parser, &line_value) == 0){
        BAIL_IF(compare_set_named_value(line_value, "URI", &message->uri));
        BAIL_IF(compare_set_named_value(line_value, "Version", &message->version));
        BAIL_IF(compare_set_named_value(line_value, "Chain ID", &message->chain_id));
        BAIL_IF(compare_set_named_value(line_value, "Nonce", &message->nonce));
        BAIL_IF(compare_set_named_value(line_value, "Issued At", &message->issued_at));
        BAIL_IF(compare_set_named_value(line_value, "Expiration Time", &message->expiration_time));
        BAIL_IF(compare_set_named_value(line_value, "Not Before", &message->not_before));
        BAIL_IF(compare_set_named_value(line_value, "Request ID", &message->request_id));
        BAIL_IF(set_resource_values(parser, line_value, message->resources));
    }

    return 0;
}

/**
 * Entire siws message is no longer than apdu buffer
 * Most of the fields don't have max length
 * Possible message structure (extended form):
    ${domain} wants you to sign in with your Solana account:
    ${address}

    ${statement}

    URI: ${uri}
    Version: ${version}
    Chain ID: ${chain-id}
    Nonce: ${nonce}
    Issued At: ${issued-at}
    Expiration Time: ${expiration-time}
    Not Before: ${not-before}
    Request ID: ${request-id}
    Resources:
    - ${resources[0]}
    - ${resources[1]}
    ...
    - ${resources[n]}
 * @see https://github.com/phantom/sign-in-with-solana#abnf-message-format
*/
static int internal_parse_siws_message(Parser* parser, SiwsMessage* message){
    size_t max_message_size = parser->buffer_length;

    //Read domain
    BAIL_IF(parse_to_space(parser, &message->domain));

    //Ignore "wants you to sign in with your Solana account:" part
    BAIL_IF(skip_line(parser, false));

    //Read address
    BAIL_IF(parse_to_new_line(parser, &message->address));


//FIXME - remove this when memory issues will be resolved
//NanoS will not parse and display advanced fields (including statement)
#if !defined(SDK_TARGET_NANOS)
    //Minimal required message consists of domain and address. Check if further parsing is required
    if(parser->buffer_length > 1){
        //Skip empty line
        BAIL_IF(skip_line(parser, true));

        //Read statement
        BAIL_IF(parse_to_new_line(parser, &message->statement));

        //Skip empty line
        BAIL_IF(skip_line(parser, true));

        //Read advanced fields
        BAIL_IF(parse_advanced_fields(parser, message));
    }
#endif

    BAIL_IF(validate_siws_message(message, max_message_size));

    return 0;
}

int parse_siws_message(Parser* parser, SiwsMessage* message){
    //Mark parser position before trying to parse siws message
    Parser siws_parser_copy = {parser->buffer, parser->buffer_length};

    int result = internal_parse_siws_message(parser, message);

    if(result == 1){
        //Rollback parser to the position before trying to parse siws message
        *parser = siws_parser_copy;

        siws_changelist_rollback();
    }

    return result;
}