#include "siws.c"
#include "include/sol/parser.h"
#include <assert.h>
#include <stdio.h>
#include "util.h"

void test_validate_chain_id_valid(){
    const char* chain_id = "testnet";
    assert(validate_chain_id(chain_id) == true);

    chain_id = "mainnet";
    assert(validate_chain_id(chain_id) == true);

    chain_id = "devnet";
    assert(validate_chain_id(chain_id) == true);

    chain_id = "localnet";
    assert(validate_chain_id(chain_id) == true);

    chain_id = "solana:mainnet";
    assert(validate_chain_id(chain_id) == true);

    chain_id = "solana:testnet";
    assert(validate_chain_id(chain_id) == true);

    chain_id = "solana:devnet";
    assert(validate_chain_id(chain_id) == true);
}

void test_validate_chain_id_invalid(){
    const char* chain_id = "testnet2";
    assert(validate_chain_id(chain_id) == false);

    chain_id = "qwerty";
    assert(validate_chain_id(chain_id) == false);

    chain_id = "";
    assert(validate_chain_id(chain_id) == false);

    chain_id = "\n";
    assert(validate_chain_id(chain_id) == false);

    chain_id = " ";
    assert(validate_chain_id(chain_id) == false);

    chain_id = "solana:localnet";
    assert(validate_chain_id(chain_id) == false);

    char chain_id2[] = {0x00};
    assert(validate_chain_id(chain_id2) == false);
}

void test_is_alphanumeric_valid(){
    const char* value = "testnet2";
    size_t max_length = 8;
    assert(is_alphanumeric(value, max_length) == true);

    value = "abc4";
    max_length = 10;
    assert(is_alphanumeric(value, max_length) == true);

    char value2[] = {'\0'};
    assert(is_alphanumeric(value2, max_length) == true);

}

void test_is_alphanumeric_invalid(){
    const char* value = "\n";
    size_t max_length = 8;
    assert(is_alphanumeric(value, max_length) == false);

    value = "@testnet";
    assert(is_alphanumeric(value, max_length) == false);

    value = "testnet@";
    assert(is_alphanumeric(value, max_length) == false);

    value = "!@#$";
    assert(is_alphanumeric(value, max_length) == false);

    value = "qwerty with spaces   ";
    max_length = 20;
    assert(is_alphanumeric(value, max_length) == false);

    value = "  short max length";
    max_length = 3;
    assert(is_alphanumeric(value, max_length) == false);

}


void test_is_alphanumeric_max_length(){
    const char* value = "abc#@4";
    size_t max_length = 8;
    assert(is_alphanumeric(value, max_length) == false);

    max_length = 3;
    assert(is_alphanumeric(value, max_length) == true);
}

void test_parse_siws_message_valid(){
    char message[] = "localhost:3001 wants you to sign in with your Solana account:\n"
        "9Pr7yXpVtAVVzEivf8aUNAPDLXXmiHFsMshgjJSySGpn\n"
        "\n"
        "Clicking Sign or Approve only means you have proved this wallet is owned by you. This request will not trigger any blockchain transaction or cost any gas fee.\n"
        "\n"
        "Version: 1\n"
        "Chain ID: mainnet\n"
        "Request ID: 2137\n"
        "Not Before: 2023-10-13 13:33:00\n"
        "Nonce: oBbLoEldZs\n"
        "URI: http://localhost:3001\n"
        "Issued At: 2023-10-10T07:22:44.343Z\n"
        "Expiration Time: 2023-10-13\n"
        "Resources:\n"
        "- https://example.com\n"
        "- https://phantom.app/";

    Parser parser = { (const uint8_t*)message, strlen(message)};
    SiwsMessage siws_msg = {0};

    //Parsing status
    assert(parse_siws_message(&parser, &siws_msg) == 0);

    //Check parsed values
    assert(strcmp(siws_msg.domain, "localhost:3001") == 0);
    assert(strcmp(siws_msg.address, "9Pr7yXpVtAVVzEivf8aUNAPDLXXmiHFsMshgjJSySGpn") == 0);
    assert(strcmp(siws_msg.statement, "Clicking Sign or Approve only means you have proved this wallet is owned by you. This request will not trigger any blockchain transaction or cost any gas fee.") == 0);
    assert(*siws_msg.version == '1');
    assert(strcmp(siws_msg.chain_id, "mainnet") == 0);
    assert(strcmp(siws_msg.request_id, "2137") == 0);
    assert(strcmp(siws_msg.not_before, "2023-10-13 13:33:00") == 0);
    assert(strcmp(siws_msg.nonce, "oBbLoEldZs") == 0);
    assert(strcmp(siws_msg.uri, "http://localhost:3001") == 0);
    assert(strcmp(siws_msg.issued_at, "2023-10-10T07:22:44.343Z") == 0);
    assert(strcmp(siws_msg.expiration_time, "2023-10-13") == 0);
    assert(strcmp(siws_msg.resources[0], "https://example.com") == 0);
    assert(strcmp(siws_msg.resources[1], "https://phantom.app/") == 0);
    assert(siws_msg.resources[2] == NULL);
}

