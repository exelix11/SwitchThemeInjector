#include <string.h>
#include <stdio.h>
#include "packages.h"
#include "aes.h"
#include "rsa.h"
#include "sha.h"

void pk11_process(pk11_ctx_t *ctx) {
    fseeko64(ctx->file, 0, SEEK_SET);
    if (fread(&ctx->stage1, 1, sizeof(ctx->stage1), ctx->file) != sizeof(ctx->stage1)) {
        fprintf(stderr, "Failed to read PK11 Stage 1!\n");
        exit(EXIT_FAILURE);
    }
    
    /* Check if PK11 was built in 2016. */
    /* This is a heuristic to detect an older layout for the PK11 binary. */
    if (ctx->stage1.build_date[0] == '2' && ctx->stage1.build_date[1] == '0' && ctx->stage1.build_date[2] == '1' && ctx->stage1.build_date[3] == '6') {
        ctx->is_pilot = 1;
    } else {
        ctx->is_pilot = 0;
    }
    
    ctx->pk11 = malloc(ctx->stage1.pk11_size);
    if (ctx->pk11 == NULL) {
        fprintf(stderr, "Failed to allocate PK11!\n");
        exit(EXIT_FAILURE);
    }
    
    if (fread(ctx->pk11, 1, ctx->stage1.pk11_size, ctx->file) != ctx->stage1.pk11_size) {
        fprintf(stderr, "Failed to read PK11!\n");
        exit(EXIT_FAILURE);
    }
    
    aes_ctx_t *crypt_ctx = NULL;
    pk11_t dec_header;
    for (unsigned int i = 0; i < 0x20; i++) {
        ctx->key_rev = i;
        crypt_ctx = new_aes_ctx(&ctx->tool_ctx->settings.keyset.package1_keys[i], 0x10, AES_MODE_CTR);
        aes_setiv(crypt_ctx, ctx->stage1.ctr, 0x10);
        aes_decrypt(crypt_ctx, &dec_header, ctx->pk11, sizeof(dec_header));
        if (dec_header.magic == MAGIC_PK11) {
            break;
        }
        free_aes_ctx(crypt_ctx);
        crypt_ctx = NULL;
    }
    
    if (crypt_ctx == NULL) {
        fprintf(stderr, "Failed to decrypt PK11! Is correct key present?\n");
        exit(EXIT_FAILURE);
    }
    
    aes_setiv(crypt_ctx, ctx->stage1.ctr, 0x10);
    aes_decrypt(crypt_ctx, ctx->pk11, ctx->pk11, ctx->stage1.pk11_size);
    
    uint64_t pk11_size = 0x20 + ctx->pk11->warmboot_size + ctx->pk11->nx_bootloader_size + ctx->pk11->secmon_size;
    pk11_size = align64(pk11_size, 0x10);
    if (pk11_size != ctx->stage1.pk11_size) {
        fprintf(stderr, "PK11 seems corrupt!\n");
        exit(EXIT_FAILURE);
    }
    
    if (ctx->tool_ctx->action & ACTION_INFO) {
        pk11_print(ctx);
    }
    
    if (ctx->tool_ctx->action & ACTION_EXTRACT) {
        pk11_save(ctx);
    }
}

void pk11_print(pk11_ctx_t *ctx) {
    printf("PK11:\n");
    printf("    Build Date:                     %s\n", ctx->stage1.build_date);
    memdump(stdout, "    Build Hash:                     ", ctx->stage1.build_hash, 0x10);
    printf("    Key Revision:                   %02"PRIx32" (%s)\n", ctx->key_rev, get_key_revision_summary((uint8_t)ctx->key_rev));
    printf("    PK11 Size:                      %08"PRIx32"\n", ctx->stage1.pk11_size);
    printf("    Warmboot.bin Size:              %08"PRIx32"\n", ctx->pk11->warmboot_size);
    printf("    NX_Bootloader.bin Size          %08"PRIx32"\n", ctx->pk11->nx_bootloader_size);
    printf("    Secure_Monitor.bin Size:        %08"PRIx32"\n", ctx->pk11->secmon_size);
    printf("\n");
}

