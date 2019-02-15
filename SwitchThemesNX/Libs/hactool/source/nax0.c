#include <string.h>
#include "aes.h"
#include "sha.h"
#include "nax0.h"

static size_t nax0_read(nax0_ctx_t *ctx, uint64_t offset, void *dst, size_t size) {
    if (ctx->num_files == 1) {
        fseeko64(ctx->files[0], offset, SEEK_SET);
        return fread(dst, 1, size, ctx->files[0]);
    }
    
    FILE *which = ctx->files[offset / 0xFFFF0000ULL];
    uint64_t offset_in_file = offset % 0xFFFF0000ULL;
    fseeko64(which, offset_in_file, SEEK_SET);
    uint64_t left_in_file = 0xFFFF0000ULL - offset_in_file;
    if (size > left_in_file) {
        return fread(dst, 1, left_in_file, which) + nax0_read(ctx, offset + left_in_file, (unsigned char *)dst + left_in_file, size - left_in_file);
    } else {
        return fread(dst, 1, size, which);
    }
}

void nax0_process(nax0_ctx_t *ctx) {
    /* First things first... */
    FILE *f_temp;
    if ((f_temp = os_fopen(ctx->base_path.os_path, OS_MODE_READ)) != NULL) {
        ctx->num_files = 1;
        ctx->files = calloc(1, sizeof(FILE *));
        if (ctx->files == NULL) {
            fprintf(stderr, "Failed to allocate NAX0 file holder!\n");
            exit(EXIT_FAILURE);
        }
        ctx->files[0] = f_temp;
    } else {
        ctx->num_files = 0;
        filepath_t temp_path;
        while (1) {
            filepath_copy(&temp_path, &ctx->base_path);
            filepath_append(&temp_path, "%02"PRIu32, ctx->num_files);
            if ((f_temp = os_fopen(temp_path.os_path, OS_MODE_READ)) == NULL) {
                break;
            }
            ctx->num_files++;
            fclose(f_temp);
        }
        if (ctx->num_files == 0) {
            fprintf(stderr, "Input path appears to neither be a NAX0, nor a NAX0 directory!\n");
            exit(EXIT_FAILURE);
        }
        ctx->files = calloc(ctx->num_files, sizeof(FILE *));
        if (ctx->files == NULL) {
            fprintf(stderr, "Failed to allocate NAX0 file holder!\n");
            exit(EXIT_FAILURE);
        }
        for (unsigned int i = 0; i < ctx->num_files; i++) {
            filepath_copy(&temp_path, &ctx->base_path);
            filepath_append(&temp_path, "%02"PRIu32, i);
            if ((ctx->files[i] = os_fopen(temp_path.os_path, OS_MODE_READ)) == NULL) {
                fprintf(stderr, "Failed to open %s!\n", temp_path.char_path);
                exit(EXIT_FAILURE);
            }
        }
    }
    
    nax0_read(ctx, 0, &ctx->header, sizeof(ctx->header));
    if (ctx->header.magic != MAGIC_NAX0) {
        printf("Error: File has invalid NAX0 magic!\n");
        return;
    }
    
    memcpy(ctx->encrypted_keys, ctx->header.keys, sizeof(ctx->header.keys));
    
    int found = 0;
    for (ctx->k = 0; ctx->k < 2; ctx->k++) {
        unsigned char nax_specific_keys[2][0x10];
        sha256_get_buffer_hmac(nax_specific_keys, ctx->tool_ctx->settings.keyset.sd_card_keys[ctx->k], 0x10, ctx->tool_ctx->settings.nax0_sd_path.char_path, strlen(ctx->tool_ctx->settings.nax0_sd_path.char_path));
        for (unsigned int i = 0; i < 2; i++) {
            aes_ctx_t *nax_k_ctx = new_aes_ctx(nax_specific_keys[i], 0x10, AES_MODE_ECB);
            aes_decrypt(nax_k_ctx, ctx->header.keys[i], ctx->encrypted_keys[i], 0x10);
            free_aes_ctx(nax_k_ctx);
        }
        
        unsigned char validation_mac[0x20];
        sha256_get_buffer_hmac(validation_mac, &ctx->header.magic, 0x60, ctx->tool_ctx->settings.keyset.sd_card_keys[ctx->k] + 0x10, 0x10);
        if (memcmp(ctx->header.hmac_header, validation_mac, 0x20) == 0) {
            found = 1;
            break;
        }
    }
    
    if (!found) {
        printf("Error: NAX0 key derivation failed. Check SD card seed and relative path?\n");
        return;
    }
    
    ctx->aes_ctx = new_aes_ctx(ctx->header.keys, 0x20, AES_MODE_XTS);
    
    if (ctx->tool_ctx->action & ACTION_INFO) {
        nax0_print(ctx);
    }
    
    if (ctx->tool_ctx->action & ACTION_EXTRACT) {
        nax0_save(ctx);
    }
}

void nax0_save(nax0_ctx_t *ctx) {
 /* Save Decrypted Contents. */
    filepath_t *dec_path = &ctx->tool_ctx->settings.plaintext_path;

    if (dec_path->valid != VALIDITY_VALID) {
        return;
    }

    printf("Saving Decrypted NAX0 Content to %s...\n", dec_path->char_path);
    FILE *f_dec = os_fopen(dec_path->os_path, OS_MODE_WRITE);

    if (f_dec == NULL) {
        fprintf(stderr, "Failed to open %s!\n", dec_path->char_path);
        return;
    }

    uint64_t ofs = 0x4000;
    uint64_t end_ofs = ofs + ctx->header.size;
    unsigned char *buf = malloc(0x400000);
    if (buf == NULL) {
        fprintf(stderr, "Failed to allocate file-save buffer!\n");
        exit(EXIT_FAILURE);
    }

    uint64_t read_size = 0x400000; /* 4 MB buffer. */
    memset(buf, 0xCC, read_size); /* Debug in case I fuck this up somehow... */
    while (ofs < end_ofs) {
        if (ofs + read_size >= end_ofs) read_size = end_ofs - ofs;
        if (nax0_read(ctx, ofs, buf, read_size) != read_size) {
            fprintf(stderr, "Failed to read file!\n");
            exit(EXIT_FAILURE);
        }

        uint64_t dec_size = (read_size + 0x3FFF) & ~0x3FFF;
        aes_xts_decrypt(ctx->aes_ctx, buf, buf, dec_size, (ofs - 0x4000) >> 14, 0x4000);

        if (fwrite(buf, 1, read_size, f_dec) != read_size) {
            fprintf(stderr, "Failed to write file!\n");
            exit(EXIT_FAILURE);
        }
        ofs += read_size;
    }

    fclose(f_dec);
    free(buf);
}

static const char *nax0_get_key_summary(unsigned int k) {
    switch (k) {
        case 0:
            return "Save";
        case 1:
            return "NCA";
        default:
            return "Unknown";
    }
}

void nax0_print(nax0_ctx_t *ctx) {
    printf("\nNAX0:\n");
    print_magic("    Magic:                          ", ctx->header.magic);
    printf("    Content Type:                   %s\n", nax0_get_key_summary(ctx->k));
    printf("    Content Size:                   %012"PRIx64"\n", ctx->header.size);
    memdump(stdout, "    Header HMAC:                    ", ctx->header.hmac_header, 0x20);
    memdump(stdout, "    Encrypted Keys:                 ", ctx->encrypted_keys, 0x20);
    memdump(stdout, "    Decrypted Keys:                 ", ctx->header.keys, 0x20);
}