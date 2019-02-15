#include <string.h>
#include "rsa.h"
#include "xci.h"

/* This RSA-PKCS1 public key is only accessible to the gamecard controller. */
/* However, it (and other XCI keys) can be dumped with a GCD attack on two signatures. */
/* Contact SciresM for details, if curious. */
static const unsigned char xci_header_pubk[0x100] = {
    0x98, 0xC7, 0x26, 0xB6, 0x0D, 0x0A, 0x50, 0xA7, 0x39, 0x21, 0x0A, 0xE3, 0x2F, 0xE4, 0x3E, 0x2E, 
    0x5B, 0xA2, 0x86, 0x75, 0xAA, 0x5C, 0xEE, 0x34, 0xF1, 0xA3, 0x3A, 0x7E, 0xBD, 0x90, 0x4E, 0xF7, 
    0x8D, 0xFA, 0x17, 0xAA, 0x6B, 0xC6, 0x36, 0x6D, 0x4C, 0x9A, 0x6D, 0x57, 0x2F, 0x80, 0xA2, 0xBC, 
    0x38, 0x4D, 0xDA, 0x99, 0xA1, 0xD8, 0xC3, 0xE2, 0x99, 0x79, 0x36, 0x71, 0x90, 0x20, 0x25, 0x9D, 
    0x4D, 0x11, 0xB8, 0x2E, 0x63, 0x6B, 0x5A, 0xFA, 0x1E, 0x9C, 0x04, 0xD1, 0xC5, 0xF0, 0x9C, 0xB1, 
    0x0F, 0xB8, 0xC1, 0x7B, 0xBF, 0xE8, 0xB0, 0xD2, 0x2B, 0x47, 0x01, 0x22, 0x6B, 0x23, 0xC9, 0xD0, 
    0xBC, 0xEB, 0x75, 0x6E, 0x41, 0x7D, 0x4C, 0x26, 0xA4, 0x73, 0x21, 0xB4, 0xF0, 0x14, 0xE5, 0xD9, 
    0x8D, 0xB3, 0x64, 0xEE, 0xA8, 0xFA, 0x84, 0x1B, 0xB8, 0xB8, 0x7C, 0x88, 0x6B, 0xEF, 0xCC, 0x97, 
    0x04, 0x04, 0x9A, 0x67, 0x2F, 0xDF, 0xEC, 0x0D, 0xB2, 0x5F, 0xB5, 0xB2, 0xBD, 0xB5, 0x4B, 0xDE, 
    0x0E, 0x88, 0xA3, 0xBA, 0xD1, 0xB4, 0xE0, 0x91, 0x81, 0xA7, 0x84, 0xEB, 0x77, 0x85, 0x8B, 0xEF, 
    0xA5, 0xE3, 0x27, 0xB2, 0xF2, 0x82, 0x2B, 0x29, 0xF1, 0x75, 0x2D, 0xCE, 0xCC, 0xAE, 0x9B, 0x8D, 
    0xED, 0x5C, 0xF1, 0x8E, 0xDB, 0x9A, 0xD7, 0xAF, 0x42, 0x14, 0x52, 0xCD, 0xE3, 0xC5, 0xDD, 0xCE, 
    0x08, 0x12, 0x17, 0xD0, 0x7F, 0x1A, 0xAA, 0x1F, 0x7D, 0xE0, 0x93, 0x54, 0xC8, 0xBC, 0x73, 0x8A, 
    0xCB, 0xAD, 0x6E, 0x93, 0xE2, 0x19, 0x72, 0x6B, 0xD3, 0x45, 0xF8, 0x73, 0x3D, 0x2B, 0x6A, 0x55, 
    0xD2, 0x3A, 0x8B, 0xB0, 0x8A, 0x42, 0xE3, 0x3D, 0xF1, 0x92, 0x23, 0x42, 0x2E, 0xBA, 0xCC, 0x9C, 
    0x9A, 0xC1, 0xDD, 0x62, 0x86, 0x9C, 0x2E, 0xE1, 0x2D, 0x6F, 0x62, 0x67, 0x51, 0x08, 0x0E, 0xCF
};