void test_parse_siws_message_valid_resources_location(){
    //Changed resources location and added additional new lines chars at the end
    char message[] = "localhost:3001 wants you to sign in with your Solana account:\n"
        "9Pr7yXpVtAVVzEivf8aUNAPDLXXmiHFsMshgjJSySGpn\n"
        "\n"
        "Clicking Sign or Approve only means you have proved this wallet is owned by you. This request will not trigger any blockchain transaction or cost any gas fee.\n"
        "\n"
        "Resources:\n"
        "- https://example.com\n"
        "- https://phantom.app/\n"
        "Version: 1\n"
        "Chain ID: mainnet\n"
        "Request ID: 2137\n"
        "Not Before: 2023-10-13 13:33:00\n"
        "Nonce: oBbLoEldZs\n"
        "URI: http://localhost:3001\n"
        "Issued At: 2023-10-10T07:22:44.343Z\n"
        "Expiration Time: 2023-10-13\n"
        "\n"
        "";

    Parser parser = { (const uint8_t*)message, strlen(message)};
    SiwsMessage siws_msg = {0};

    //Parsing status
    assert(parse_siws_message(&parser, &siws_msg) == 0);

    //Check parsed values
    assert(strcmp(siws_msg.domain, "localhost:3001") == 0);
    assert(strcmp(siws_msg.address, "9Pr7yXpVtAVVzEivf8aUNAPDLXXmiHFsMshgjJSySGpn") == 0);
    assert(strcmp(siws_msg.statement, "Clicking Sign or Approve only means you have proved this wallet is owned by you. This request will not trigger any blockchain transaction or cost any gas fee.") == 0);
    assert(siws_msg.version != NULL && *siws_msg.version == '1');
    assert(strcmp(siws_msg.chain_id, "mainnet") == 0);
    assert(strcmp(siws_msg.request_id, "2137") == 0);
    assert(strcmp(siws_msg.not_before, "2023-10-13 13:33:00") == 0);
    assert(strcmp(siws_msg.nonce, "oBbLoEldZs") == 0);
    assert(strcmp(siws_msg.uri, "http://localhost:3001") == 0);
    assert(strcmp(siws_msg.issued_at, "2023-10-10T07:22:44.343Z") == 0);
    assert(strcmp(siws_msg.expiration_time, "2023-10-13") == 0);
    assert(strcmp(siws_msg.resources[0], "https://example.com") == 0);
    assert(strcmp(siws_msg.resources[1], "https://phantom.app/") == 0);
    assert(siws_msg.resources[2] == NULL);
}

void test_parse_siws_message_valid_minimal_message(){
    char message[] = "localhost:3001 wants you to sign in with your Solana account:\n"
        "9Pr7yXpVtAVVzEivf8aUNAPDLXXmiHFsMshgjJSySGpn";

    Parser parser = { (const uint8_t*)message, strlen(message)};
    SiwsMessage siws_msg = {0};

    //Parsing status
    assert(parse_siws_message(&parser, &siws_msg) == 0);

    //Check parsed values
    assert(strcmp(siws_msg.domain, "localhost:3001") == 0);
    assert(strcmp(siws_msg.address, "9Pr7yXpVtAVVzEivf8aUNAPDLXXmiHFsMshgjJSySGpn") == 0);
    assert(siws_msg.statement == NULL);
    assert(siws_msg.version == NULL);
    assert(siws_msg.chain_id == NULL);
    assert(siws_msg.request_id == NULL);
    assert(siws_msg.not_before == NULL);
    assert(siws_msg.nonce == NULL);
    assert(siws_msg.uri == NULL);
    assert(siws_msg.issued_at == NULL);
    assert(siws_msg.expiration_time == NULL);
    assert(siws_msg.resources[0] == NULL);

}

