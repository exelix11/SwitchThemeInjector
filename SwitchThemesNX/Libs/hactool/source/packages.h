#ifndef HACTOOL_PACKAGES_H
#define HACTOOL_PACKAGES_H

#include "types.h"
#include "utils.h"
#include "settings.h"
#include "kip.h"

#define MAGIC_PK11 0x31314B50
#define MAGIC_PK21 0x31324B50
#define MAGIC_KRNLLDR_STRCT_END 0xD51C403E

typedef struct {
    unsigned char aes_mac[0x10];
    unsigned char rsa_sig[0x100];
    unsigned char salt[0x20];
    unsigned char hash[0x20];
    uint32_t bl_version;
    uint32_t bl_size;
    uint32_t bl_load_addr;
    uint32_t bl_entrypoint;
    unsigned char _0x160[0x10];
} pk11_mariko_oem_header_t;

typedef struct {
    uint32_t ldr_hash;
    uint32_t sm_hash;
    uint32_t bl_hash;
    uint32_t _0xC;
    char build_date[0xE];
    unsigned char _0x1E;
    unsigned char version;
} pk11_metadata_t;

typedef struct {
    unsigned char stage1[0x3FC0];
    uint32_t pk11_size;
    unsigned char _0x3FC4[0xC];
    unsigned char ctr[0x10];
} pk11_legacy_stage1_t;

typedef struct {
    unsigned char stage1[0x6FC0];
    uint32_t pk11_size;
    unsigned char _0x6FC4[0xC];
    unsigned char iv[0x10];
} pk11_modern_stage1_t;

typedef union {
    pk11_legacy_stage1_t legacy;
    pk11_modern_stage1_t modern;
} pk11_stage1_t;

typedef struct {
    uint32_t magic;
    uint32_t wb_size;
    uint32_t wb_ep;
    uint32_t _0xC;
    uint32_t bl_size;
    uint32_t bl_ep;
    uint32_t sm_size;
    uint32_t sm_ep;
    unsigned char data[];
} pk11_t;

typedef struct {
    FILE *file;
    hactool_ctx_t *tool_ctx;
    int is_modern;
    int is_mariko;
    int is_decrypted;
    unsigned int key_rev;
    pk11_mariko_oem_header_t mariko_oem_header;
    pk11_metadata_t metadata;
    pk11_stage1_t stage1;
    uint32_t pk11_size;
    uint8_t *mariko_bl;
    pk11_t *pk11;
    unsigned char pk11_mac[0x10];
} pk11_ctx_t;

typedef enum {
    PK11_SECTION_BL,
    PK11_SECTION_SM,
    PK11_SECTION_WB,
} pk11_section_id_t;

static inline int pk11_get_section_idx(pk11_ctx_t *ctx, pk11_section_id_t section_id) {
    if (ctx->metadata.version >= 0x07) {
        switch (section_id) {
            case PK11_SECTION_BL: return 0;
            case PK11_SECTION_SM: return 1;
            case PK11_SECTION_WB: return 2;
        }
    } else if (ctx->metadata.version >= 0x02) {
        switch (section_id) {
            case PK11_SECTION_BL: return 1;
            case PK11_SECTION_SM: return 2;
            case PK11_SECTION_WB: return 0;
        }
    } else {
        switch (section_id) {
            case PK11_SECTION_BL: return 1;
            case PK11_SECTION_SM: return 0;
            case PK11_SECTION_WB: return 2;
        }
    }

    return 0;
}

static inline pk11_section_id_t pk11_get_section_id(pk11_ctx_t *ctx, int id) {
    if (pk11_get_section_idx(ctx, PK11_SECTION_BL) == id) {
        return PK11_SECTION_BL;
    } else if (pk11_get_section_idx(ctx, PK11_SECTION_SM) == id) {
        return PK11_SECTION_SM;
    } else {
        return PK11_SECTION_WB;
    }
}

