#ifndef HACTOOL_XCI_H
#define HACTOOL_XCI_H
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "types.h"
#include "settings.h"
#include "hfs0.h"

#define MAGIC_HEAD 0x44414548 /* "HEAD" */

typedef enum {
    CARTSIZE_1GB = 0xFA,
    CARTSIZE_2GB = 0xF8,
    CARTSIZE_4GB = 0xF0,
    CARTSIZE_8GB = 0xE0,
    CARTSIZE_16GB = 0xE1,
    CARTSIZE_32GB = 0xE2
} cartridge_type_t;

typedef enum {
    GC_FIRMWARE_DEVELOPMENT = 0x00,
    GC_FIRMWARE_RETAIL_100 = 0x01,
    GC_FIRMWARE_RETAIL_400 = 0x02
} gamecard_firmware_version_t;

typedef enum {
    GC_ACCESS_CONTROL_25MHZ = 0x00A10011,
    GC_ACCESS_CONTROL_50MHZ = 0x00A10010,
} gamecard_access_control_t;

typedef enum {
    COMPAT_GLOBAL = 0x00,
    COMPAT_CHINA = 0x01
} xci_region_compatibility_t;

typedef struct {
    uint64_t firmware_version;
    uint32_t access_control;
    uint32_t read_time_wait_1;
    uint32_t read_time_wait_2;
    uint32_t write_time_wait_1;
    uint32_t write_time_wait_2;
    uint32_t firmware_mode;
    uint32_t cup_version;
    uint8_t compatibility_type;
    uint8_t _0x25;
    uint8_t _0x26;
    uint8_t _0x27;
    unsigned char update_partition_hash[0x8];
    uint64_t cup_title_id;
} xci_gamecard_info_t;

typedef struct {
    uint8_t header_sig[0x100];
    uint32_t magic;
    uint32_t secure_offset;
    uint32_t _0x108;
    uint8_t _0x10C;
    uint8_t cart_type;
    uint8_t _0x10E;
    uint8_t _0x10F;
    uint64_t _0x110;
    uint64_t cart_size;
    unsigned char reversed_iv[0x10];
    uint64_t hfs0_offset;
    uint64_t hfs0_header_size;
    unsigned char hfs0_header_hash[0x20];
    unsigned char crypto_header_hash[0x20];
    uint32_t _0x180;
    uint32_t _0x184;
    uint32_t _0x188;
    uint32_t _0x18C;
    unsigned char encrypted_data[0x70];
} xci_header_t;

typedef struct {
    FILE *file; /* File for this NCA. */
    validity_t header_sig_validity;
    validity_t cert_sig_validity;
    validity_t hfs0_hash_validity;
    hfs0_ctx_t partition_ctx;
    hfs0_ctx_t normal_ctx;
    hfs0_ctx_t update_ctx;
    hfs0_ctx_t secure_ctx;
    hfs0_ctx_t logo_ctx;
    hactool_ctx_t *tool_ctx;
    unsigned char iv[0x10];
    int has_decrypted_header;
    unsigned char decrypted_header[0x70];
    xci_header_t header;
    int has_fake_compat_type;
    uint8_t fake_compat_type;
} xci_ctx_t;

void xci_process(xci_ctx_t *ctx);
void xci_save(xci_ctx_t *ctx);
void xci_print(xci_ctx_t *ctx);

#endif
