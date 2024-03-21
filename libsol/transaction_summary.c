#include "sol/parser.h"
#include "sol/printer.h"
#include "sol/transaction_summary.h"
#include "util.h"
#include <string.h>

struct SummaryItem {
    const char* title;
    enum SummaryItemKind kind;
    union {
        uint64_t u64;
        int64_t i64;
        const Pubkey* pubkey;
        const Hash* hash;
        const char* string;
        SizedString sized_string;
        TokenAmount token_amount;
        const OffchainMessageApplicationDomain* application_domain;
    };
};

void summary_item_set_amount(SummaryItem* item, const char* title, uint64_t value) {
    item->kind = SummaryItemAmount;
    item->title = title;
    item->u64 = value;
}

void summary_item_set_token_amount(SummaryItem* item,
                                   const char* title,
                                   uint64_t value,
                                   const char* symbol,
                                   uint8_t decimals) {
    item->kind = SummaryItemTokenAmount;
    item->title = title;
    item->token_amount.value = value;
    item->token_amount.symbol = symbol;
    item->token_amount.decimals = decimals;
}

void summary_item_set_i64(SummaryItem* item, const char* title, int64_t value) {
    item->kind = SummaryItemI64;
    item->title = title;
    item->i64 = value;
}

void summary_item_set_u64(SummaryItem* item, const char* title, uint64_t value) {
    item->kind = SummaryItemU64;
    item->title = title;
    item->u64 = value;
}

void summary_item_set_pubkey(SummaryItem* item, const char* title, const Pubkey* value) {
    item->kind = SummaryItemPubkey;
    item->title = title;
    item->pubkey = value;
}

void summary_item_set_hash(SummaryItem* item, const char* title, const Hash* value) {
    item->kind = SummaryItemHash;
    item->title = title;
    item->hash = value;
}

void summary_item_set_sized_string(SummaryItem* item, const char* title, const SizedString* value) {
    item->kind = SummaryItemSizedString;
    item->title = title;
    item->sized_string.length = value->length;
    item->sized_string.string = value->string;
}

/**
 * Set the string value of a SummaryItem, but only if the value is not NULL.
 */
void summary_item_safe_set_string(SummaryItem* item, const char* title, const char* value) {
    if(value != NULL){
        summary_item_set_string(item, title, value);
    }
}

void summary_item_set_string(SummaryItem* item, const char* title, const char* value) {
    item->kind = SummaryItemString;
    item->title = title;
    item->string = value;
}

void summary_item_safe_set_extended_string(SummaryItem* item, const char* title, const char* value){
    if(value != NULL){
        summary_item_set_extended_string(item, title, value);
    }
}

void summary_item_set_extended_string(SummaryItem* item, const char* title, const char* value) {
    item->kind = SummaryItemExtendedString;
    item->title = title;
    item->string = value;
}

void summary_item_set_timestamp(SummaryItem* item, const char* title, int64_t value) {
    item->kind = SummaryItemTimestamp;
    item->title = title;
    item->i64 = value;
}

void summary_item_set_offchain_message_application_domain(SummaryItem* item,
                                                          const char* title,
                                                          const OffchainMessageApplicationDomain* value) {
    item->kind = SummaryItemOffchainMessageApplicationDomain;
    item->title = title;
    item->application_domain = value;
}

typedef struct TransactionSummary {
    SummaryItem primary;
    SummaryItem fee_payer;
    SummaryItem nonce_account;
    SummaryItem nonce_authority;
    SummaryItem general[NUM_GENERAL_ITEMS];
} TransactionSummary;

static TransactionSummary G_transaction_summary;

char G_transaction_summary_title[TITLE_SIZE];
char G_transaction_summary_text[TEXT_BUFFER_LENGTH];

char* G_transaction_summary_extended_text;