void test_parse_siws_message_invalid_minimal_message(){
    //Additional new line at the end of address line
    char message[] = "localhost:3001 wants you to sign in with your Solana account:\n"
        "9Pr7yXpVtAVVzEivf8aUNAPDLXXmiHFsMshgjJSySGpn\n"
        "statement in wrong spot";

    Parser parser = { (const uint8_t*)message, strlen(message)};

    Parser parser_initial_position = parser;

    SiwsMessage siws_msg = {0};

    //Parsing status
    assert(parse_siws_message(&parser, &siws_msg) > 0);

    //Check parsed values

    //SiwsMessage buffer is not zeroed out when parsing fails, it will contain previous values
    assert(siws_msg.domain != NULL);
    assert(siws_msg.address != NULL);

    //Rest of the fields has to be null
    assert(siws_msg.statement == NULL);
    assert(siws_msg.version == NULL);
    assert(siws_msg.chain_id == NULL);
    assert(siws_msg.request_id == NULL);
    assert(siws_msg.not_before == NULL);
    assert(siws_msg.nonce == NULL);
    assert(siws_msg.uri == NULL);
    assert(siws_msg.issued_at == NULL);
    assert(siws_msg.expiration_time == NULL);
    assert(siws_msg.resources[0] == NULL);

    assert(parser.buffer == parser_initial_position.buffer);
    assert(parser.buffer_length == parser_initial_position.buffer_length);
}

void test_parse_siws_message_invalid_version_verify_rollback(){
    char message[] = "localhost:3001 wants you to sign in with your Solana account:\n"
        "9Pr7yXpVtAVVzEivf8aUNAPDLXXmiHFsMshgjJSySGpn\n"
        "\n"
        "Clicking Sign or Approve only means you have proved this wallet is owned by you. This request will not trigger any blockchain transaction or cost any gas fee.\n"
        "\n"
        "Version: 2\n"
        "Chain ID: mainnet\n"
        "Request ID: 2137\n"
        "Not Before: 2023-10-13 13:33:00\n"
        "Nonce: oBbLoEldZs\n"
        "URI: http://localhost:3001\n"
        "Issued At: 2023-10-10T07:22:44.343Z\n"
        "Expiration Time: 2023-10-13\n"
        "Resources:\n"
        "- https://example.com\n"
        "- https://phantom.app/";

    Parser parser = { (const uint8_t*)message, strlen(message)};

    Parser parser_initial_position = parser;

    SiwsMessage siws_msg = {0};

    //Parsing status
    assert(parse_siws_message(&parser, &siws_msg) > 0);

    //Check parsed values
    assert(siws_msg.domain != NULL);
    assert(siws_msg.address != NULL);
    assert(siws_msg.statement != NULL);
    assert(siws_msg.version != NULL);
    assert(siws_msg.chain_id != NULL);
    assert(siws_msg.request_id != NULL);
    assert(siws_msg.not_before != NULL);
    assert(siws_msg.nonce != NULL);
    assert(siws_msg.uri != NULL);
    assert(siws_msg.issued_at != NULL);
    assert(siws_msg.expiration_time != NULL);
    assert(siws_msg.resources[0] != NULL);


    assert(parser.buffer == parser_initial_position.buffer);
    assert(parser.buffer_length == parser_initial_position.buffer_length);

    //Verify that rollback set back previous values
    assert(parser.buffer[14] == ' ');// \0 should be replaced back with space
    assert(parser.buffer[61] == '\n');
    assert(parser.buffer[106] == '\n');
    assert(parser.buffer[107] == '\n');
    assert(parser.buffer[266] == '\n');
    assert(parser.buffer[267] == '\n');
    assert(parser.buffer[278] == '\n');
    assert(parser.buffer[296] == '\n');
    assert(parser.buffer[313] == '\n');
    assert(parser.buffer[345] == '\n');
    assert(parser.buffer[363] == '\n');
    assert(parser.buffer[390] == '\n');
    assert(parser.buffer[426] == '\n');
    assert(parser.buffer[454] == '\n');
    assert(parser.buffer[465] == '\n');
    assert(parser.buffer[487] == '\n');

}