void xci_process(xci_ctx_t *ctx) {
    fseeko64(ctx->file, 0, SEEK_SET);
    if (fread(&ctx->header, 1, 0x200, ctx->file) != 0x200) {
        fprintf(stderr, "Failed to read XCI header!\n");
        return;
    }
    
    if (ctx->header.magic != MAGIC_HEAD) {
        fprintf(stderr, "Error: XCI header is corrupt!\n");
        exit(EXIT_FAILURE);
    }
    
    if (ctx->tool_ctx->action & ACTION_VERIFY) {
        if (rsa2048_pkcs1_verify(&ctx->header.magic, 0x100, ctx->header.header_sig, xci_header_pubk)) {
            ctx->header_sig_validity = VALIDITY_VALID;
        } else {
            ctx->header_sig_validity = VALIDITY_INVALID;
        }
    }
    
    ctx->hfs0_hash_validity = check_memory_hash_table(ctx->file, ctx->header.hfs0_header_hash, ctx->header.hfs0_offset, ctx->header.hfs0_header_size, ctx->header.hfs0_header_size, 0);
    if (ctx->hfs0_hash_validity != VALIDITY_VALID) {
        fprintf(stderr, "Error: XCI partition is corrupt!\n");
        exit(EXIT_FAILURE);
    }
    
    hactool_ctx_t blank_ctx;
    memset(&blank_ctx, 0, sizeof(blank_ctx));
    blank_ctx.action = ctx->tool_ctx->action & ~(ACTION_EXTRACT | ACTION_INFO);
    
    ctx->partition_ctx.file = ctx->file;
    ctx->partition_ctx.offset = ctx->header.hfs0_offset;
    ctx->partition_ctx.tool_ctx = &blank_ctx;
    ctx->partition_ctx.name = "rootpt";
    hfs0_process(&ctx->partition_ctx);
    
    if (ctx->partition_ctx.header->num_files > 4) {
        fprintf(stderr, "Error: Invalid XCI partition!\n");
        exit(EXIT_FAILURE);    
    }
    
    for (unsigned int i = 0; i < ctx->partition_ctx.header->num_files; i++)  {
        hfs0_ctx_t *cur_ctx = NULL;
        
        hfs0_file_entry_t *cur_file = hfs0_get_file_entry(ctx->partition_ctx.header, i);
        char *cur_name = hfs0_get_file_name(ctx->partition_ctx.header, i);
        if (!strcmp(cur_name, "update") && ctx->update_ctx.file == NULL) {
            cur_ctx = &ctx->update_ctx;
        } else if (!strcmp(cur_name, "normal") && ctx->normal_ctx.file == NULL) {
            cur_ctx = &ctx->normal_ctx;
        } else if (!strcmp(cur_name, "secure") && ctx->secure_ctx.file == NULL) {
            cur_ctx = &ctx->secure_ctx;
        } else if (!strcmp(cur_name, "logo") && ctx->logo_ctx.file == NULL) {
            cur_ctx = &ctx->logo_ctx;
        } 
        
        if (cur_ctx == NULL) {
            fprintf(stderr, "Unknown XCI partition: %s\n", cur_name);
            exit(EXIT_FAILURE);
        }
        
        cur_ctx->name = cur_name;
        cur_ctx->offset = ctx->partition_ctx.offset + hfs0_get_header_size(ctx->partition_ctx.header) + cur_file->offset;
        cur_ctx->tool_ctx = &blank_ctx;
        cur_ctx->file = ctx->file;
        hfs0_process(cur_ctx);
    }
    
    for (unsigned int i = 0; i < 0x10; i++) {
        ctx->iv[i] = ctx->header.reversed_iv[0xF-i];
    }

    if (ctx->tool_ctx->action & ACTION_INFO) {
        xci_print(ctx);
    }
    
    if (ctx->tool_ctx->action & ACTION_EXTRACT) {
        xci_save(ctx);
    }
}