void pk11_save(pk11_ctx_t *ctx) {
    /* Extract to directory. */
    filepath_t *dirpath = NULL;
    if (ctx->tool_ctx->file_type == FILETYPE_PACKAGE1 && ctx->tool_ctx->settings.out_dir_path.enabled) {
        dirpath = &ctx->tool_ctx->settings.out_dir_path.path;
    }
    if (dirpath == NULL || dirpath->valid != VALIDITY_VALID) {
        dirpath = &ctx->tool_ctx->settings.pk11_dir_path;
    }
    if (dirpath != NULL && dirpath->valid == VALIDITY_VALID) {
        os_makedir(dirpath->os_path);
        
        /* Save Decrypted.bin */
        printf("Saving decrypted binary to %s/Decrypted.bin\n", dirpath->char_path);
        char *decrypted_bin = malloc(sizeof(ctx->stage1) + ctx->stage1.pk11_size);
        if (decrypted_bin == NULL) {
            fprintf(stderr, "Failed to allocate buffer!\n");
            exit(EXIT_FAILURE);
        }
        memcpy(decrypted_bin, &ctx->stage1, sizeof(ctx->stage1));
        memcpy(decrypted_bin + sizeof(ctx->stage1), ctx->pk11, ctx->stage1.pk11_size);
        save_buffer_to_directory_file(decrypted_bin, sizeof(ctx->stage1) + ctx->stage1.pk11_size, dirpath, "Decrypted.bin");
        free(decrypted_bin);
        
        /* Save Warmboot.bin */
        printf("Saving Warmboot.bin to %s/Warmboot.bin...\n", dirpath->char_path);
        save_buffer_to_directory_file(pk11_get_warmboot_bin(ctx), ctx->pk11->warmboot_size, dirpath, "Warmboot.bin");
        
        /* Save NX_Bootloader.bin */
        printf("Saving NX_Bootloader.bin to %s/NX_Bootloader.bin...\n", dirpath->char_path);
        save_buffer_to_directory_file(pk11_get_nx_bootloader(ctx), ctx->pk11->nx_bootloader_size, dirpath, "NX_Bootloader.bin");
        
        /* Save Secure_Monitor.bin */
        printf("Saving Secure_Monitor.bin to %s/Secure_Monitor.bin...\n", dirpath->char_path);
        save_buffer_to_directory_file(pk11_get_secmon(ctx), ctx->pk11->secmon_size, dirpath, "Secure_Monitor.bin");
    }
}