void test_parse_siws_message_invalid_nonce_1(){
    //Nonce min length is 8
    char message[] = "localhost:3001 wants you to sign in with your Solana account:\n"
        "9Pr7yXpVtAVVzEivf8aUNAPDLXXmiHFsMshgjJSySGpn\n"
        "\n"
        "Clicking Sign or Approve only means you have proved this wallet is owned by you. This request will not trigger any blockchain transaction or cost any gas fee.\n"
        "\n"
        "Version: 1\n"
        "Chain ID: mainnet\n"
        "Request ID: 2137\n"
        "Not Before: 2023-10-13 13:33:00\n"
        "Nonce: 1234\n"
        "URI: http://localhost:3001\n"
        "Issued At: 2023-10-10T07:22:44.343Z\n"
        "Expiration Time: 2023-10-13\n"
        "Resources:\n"
        "- https://example.com\n"
        "- https://phantom.app/";

    Parser parser = { (const uint8_t*)message, strlen(message)};

    SiwsMessage siws_msg = {0};

    //Parsing status
    assert(parse_siws_message(&parser, &siws_msg) > 0);

}

void test_parse_siws_message_invalid_nonce_2(){
    char message[] = "localhost:3001 wants you to sign in with your Solana account:\n"
        "9Pr7yXpVtAVVzEivf8aUNAPDLXXmiHFsMshgjJSySGpn\n"
        "\n"
        "Clicking Sign or Approve only means you have proved this wallet is owned by you. This request will not trigger any blockchain transaction or cost any gas fee.\n"
        "\n"
        "Version: 1\n"
        "Chain ID: mainnet\n"
        "Request ID: 2137\n"
        "Not Before: 2023-10-13 13:33:00\n"
        "Nonce: @notalphanum!!\n"
        "URI: http://localhost:3001\n"
        "Issued At: 2023-10-10T07:22:44.343Z\n"
        "Expiration Time: 2023-10-13\n"
        "Resources:\n"
        "- https://example.com\n"
        "- https://phantom.app/";

    Parser parser = { (const uint8_t*)message, strlen(message)};

    SiwsMessage siws_msg = {0};

    //Parsing status
    assert(parse_siws_message(&parser, &siws_msg) > 0);

}

void test_parse_siws_message_invalid_address(){
    char message[] = "localhost:3001 wants you to sign in with your Solana account:\n"
        "9Pr7yXpMshgjJSySGpn\n"
        "\n"
        "Clicking Sign or Approve only means you have proved this wallet is owned by you. This request will not trigger any blockchain transaction or cost any gas fee.\n"
        "\n"
        "Version: 1\n"
        "Chain ID: mainnet\n"
        "Request ID: 2137\n"
        "Not Before: 2023-10-13 13:33:00\n"
        "Nonce: randomNonce\n"
        "URI: http://localhost:3001\n"
        "Issued At: 2023-10-10T07:22:44.343Z\n"
        "Expiration Time: 2023-10-13\n"
        "Resources:\n"
        "- https://example.com\n"
        "- https://phantom.app/";

    Parser parser = { (const uint8_t*)message, strlen(message)};

    SiwsMessage siws_msg = {0};

    //Parsing status
    assert(parse_siws_message(&parser, &siws_msg) > 0);

}

void test_parse_siws_message_invalid_chain_id(){
    char message[] = "localhost:3001 wants you to sign in with your Solana account:\n"
        "9Pr7yXpVtAVVzEivf8aUNAPDLXXmiHFsMshgjJSySGpn\n"
        "\n"
        "Clicking Sign or Approve only means you have proved this wallet is owned by you. This request will not trigger any blockchain transaction or cost any gas fee.\n"
        "\n"
        "Version: 1\n"
        "Chain ID: mynet\n"
        "Request ID: 2137\n"
        "Not Before: 2023-10-13 13:33:00\n"
        "Nonce: oBbLoEldZs\n"
        "URI: http://localhost:3001\n"
        "Issued At: 2023-10-10T07:22:44.343Z\n"
        "Expiration Time: 2023-10-13\n"
        "Resources:\n"
        "- https://example.com\n"
        "- https://phantom.app/";

    Parser parser = { (const uint8_t*)message, strlen(message)};

    SiwsMessage siws_msg = {0};

    //Parsing status
    assert(parse_siws_message(&parser, &siws_msg) > 0);

}