void transaction_summary_reset() {
    explicit_bzero(&G_transaction_summary, sizeof(TransactionSummary));
    explicit_bzero(&G_transaction_summary_title, TITLE_SIZE);
    explicit_bzero(&G_transaction_summary_text, TEXT_BUFFER_LENGTH);
}

static bool is_summary_item_used(const SummaryItem* item) {
    return (item->kind != SummaryItemNone);
}

static SummaryItem* summary_item_as_unused(SummaryItem* item) {
    if (!is_summary_item_used(item)) {
        return item;
    }
    return NULL;
}

SummaryItem* transaction_summary_primary_item() {
    SummaryItem* item = &G_transaction_summary.primary;
    return summary_item_as_unused(item);
}

SummaryItem* transaction_summary_fee_payer_item() {
    SummaryItem* item = &G_transaction_summary.fee_payer;
    return summary_item_as_unused(item);
}

SummaryItem* transaction_summary_nonce_account_item() {
    SummaryItem* item = &G_transaction_summary.nonce_account;
    return summary_item_as_unused(item);
}

SummaryItem* transaction_summary_nonce_authority_item() {
    SummaryItem* item = &G_transaction_summary.nonce_authority;
    return summary_item_as_unused(item);
}

/*
 * Returns first available unused general item
 */
SummaryItem* transaction_summary_general_item() {
    for (size_t i = 0; i < NUM_GENERAL_ITEMS; i++) {
        SummaryItem* item = &G_transaction_summary.general[i];
        if (!is_summary_item_used(item)) {
            return item;
        }
    }
    return NULL;
}

/*
 * Returns number of used general items
 */
uint8_t transaction_summary_general_item_count() {
    uint8_t count = 0;
    for (size_t i = 0; i < NUM_GENERAL_ITEMS; i++) {
        SummaryItem* item = &G_transaction_summary.general[i];
        if (is_summary_item_used(item)) {
        count++;
        }
    }
    return count;
}

#define FEE_PAYER_TITLE "Fee payer"
int transaction_summary_set_fee_payer_pubkey(const Pubkey* pubkey) {
    SummaryItem* item = transaction_summary_fee_payer_item();
    BAIL_IF(item == NULL);
    summary_item_set_pubkey(item, FEE_PAYER_TITLE, pubkey);
    return 0;
}

/*
 * Suppress warning about const qualifier - changes required in ledger's SDK
 * warning suppression is used on purpose to avoid casting pointers without const qualifier
 * G_ux.externalText is not modified anywhere
 */
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wcast-qual"
static void set_extended_string(const char* value){
    //Set pointer to the temporary buffer, so 'libsol' does not have to include ux API from SDK
    G_transaction_summary_extended_text = (char*) value;
}
#pragma GCC diagnostic pop