void pk21_process(pk21_ctx_t *ctx) {    
    fseeko64(ctx->file, 0, SEEK_SET);
    if (fread(&ctx->header, 1, sizeof(ctx->header), ctx->file) != sizeof(ctx->header)) {
        fprintf(stderr, "Failed to read PK21 Header!\n");
        exit(EXIT_FAILURE);
    }
    
    bool is_encrypted = false;
    for (unsigned int i = 0; i < 0x100; i++) {
        if (ctx->header.signature[i] != 0) {
            is_encrypted = true;
        }
    }
    is_encrypted &= ctx->header.magic != MAGIC_PK21;
    
    if (is_encrypted) {
        if (rsa2048_pss_verify(&ctx->header.ctr, 0x100, ctx->header.signature, ctx->tool_ctx->settings.keyset.package2_fixed_key_modulus)) {
            ctx->signature_validity = VALIDITY_VALID;
        } else {
            ctx->signature_validity = VALIDITY_INVALID;
        }
    } else {
        ctx->signature_validity = VALIDITY_UNCHECKED;
    }
    
    
    /* Nintendo, what the fuck? */
    ctx->package_size = ctx->header.ctr_dwords[0] ^ ctx->header.ctr_dwords[2] ^ ctx->header.ctr_dwords[3];
    if (ctx->package_size > 0x7FC000) {
        fprintf(stderr, "Error: Package2 Header is corrupt!\n");
        exit(EXIT_FAILURE);
    }
    
    aes_ctx_t *crypt_ctx = NULL;
    if (is_encrypted) {
        unsigned char ctr[0x10];
        pk21_header_t temp_header;
        memcpy(ctr, ctx->header.ctr, sizeof(ctr));
        
        for (unsigned int i = 0; i < 0x20; i++) {
            ctx->key_rev = i;
            memcpy(&temp_header, &ctx->header, sizeof(temp_header));
            crypt_ctx = new_aes_ctx(&ctx->tool_ctx->settings.keyset.package2_keys[i], 0x10, AES_MODE_CTR);
            aes_setiv(crypt_ctx, ctr, 0x10);
            aes_decrypt(crypt_ctx, &temp_header.ctr[0], &temp_header.ctr[0], 0x100);
            if (temp_header.magic == MAGIC_PK21) {
                memcpy(&ctx->header, &temp_header, sizeof(temp_header));
                memcpy(ctx->header.ctr, ctr, sizeof(ctr));
                break;
            }
            free_aes_ctx(crypt_ctx);
            crypt_ctx = NULL;
        }
        
        if (crypt_ctx == NULL) {
            fprintf(stderr, "Failed to decrypt PK21! Is correct key present?\n");
            exit(EXIT_FAILURE);
        }
    }
    
    if (ctx->package_size != 0x200 + ctx->header.section_sizes[0] + ctx->header.section_sizes[1] + ctx->header.section_sizes[2]) {
        fprintf(stderr, "Error: Package2 Header is corrupt!\n");
        exit(EXIT_FAILURE);
    }
    
    ctx->sections = malloc(ctx->package_size);
    if (ctx->sections == NULL) {
        fprintf(stderr, "Failed to allocate sections!\n");
        exit(EXIT_FAILURE);
    }
    
    if (fread(ctx->sections, 1, ctx->package_size - 0x200, ctx->file) != ctx->package_size - 0x200) {
        fprintf(stderr, "Failed to read PK21 Sections!\n");
        exit(EXIT_FAILURE);
    }
    
    uint64_t offset = 0;
    for (unsigned int i = 0; i < 3; i++) {
        unsigned char calc_hash[0x20];
        sha256_hash_buffer(calc_hash, ctx->sections + offset, ctx->header.section_sizes[i]);
        if (memcmp(calc_hash, ctx->header.section_hashes[i], 0x20) == 0) {
            ctx->section_validities[i] = VALIDITY_VALID;
        } else {
            ctx->section_validities[i] = VALIDITY_INVALID;
        }
        if (is_encrypted) {
            aes_setiv(crypt_ctx, ctx->header.section_ctrs[i], 0x10);
            aes_decrypt(crypt_ctx, ctx->sections + offset, ctx->sections + offset, ctx->header.section_sizes[i]);   
        }
        offset += ctx->header.section_sizes[i];
    }
    
    ctx->ini1_ctx.tool_ctx = ctx->tool_ctx;
    ctx->ini1_ctx.header = (ini1_header_t *)(ctx->sections + ctx->header.section_sizes[0]);
    if (ctx->ini1_ctx.header->magic == MAGIC_INI1 && ctx->ini1_ctx.header->num_processes <= INI1_MAX_KIPS) {
        offset = 0;
        for (unsigned int i = 0; i < ctx->ini1_ctx.header->num_processes; i++) {
            ctx->ini1_ctx.kips[i].tool_ctx = ctx->tool_ctx;
            ctx->ini1_ctx.kips[i].header = (kip1_header_t *)&ctx->ini1_ctx.header->kip_data[offset];
            if (ctx->ini1_ctx.kips[i].header->magic != MAGIC_KIP1) {
                fprintf(stderr, "INI1 is corrupted!\n");
                exit(EXIT_FAILURE);
            }
            offset += kip1_get_size(&ctx->ini1_ctx.kips[i]);
        }
    }
    
    if (ctx->tool_ctx->action & ACTION_INFO) {
        pk21_print(ctx);
    }
    
    if (ctx->tool_ctx->action & ACTION_EXTRACT) {
        pk21_save(ctx);
    }
}

static const char *pk21_get_section_name(int section) {
    switch (section) {
        case 0: return "Kernel";
        case 1: return "INI1";
        case 2: return "Empty";
        default: return "Unknown";
    }
}