void xci_save(xci_ctx_t *ctx) {
    /* Extract to directory. */
    if (ctx->tool_ctx->settings.out_dir_path.enabled && ctx->tool_ctx->settings.out_dir_path.path.valid == VALIDITY_VALID) {
        printf("Extracting XCI...\n");
        filepath_t *dirpath = &ctx->tool_ctx->settings.out_dir_path.path;
        os_makedir(dirpath->os_path);
        for (unsigned int i = 0; i < ctx->partition_ctx.header->num_files; i++) {
            hfs0_ctx_t *cur_ctx = NULL;
            
            char *cur_name = hfs0_get_file_name(ctx->partition_ctx.header, i);
            if (!strcmp(cur_name, "update")) {
                cur_ctx = &ctx->update_ctx;
            } else if (!strcmp(cur_name, "normal")) {
                cur_ctx = &ctx->normal_ctx;
            } else if (!strcmp(cur_name, "secure")) {
                cur_ctx = &ctx->secure_ctx;
            } else if (!strcmp(cur_name, "logo")) {
                cur_ctx = &ctx->logo_ctx;
            }
            if (cur_ctx == NULL) {
                fprintf(stderr, "Unknown XCI partition found in extraction: %s\n", cur_name);
                exit(EXIT_FAILURE);
            }
            filepath_t partition_dirpath;
            filepath_copy(&partition_dirpath, dirpath);

            filepath_append(&partition_dirpath, "%s", cur_name);
            os_makedir(partition_dirpath.os_path);
            for (uint32_t i = 0; i < cur_ctx->header->num_files; i++) {
                hfs0_save_file(cur_ctx, i, &partition_dirpath);
            }
        }
    } else {
        /* Save Root Partition. */
        if (ctx->tool_ctx->settings.rootpt_dir_path.valid == VALIDITY_VALID) {
            printf("Saving Root Partition...\n");
            os_makedir(ctx->tool_ctx->settings.rootpt_dir_path.os_path);
            for (uint32_t i = 0; i < ctx->partition_ctx.header->num_files; i++) {
                hfs0_save_file(&ctx->partition_ctx, i, &ctx->tool_ctx->settings.rootpt_dir_path);
            }
            printf("\n");
        }
        /* Save Update Partition. */
        if (ctx->tool_ctx->settings.update_dir_path.valid == VALIDITY_VALID) {
             printf("Saving Update Partition...\n");
            os_makedir(ctx->tool_ctx->settings.update_dir_path.os_path);
            for (uint32_t i = 0; i < ctx->update_ctx.header->num_files; i++) {
                hfs0_save_file(&ctx->update_ctx, i, &ctx->tool_ctx->settings.update_dir_path);
            }
            printf("\n");
        }
        /* Save Normal Partition. */
        if (ctx->tool_ctx->settings.normal_dir_path.valid == VALIDITY_VALID) {
             printf("Saving Normal Partition...\n");
            os_makedir(ctx->tool_ctx->settings.normal_dir_path.os_path);
            for (uint32_t i = 0; i < ctx->normal_ctx.header->num_files; i++) {
                hfs0_save_file(&ctx->normal_ctx, i, &ctx->tool_ctx->settings.normal_dir_path);
            }
            printf("\n");
        }
        /* Save Secure Partition. */
        if (ctx->tool_ctx->settings.secure_dir_path.valid == VALIDITY_VALID) {
            printf("Saving Secure Partition...\n");
            os_makedir(ctx->tool_ctx->settings.secure_dir_path.os_path);
            for (uint32_t i = 0; i < ctx->secure_ctx.header->num_files; i++) {
                hfs0_save_file(&ctx->secure_ctx, i, &ctx->tool_ctx->settings.secure_dir_path);
            }
            printf("\n");
        }
        /* Save Logo Partition. */
        if (ctx->tool_ctx->settings.logo_dir_path.valid == VALIDITY_VALID) {
            printf("Saving Logo Partition...\n");
            os_makedir(ctx->tool_ctx->settings.logo_dir_path.os_path);
            for (uint32_t i = 0; i < ctx->logo_ctx.header->num_files; i++) {
                hfs0_save_file(&ctx->logo_ctx, i, &ctx->tool_ctx->settings.logo_dir_path);
            }
            printf("\n");
        }  
    }
}

