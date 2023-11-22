#include "compute_budget_instruction.h"
#include "sol/transaction_summary.h"

const Pubkey compute_budget_program_id = {{PROGRAM_ID_COMPUTE_BUDGET}};

static int parse_compute_budget_instruction_kind(Parser* parser,
                                                 enum ComputeBudgetInstructionKind* kind) {
    uint8_t maybe_kind;
    BAIL_IF(parse_u8(parser, &maybe_kind));
    switch (maybe_kind) {
        case ComputeBudgetRequestHeapFrame:
        case ComputeBudgetChangeUnitLimit:
        case ComputeBudgetChangeUnitPrice:
            *kind = (enum ComputeBudgetInstructionKind) maybe_kind;
            return 0;

        // Deprecated instructions
        case ComputeBudgetRequestUnits:
        default:
            return 1;
    }
}

static int parse_request_heap_frame_instruction(Parser* parser,
                                                ComputeBudgetRequestHeapFrameInfo* info) {
    BAIL_IF(parse_u32(parser, &info->bytes));

    return 0;
}

static int parse_unit_limit_instruction(Parser* parse, ComputeBudgetChangeUnitLimitInfo* info) {
    BAIL_IF(parse_u32(parse, &info->units));

    return 0;
}

static int parse_unit_price_instruction(Parser* parse, ComputeBudgetChangeUnitPriceInfo* info) {
    BAIL_IF(parse_u32(parse, &info->units));

    return 0;
}

int print_compute_budget_unit_price(ComputeBudgetChangeUnitPriceInfo* info) {
    SummaryItemPayload* item_payload = transaction_summary_payload_priority_fees_item();
    summary_item_payload_set_u64(item_payload, info->units);
    return 0;
}

int print_compute_budget_unit_limit(ComputeBudgetChangeUnitLimitInfo* info) {
    SummaryItemPayload* item_payload = transaction_summary_payload_compute_units_limit_item();
    summary_item_payload_set_u64(item_payload, info->units);
    return 0;
}

int print_compute_budget(ComputeBudgetInfo* info) {
    switch (info->kind) {
        case ComputeBudgetChangeUnitLimit:
            print_compute_budget_unit_limit(&info->change_unit_limit);
            break;
        case ComputeBudgetChangeUnitPrice:
            print_compute_budget_unit_price(&info->change_unit_price);
            break;
        case ComputeBudgetRequestUnits:
        case ComputeBudgetRequestHeapFrame:
            break;
    }
    return 0;
}

int parse_compute_budget_instructions(const Instruction* instruction, ComputeBudgetInfo* info) {
    Parser parser = {instruction->data, instruction->data_length};

    BAIL_IF(parse_compute_budget_instruction_kind(&parser, &info->kind));

    switch (info->kind) {
        case ComputeBudgetRequestHeapFrame:
            return parse_request_heap_frame_instruction(&parser, &info->request_heap_frame);
        case ComputeBudgetChangeUnitLimit:
            return parse_unit_limit_instruction(&parser, &info->change_unit_limit);
        case ComputeBudgetChangeUnitPrice:
            return parse_unit_price_instruction(&parser, &info->change_unit_price);

        // Deprecated instructions
        case ComputeBudgetRequestUnits:
        default:
            return 1;
    }
}