void pk21_print(pk21_ctx_t *ctx) {
    printf("PK21:\n");
    if (ctx->tool_ctx->action & ACTION_VERIFY && ctx->signature_validity != VALIDITY_UNCHECKED) {
        if (ctx->signature_validity == VALIDITY_VALID) {
            memdump(stdout, "    Signature (GOOD):               ", &ctx->header.signature, 0x100);
        } else {
            memdump(stdout, "    Signature (FAIL):               ", &ctx->header.signature, 0x100);
        }
    } else {
        memdump(stdout, "    Signature:                      ", &ctx->header.signature, 0x100);
    }
    
    /* What the fuck? */
    printf("    Header Version:                 %02"PRIx32"\n", (ctx->header.ctr_dwords[1] ^ (ctx->header.ctr_dwords[1] >> 16) ^ (ctx->header.ctr_dwords[1] >> 24)) & 0xFF);
    
    for (unsigned int i = 0; i < 3; i++) {
        printf("    Section %"PRId32" (%s):\n", i, pk21_get_section_name(i));
        if (ctx->tool_ctx->action & ACTION_VERIFY) {
            if (ctx->section_validities[i] == VALIDITY_VALID) {
                memdump(stdout, "        Hash (GOOD):                ", ctx->header.section_hashes[i], 0x20);
            } else {
                memdump(stdout, "        Hash (FAIL):                ", ctx->header.section_hashes[i], 0x20);
            }
        } else {
            memdump(stdout, "        Hash:                       ", ctx->header.section_hashes[i], 0x20);
        }
        memdump(stdout, "        CTR:                        ", ctx->header.section_ctrs[i], 0x20);
        printf("        Load Address:               %08"PRIx32"\n", ctx->header.section_offsets[i] + 0x80000000);
        printf("        Size:                       %08"PRIx32"\n", ctx->header.section_sizes[i]);
    }
    
    printf("\n");
    ini1_print(&ctx->ini1_ctx);
}

void pk21_save(pk21_ctx_t *ctx) {
    /* Extract to directory. */
    filepath_t *dirpath = NULL;
    if (ctx->tool_ctx->file_type == FILETYPE_PACKAGE2 && ctx->tool_ctx->settings.out_dir_path.enabled) {
        dirpath = &ctx->tool_ctx->settings.out_dir_path.path;
    }
    if (dirpath == NULL || dirpath->valid != VALIDITY_VALID) {
        dirpath = &ctx->tool_ctx->settings.pk21_dir_path;
    }
    if (dirpath != NULL && dirpath->valid == VALIDITY_VALID) {
        os_makedir(dirpath->os_path);
        
        /* Save Decrypted.bin */
        printf("Saving decrypted binary to %s/Decrypted.bin\n", dirpath->char_path);
        char *decrypted_bin = malloc(ctx->package_size);
        if (decrypted_bin == NULL) {
            fprintf(stderr, "Failed to allocate buffer!\n");
            exit(EXIT_FAILURE);
        }
        memcpy(decrypted_bin, &ctx->header, 0x200);
        memcpy(decrypted_bin + sizeof(ctx->header), ctx->sections, ctx->package_size - 0x200);
        save_buffer_to_directory_file(decrypted_bin, ctx->package_size, dirpath, "Decrypted.bin");
        free(decrypted_bin);
        
        /* Save Kernel.bin */
        printf("Saving Kernel.bin to %s/Kernel.bin...\n", dirpath->char_path);
        save_buffer_to_directory_file(ctx->sections, ctx->header.section_sizes[0], dirpath, "Kernel.bin");
        
        /* Save INI1.bin */
        printf("Saving INI1.bin to %s/INI1.bin...\n", dirpath->char_path);
        save_buffer_to_directory_file(ctx->sections +  ctx->header.section_sizes[0], ctx->header.section_sizes[1], dirpath, "INI1.bin");
    }
    if (ctx->ini1_ctx.header != NULL && (ctx->tool_ctx->action & ACTION_EXTRACTINI1 || ctx->tool_ctx->settings.ini1_dir_path.valid == VALIDITY_VALID)) {
        filepath_t *ini1_dirpath = &ctx->tool_ctx->settings.ini1_dir_path;
        if (ini1_dirpath->valid != VALIDITY_VALID && dirpath != NULL && dirpath->valid == VALIDITY_VALID) {
            filepath_copy(ini1_dirpath, dirpath);
            filepath_append(ini1_dirpath, "INI1");
        }
        ini1_save(&ctx->ini1_ctx);
    }
}