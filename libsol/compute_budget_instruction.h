#pragma once

#include "common_byte_strings.h"
#include "util.h"
#include "sol/parser.h"

extern const Pubkey compute_budget_program_id;

enum ComputeBudgetInstructionKind {
    ComputeBudgetRequestUnits = 0,
    ComputeBudgetRequestHeapFrame,
    ComputeBudgetChangeUnitLimit,
    ComputeBudgetChangeUnitPrice
};

typedef struct ComputeBudgetRequestHeapFrameInfo {
    uint32_t bytes;
} ComputeBudgetRequestHeapFrameInfo;

typedef struct ComputeBudgetChangeUnitLimitInfo {
    uint32_t units;
} ComputeBudgetChangeUnitLimitInfo;

typedef struct ComputeBudgetChangeUnitPriceInfo {
    uint32_t units;
} ComputeBudgetChangeUnitPriceInfo;

typedef struct ComputeBudgetInfo {
    enum ComputeBudgetInstructionKind kind;
    union {
        ComputeBudgetRequestHeapFrameInfo request_heap_frame;
        ComputeBudgetChangeUnitLimitInfo change_unit_limit;
        ComputeBudgetChangeUnitPriceInfo change_unit_price;
    };
} ComputeBudgetInfo;

int parse_compute_budget_instructions(const Instruction* instruction, ComputeBudgetInfo* info);

int print_compute_budget_unit_price(ComputeBudgetChangeUnitPriceInfo* info);

int print_compute_budget_unit_limit(ComputeBudgetChangeUnitLimitInfo* info);

int print_compute_budget(ComputeBudgetInfo* info);