void test_parse_siws_message_invalid_separator_1(){
    //Request id does not have ':' separating the value
    char message[] = "localhost:3001 wants you to sign in with your Solana account:\n"
        "9Pr7yXpVtAVVzEivf8aUNAPDLXXmiHFsMshgjJSySGpn\n"
        "\n"
        "Clicking Sign or Approve only means you have proved this wallet is owned by you. This request will not trigger any blockchain transaction or cost any gas fee.\n"
        "\n"
        "Version: 1\n"
        "Chain ID: mainnet\n"
        "Request ID 2137\n"
        "Not Before: 2023-10-13 13:33:00\n"
        "Nonce: oBbLoEldZs\n"
        "URI: http://localhost:3001\n"
        "Issued At: 2023-10-10T07:22:44.343Z\n"
        "Expiration Time: 2023-10-13\n"
        "Resources:\n"
        "- https://example.com\n"
        "- https://phantom.app/";

    Parser parser = { (const uint8_t*)message, strlen(message)};

    SiwsMessage siws_msg = {0};

    //Parsing status
    assert(parse_siws_message(&parser, &siws_msg) > 0);

}

void test_parse_siws_message_invalid_separator_2(){
    //Request id does not have ' ' separating the value
    char message[] = "localhost:3001 wants you to sign in with your Solana account:\n"
        "9Pr7yXpVtAVVzEivf8aUNAPDLXXmiHFsMshgjJSySGpn\n"
        "\n"
        "Clicking Sign or Approve only means you have proved this wallet is owned by you. This request will not trigger any blockchain transaction or cost any gas fee.\n"
        "\n"
        "Version: 1\n"
        "Chain ID: mainnet\n"
        "Request ID:2137\n"
        "Not Before: 2023-10-13 13:33:00\n"
        "Nonce: oBbLoEldZs\n"
        "URI: http://localhost:3001\n"
        "Issued At: 2023-10-10T07:22:44.343Z\n"
        "Expiration Time: 2023-10-13\n"
        "Resources:\n"
        "- https://example.com\n"
        "- https://phantom.app/";

    Parser parser = { (const uint8_t*)message, strlen(message)};

    SiwsMessage siws_msg = {0};

    //Parsing status
    assert(parse_siws_message(&parser, &siws_msg) > 0);

}

void test_parse_siws_message_invalid_resources_prefix_1(){
    char message[] = "localhost:3001 wants you to sign in with your Solana account:\n"
        "9Pr7yXpVtAVVzEivf8aUNAPDLXXmiHFsMshgjJSySGpn\n"
        "\n"
        "Clicking Sign or Approve only means you have proved this wallet is owned by you. This request will not trigger any blockchain transaction or cost any gas fee.\n"
        "\n"
        "Version: 1\n"
        "Chain ID: mainnet\n"
        "Request ID: 2137\n"
        "Not Before: 2023-10-13 13:33:00\n"
        "Nonce: oBbLoEldZs\n"
        "URI: http://localhost:3001\n"
        "Issued At: 2023-10-10T07:22:44.343Z\n"
        "Expiration Time: 2023-10-13\n"
        "Resources:\n"
        " https://example.com\n"
        "- https://phantom.app/";

    Parser parser = { (const uint8_t*)message, strlen(message)};

    SiwsMessage siws_msg = {0};

    //Parsing status
    assert(parse_siws_message(&parser, &siws_msg) > 0);

}

void test_parse_siws_message_invalid_resources_prefix_2(){
    char message[] = "localhost:3001 wants you to sign in with your Solana account:\n"
        "9Pr7yXpVtAVVzEivf8aUNAPDLXXmiHFsMshgjJSySGpn\n"
        "\n"
        "Clicking Sign or Approve only means you have proved this wallet is owned by you. This request will not trigger any blockchain transaction or cost any gas fee.\n"
        "\n"
        "Version: 1\n"
        "Chain ID: mainnet\n"
        "Request ID: 2137\n"
        "Not Before: 2023-10-13 13:33:00\n"
        "Nonce: oBbLoEldZs\n"
        "URI: http://localhost:3001\n"
        "Issued At: 2023-10-10T07:22:44.343Z\n"
        "Expiration Time: 2023-10-13\n"
        "Resources:\n"
        "https://example.com\n"
        "- https://phantom.app/";

    Parser parser = { (const uint8_t*)message, strlen(message)};

    SiwsMessage siws_msg = {0};

    //Parsing status
    assert(parse_siws_message(&parser, &siws_msg) > 0);

}

