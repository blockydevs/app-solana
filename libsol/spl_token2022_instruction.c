#include "spl_token2022_instruction.h"
#include "sol/print_config.h"
#include "common_byte_strings.h"


const Pubkey spl_token2022_program_id = {{PROGRAM_ID_SPL_TOKEN_2022}};


/**
 * Use current printers from spl token when single instruction is passed
 */
int print_token2022_info(const SplTokenInfo* info, const PrintConfig* print_config){
    return print_spl_token_info(info, print_config);
}