static const char *xci_get_cartridge_type(xci_ctx_t *ctx) {
    cartridge_type_t cart_type = (cartridge_type_t)ctx->header.cart_type;
    switch (cart_type) {
        case CARTSIZE_2GB: return "2GB";
        case CARTSIZE_4GB: return "4GB";
        case CARTSIZE_8GB: return "8GB";
        case CARTSIZE_16GB: return "16GB";
        default:
            return "Unknown/Invalid";
    }
}

static void xci_print_hfs0(hfs0_ctx_t *ctx) {
    print_magic("    Magic:                          ", ctx->header->magic);
    printf("    Offset:                         %012"PRIx64"\n", ctx->offset);
    printf("    Number of files:                %"PRId32"\n", ctx->header->num_files);
    
    if (ctx->header->num_files > 0 && (ctx->header->num_files < 100 || ctx->tool_ctx->action & ACTION_VERIFY)) {
        printf("    Files:");
        for (unsigned int i = 0; i < ctx->header->num_files; i++) {
            hfs0_file_entry_t *cur_file = hfs0_get_file_entry(ctx->header, i);
            if (ctx->tool_ctx->action & ACTION_VERIFY) {
                validity_t hash_validity = check_memory_hash_table(ctx->file, cur_file->hash, ctx->offset + hfs0_get_header_size(ctx->header) + cur_file->offset, cur_file->hashed_size, cur_file->hashed_size, 0);
                printf("%s%s:/%-48s %012"PRIx64"-%012"PRIx64" (%s)\n", i == 0 ? "                          " : "                                    ", ctx->name == NULL ? "hfs0" : ctx->name, hfs0_get_file_name(ctx->header, i), cur_file->offset, cur_file->offset + cur_file->size, GET_VALIDITY_STR(hash_validity));
            } else {
                printf("%s%s:/%-48s %012"PRIx64"-%012"PRIx64"\n", i == 0 ? "                          " : "                                    ", ctx->name == NULL ? "hfs0" : ctx->name, hfs0_get_file_name(ctx->header, i), cur_file->offset, cur_file->offset + cur_file->size);
            }
        }
    }
}
    
void xci_print(xci_ctx_t *ctx) {
    printf("\nXCI:\n");
    print_magic("Magic:                              ", ctx->header.magic);

    if (ctx->tool_ctx->action & ACTION_VERIFY) {
        if (ctx->header_sig_validity == VALIDITY_VALID) {
            memdump(stdout, "Header Signature (GOOD):            ", &ctx->header.header_sig, 0x100);
        } else {
            memdump(stdout, "Header Signature (FAIL):            ", &ctx->header.header_sig, 0x100);
        }
    } else {
        memdump(stdout, "Header Signature:                   ", &ctx->header.header_sig, 0x100);
    }
    
    printf("Cartridge Type:                     %s\n", xci_get_cartridge_type(ctx));
    printf("Cartridge Size:                     %012"PRIx64"\n", media_to_real(ctx->header.cart_size + 1));
    memdump(stdout, "Header IV:                          ", ctx->iv, 0x10);
    memdump(stdout, "Encrypted Header:                   ", ctx->header.encrypted_data, 0x70);

    if (ctx->tool_ctx->action & ACTION_VERIFY) {
        printf("Root Partition (%s):\n", GET_VALIDITY_STR(ctx->hfs0_hash_validity));
    } else {
        printf("Root Partition:\n");
    }
    xci_print_hfs0(&ctx->partition_ctx);
    
    printf("Update Partition:\n");
    xci_print_hfs0(&ctx->update_ctx);
    
    printf("Normal Partition:\n");
    xci_print_hfs0(&ctx->normal_ctx);
    
    printf("Secure Partition:\n");
    xci_print_hfs0(&ctx->secure_ctx);
    
    /* Ensure that Logo partition exists. */
    if (ctx->partition_ctx.header->num_files == 4) {
        printf("Logo Partition:\n");
        xci_print_hfs0(&ctx->logo_ctx);
    }
}