void test_parse_siws_message_valid_full_resources_table(){
    char message[] = "localhost:3001 wants you to sign in with your Solana account:\n"
        "9Pr7yXpVtAVVzEivf8aUNAPDLXXmiHFsMshgjJSySGpn\n"
        "\n"
        "Clicking Sign or Approve only means you have proved this wallet is owned by you. This request will not trigger any blockchain transaction or cost any gas fee.\n"
        "\n"
        "Resources:\n"
        "- https://example.com\n"
        "- https://phantom.app/\n"
        "- https://example3.com\n"
        "- https://example4.com\n"
        "- https://example5.com\n"
        "- https://example6.com\n"
        "- https://example7.com\n"
        "- https://example8.com\n"
        "- https://example9.com\n"
        "- https://example10.com";

    Parser parser = { (const uint8_t*)message, strlen(message)};
    SiwsMessage siws_msg = {0};

    //Parsing status
    assert(parse_siws_message(&parser, &siws_msg) == 0);

    //Check parsed values
    assert(strcmp(siws_msg.domain, "localhost:3001") == 0);
    assert(strcmp(siws_msg.address, "9Pr7yXpVtAVVzEivf8aUNAPDLXXmiHFsMshgjJSySGpn") == 0);
    assert(strcmp(siws_msg.statement, "Clicking Sign or Approve only means you have proved this wallet is owned by you. This request will not trigger any blockchain transaction or cost any gas fee.") == 0);

    assert(siws_msg.version == NULL);
    assert(siws_msg.chain_id == NULL);
    assert(siws_msg.request_id == NULL);
    assert(siws_msg.not_before == NULL);
    assert(siws_msg.nonce == NULL);
    assert(siws_msg.uri == NULL);
    assert(siws_msg.issued_at == NULL);
    assert(siws_msg.expiration_time == NULL);

    assert(strcmp(siws_msg.resources[0], "https://example.com") == 0);
    assert(strcmp(siws_msg.resources[1], "https://phantom.app/") == 0);
    assert(strcmp(siws_msg.resources[2], "https://example3.com") == 0);
    assert(strcmp(siws_msg.resources[3], "https://example4.com") == 0);
    assert(strcmp(siws_msg.resources[4], "https://example5.com") == 0);
    assert(strcmp(siws_msg.resources[5], "https://example6.com") == 0);
    assert(strcmp(siws_msg.resources[6], "https://example7.com") == 0);
    assert(strcmp(siws_msg.resources[7], "https://example8.com") == 0);
    assert(strcmp(siws_msg.resources[8], "https://example9.com") == 0);
    assert(strcmp(siws_msg.resources[9], "https://example10.com") == 0);
}


void test_parse_siws_message_invalid_required_fields_not_present(){
    char message[] = "localhost:3001 wants you to sign in with your Solana account:\n";

    Parser parser = { (const uint8_t*)message, strlen(message)};

    SiwsMessage siws_msg = {0};

    //Parsing status
    assert(parse_siws_message(&parser, &siws_msg) > 0);

}

void test_parse_siws_message_invalid_required_fields_no_space(){
    char message[] = "localhost:3001\n"
        "9Pr7yXpVtAVVzEivf8aUNAPDLXXmiHFsMshgjJSySGpn";

    Parser parser = { (const uint8_t*)message, strlen(message)};

    SiwsMessage siws_msg = {0};

    //Parsing status
    assert(parse_siws_message(&parser, &siws_msg) > 0);

}


int main() {
    test_is_alphanumeric_valid();
    test_is_alphanumeric_max_length();
    test_is_alphanumeric_invalid();
    test_validate_chain_id_valid();
    test_validate_chain_id_invalid();

    test_parse_siws_message_valid();
    test_parse_siws_message_valid_minimal_message();
    test_parse_siws_message_valid_resources_location();
    test_parse_siws_message_valid_full_resources_table();
    test_parse_siws_message_invalid_minimal_message();
    test_parse_siws_message_invalid_version_verify_rollback();
    test_parse_siws_message_invalid_nonce_1();
    test_parse_siws_message_invalid_nonce_2();
    test_parse_siws_message_invalid_address();
    test_parse_siws_message_invalid_chain_id();
    test_parse_siws_message_invalid_required_fields_not_present();
    test_parse_siws_message_invalid_separator_1();
    test_parse_siws_message_invalid_separator_2();
    test_parse_siws_message_invalid_resources_prefix_1();
    test_parse_siws_message_invalid_resources_prefix_2();
    test_parse_siws_message_invalid_required_fields_no_space();

    printf("passed\n");
    return 0;
}