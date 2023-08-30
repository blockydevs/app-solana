#include "common_byte_strings.h"
#include "instruction.h"
#include "sol/parser.h"
#include "compute_budget_instruction.h"
#include "util.h"
#include <string.h>

const Pubkey compute_budget_program_id = {{PROGRAM_ID_COMPUTE_BUDGET}};

static int parse_compute_budget_instruction_kind(Parser* parser,
                                                 enum ComputeBudgetInstructionKind* kind) {
    uint8_t maybe_kind;
    BAIL_IF(parse_u8(parser, &maybe_kind));
    switch (maybe_kind) {
        case ComputeBudgetChangeUnitLimit:
        case ComputeBudgetChangeUnitPrice:
            *kind = (enum ComputeBudgetInstructionKind) maybe_kind;
            return 0;
        default:
            return 1;
    }
}

int parse_compute_budget_instructions(const Instruction* instruction,
                                      const MessageHeader* header,
                                      SystemInfo* info) {
    Parser parser = {instruction->data, instruction->data_length};

    enum ComputeBudgetInstructionKind tmpKind;

    parse_compute_budget_instruction_kind(&parser, &tmpKind);

    PRINTF("Kind: %d", tmpKind);

    return 0;
}