static inline uint32_t pk11_get_section_size(pk11_ctx_t *ctx, pk11_section_id_t section_id) {
    switch (section_id) {
        case PK11_SECTION_BL: return ctx->pk11->bl_size;
        case PK11_SECTION_SM: return ctx->pk11->sm_size;
        case PK11_SECTION_WB: return ctx->pk11->wb_size;
    }

    return 0;
}

static inline uint32_t pk11_get_section_ofs(pk11_ctx_t *ctx, pk11_section_id_t section_id) {
    switch (pk11_get_section_idx(ctx, section_id)) {
        case 0:
        default:
            return 0;
        case 1:
            return pk11_get_section_size(ctx, pk11_get_section_id(ctx, 0));
        case 2:
            return pk11_get_section_size(ctx, pk11_get_section_id(ctx, 0)) + pk11_get_section_size(ctx, pk11_get_section_id(ctx, 1));
    }
}

static inline unsigned char *pk11_get_section(pk11_ctx_t *ctx, pk11_section_id_t section_id) {
    return &ctx->pk11->data[pk11_get_section_ofs(ctx, section_id)];
}

static inline unsigned char *pk11_get_warmboot_bin(pk11_ctx_t *ctx) {
    return pk11_get_section(ctx, PK11_SECTION_WB);
}

static inline unsigned char *pk11_get_secmon(pk11_ctx_t *ctx) {
    return pk11_get_section(ctx, PK11_SECTION_SM);
}

static inline unsigned char *pk11_get_nx_bootloader(pk11_ctx_t *ctx) {
    return pk11_get_section(ctx, PK11_SECTION_BL);
}

static inline unsigned int pk11_get_warmboot_bin_size(pk11_ctx_t *ctx) {
    return pk11_get_section_size(ctx, PK11_SECTION_WB);
}

static inline unsigned int pk11_get_secmon_size(pk11_ctx_t *ctx) {
    return pk11_get_section_size(ctx, PK11_SECTION_SM);
}

static inline unsigned int pk11_get_nx_bootloader_size(pk11_ctx_t *ctx) {
    return pk11_get_section_size(ctx, PK11_SECTION_BL);
}


void pk11_process(pk11_ctx_t *ctx);
void pk11_print(pk11_ctx_t *ctx);
void pk11_save(pk11_ctx_t *ctx);


/* Package2 */
#pragma pack(push, 1)
typedef struct {
    unsigned char signature[0x100];
    union {
        unsigned char ctr[0x10];
        uint32_t ctr_dwords[0x4];
    };
    unsigned char section_ctrs[4][0x10];
    uint32_t magic;
    uint32_t base_offset;
    uint32_t _0x58;
    uint8_t version_max; /* Must be > TZ value. */
    uint8_t version_min; /* Must be < TZ value. */
    uint16_t _0x5E;
    uint32_t section_sizes[4];
    uint32_t section_offsets[4];
    unsigned char section_hashes[4][0x20];
} pk21_header_t;
#pragma pack(pop)

typedef struct {
    uint32_t text_start_offset;
    uint32_t text_end_offset;
    uint32_t rodata_start_offset;
    uint32_t rodata_end_offset;
    uint32_t data_start_offset;
    uint32_t data_end_offset;
    uint32_t bss_start_offset;
    uint32_t bss_end_offset;
    uint32_t ini1_start_offset;
    uint32_t dynamic_offset;
    uint32_t init_array_start_offset;
    uint32_t init_array_end_offset;
} kernel_map_t;

typedef struct {
    FILE *file;
    hactool_ctx_t *tool_ctx;
    unsigned int key_rev;
    uint32_t package_size;
    validity_t signature_validity;
    validity_t section_validities[4];
    unsigned char *sections;
    pk21_header_t header;
    ini1_ctx_t ini1_ctx;
    kernel_map_t *kernel_map;
} pk21_ctx_t;

void pk21_process(pk21_ctx_t *ctx);
void pk21_print(pk21_ctx_t *ctx);
void pk21_save(pk21_ctx_t *ctx);

#endif