static int transaction_summary_update_display_for_item(const SummaryItem* item,
                                                       enum DisplayFlags flags) {
    switch (item->kind) {
        case SummaryItemNone:
            return 1;
        case SummaryItemAmount:
            BAIL_IF(print_amount(item->u64, G_transaction_summary_text, BASE58_PUBKEY_LENGTH));
            break;
        case SummaryItemTokenAmount:
            BAIL_IF(print_token_amount(item->token_amount.value,
                                       item->token_amount.symbol,
                                       item->token_amount.decimals,
                                       G_transaction_summary_text,
                                       TEXT_BUFFER_LENGTH));
            break;
        case SummaryItemI64:
            BAIL_IF(print_i64(item->i64, G_transaction_summary_text, TEXT_BUFFER_LENGTH));
            break;
        case SummaryItemU64:
            BAIL_IF(print_u64(item->u64, G_transaction_summary_text, TEXT_BUFFER_LENGTH));
            break;
        case SummaryItemPubkey: {
            char tmp_buf[BASE58_PUBKEY_LENGTH];
            BAIL_IF(encode_base58(item->pubkey, PUBKEY_SIZE, tmp_buf, sizeof(tmp_buf)));
            if (flags & DisplayFlagLongPubkeys) {
                BAIL_IF(print_string(tmp_buf, G_transaction_summary_text, TEXT_BUFFER_LENGTH));
            } else {
                BAIL_IF(print_summary(tmp_buf,
                                      G_transaction_summary_text,
                                      BASE58_PUBKEY_SHORT,
                                      SUMMARY_LENGTH,
                                      SUMMARY_LENGTH));
            }
            break;
        }
        case SummaryItemHash:
            BAIL_IF(encode_base58(item->hash,
                                  BLOCKHASH_SIZE,
                                  G_transaction_summary_text,
                                  TEXT_BUFFER_LENGTH));
            break;
        case SummaryItemExtendedString:
            set_extended_string(item->string);
            break;
        case SummaryItemString:
            print_string(item->string, G_transaction_summary_text, TEXT_BUFFER_LENGTH);
            break;
        case SummaryItemSizedString:
            print_sized_string(&item->sized_string, G_transaction_summary_text, TEXT_BUFFER_LENGTH);
            break;
        case SummaryItemTimestamp:
            BAIL_IF(print_timestamp(item->i64, G_transaction_summary_text, TEXT_BUFFER_LENGTH));
            break;
        case SummaryItemOffchainMessageApplicationDomain:
            BAIL_IF(encode_base58(item->application_domain,
                                  OFFCHAIN_MESSAGE_APPLICATION_DOMAIN_LENGTH,
                                  G_transaction_summary_text,
                                  TEXT_BUFFER_LENGTH));
            break;
    }
    print_string(item->title, G_transaction_summary_title, TITLE_SIZE);
    return 0;
}

// find item_index in G_transaction_summary in the following order:
//     summary->primary
//     used items of summary->general[]
//     used summary->nonce_account
//     used summary->nonce_authority
//     summary->fee_payer
static SummaryItem* transaction_summary_find_item(size_t item_index) {
    struct TransactionSummary* summary = &G_transaction_summary;
    size_t current_index = 0;

    if (current_index == item_index) {
        return &summary->primary;
    }
    ++current_index;

    for (size_t i = 0; i < NUM_GENERAL_ITEMS; i++) {
        if (is_summary_item_used(&summary->general[i])) {
            if (current_index == item_index) {
                return &summary->general[i];
            }
            ++current_index;
        }
    }

    if (is_summary_item_used(&summary->nonce_account)) {
        if (current_index == item_index) {
            return &summary->nonce_account;
        }
        ++current_index;
    }

    if (is_summary_item_used(&summary->nonce_authority)) {
        if (current_index == item_index) {
            return &summary->nonce_authority;
        }
        ++current_index;
    }

    if (current_index == item_index) {
        return &summary->fee_payer;
    }

    return NULL;
}

int transaction_summary_display_item(size_t item_index, enum DisplayFlags flags) {
    const SummaryItem* item;

    item = transaction_summary_find_item(item_index);
    if (item == NULL) {
        return 1;
    }

    return transaction_summary_update_display_for_item(item, flags);
}

#define SET_IF_USED(item, item_kinds, index) \
    do {                                     \
        if (item.kind != SummaryItemNone) {  \
            item_kinds[index++] = item.kind; \
        }                                    \
    } while (0)

int transaction_summary_finalize(enum SummaryItemKind* item_kinds, size_t* item_kinds_len) {
    const TransactionSummary* summary = &G_transaction_summary;
    size_t index = 0;

    if (summary->primary.kind == SummaryItemNone) {
        return 1;
    }

    SET_IF_USED(summary->primary, item_kinds, index);

    for (size_t i = 0; i < NUM_GENERAL_ITEMS; i++) {
        SET_IF_USED(summary->general[i], item_kinds, index);
    }

    SET_IF_USED(summary->nonce_account, item_kinds, index);
    SET_IF_USED(summary->nonce_authority, item_kinds, index);
    SET_IF_USED(summary->fee_payer, item_kinds, index);

    *item_kinds_len = index;
    return 0;
}
