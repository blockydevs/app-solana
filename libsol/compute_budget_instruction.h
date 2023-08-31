#pragma once

#include "sol/parser.h"

extern const Pubkey compute_budget_program_id;

enum ComputeBudgetInstructionKind {
    ComputeBudgetChangeUnitLimit = 2,
    ComputeBudgetChangeUnitPrice
};

typedef struct ComputeBudgetUnitLimit {
    uint32_t lamports;
} ComputeBudgetUnitLimit;

typedef struct ComputeBudgetUnitPrice {
    uint32_t lamports;
} ComputeBudgetUnitPrice;


int parse_compute_budget_instructions(const Instruction* instruction, const MessageHeader* header, SystemInfo* info);


