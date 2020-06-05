#include <string.h>
#include <stdio.h>
#include "packages.h"
#include "aes.h"
#include "rsa.h"
#include "sha.h"

static int pk11_is_mariko(pk11_ctx_t *ctx) {
    fseeko64(ctx->file, 0, SEEK_SET);
    if (fread(&ctx->mariko_oem_header, 1, sizeof(ctx->mariko_oem_header), ctx->file) != sizeof(ctx->mariko_oem_header)) {
        fprintf(stderr, "Failed to read PK11 OEM Header!\n");
        exit(EXIT_FAILURE);
    }

    for (int i = 0; i < 0x10; i++) {
        if (ctx->mariko_oem_header.aes_mac[i] != 0 || ctx->mariko_oem_header._0x160[i] != 0) {
            return 0;
        }
    }

    return 1;
}

static int pk11_is_legacy(pk11_ctx_t *ctx) {
    return ctx->metadata.version < 0x0E || memcmp(ctx->metadata.build_date, "20181107", 8) < 0;
}

void pk11_process(pk11_ctx_t *ctx) {
    fseeko64(ctx->file, 0, SEEK_SET);
    if (fread(&ctx->stage1, 1, sizeof(ctx->stage1), ctx->file) != sizeof(ctx->stage1)) {
        fprintf(stderr, "Failed to read PK11 Stage 1!\n");
        exit(EXIT_FAILURE);
    }

    // Detect mariko
    ctx->is_mariko = pk11_is_mariko(ctx);

    if (ctx->is_mariko) {
        fseeko64(ctx->file, sizeof(ctx->mariko_oem_header), SEEK_SET);

        if (ctx->mariko_oem_header.bl_size < sizeof(ctx->metadata)) {
            fprintf(stderr, "PK11 seems corrupt!\n");
            exit(EXIT_FAILURE);
        }

        ctx->mariko_bl = calloc(1, ctx->mariko_oem_header.bl_size);
        if (fread(ctx->mariko_bl, 1, ctx->mariko_oem_header.bl_size, ctx->file) != ctx->mariko_oem_header.bl_size) {
            fprintf(stderr, "Failed to read Mariko PK11!\n");
            exit(EXIT_FAILURE);
        }

        memcpy(&ctx->metadata, ctx->mariko_bl, sizeof(ctx->metadata));

        ctx->is_decrypted = memcmp(&ctx->metadata, ctx->mariko_bl + 0x20, sizeof(ctx->metadata)) == 0;

        if (!ctx->is_decrypted) {
            uint32_t enc_size = ctx->mariko_oem_header.bl_size - sizeof(ctx->metadata);
            if (enc_size > 0) {
                aes_ctx_t *crypt_ctx = new_aes_ctx(ctx->tool_ctx->settings.keyset.mariko_bek, 0x10, AES_MODE_CBC);

                aes_setiv(crypt_ctx, ctx->mariko_bl + 0x10, 0x10);
                aes_decrypt(crypt_ctx, ctx->mariko_bl + 0x20, ctx->mariko_bl + 0x20, enc_size);

                free_aes_ctx(crypt_ctx);

                ctx->is_decrypted = memcmp(&ctx->metadata, ctx->mariko_bl + 0x20, sizeof(ctx->metadata)) == 0;
            }
        }
    } else {
        fseeko64(ctx->file, 0, SEEK_SET);

        if (fread(&ctx->metadata, 1, sizeof(ctx->metadata), ctx->file) != sizeof(ctx->metadata)) {
            fprintf(stderr, "Failed to read PK11 Metadata!\n");
            exit(EXIT_FAILURE);
        }
    }

    ctx->is_modern = !pk11_is_legacy(ctx);

    if (ctx->is_mariko) {
        if (ctx->is_decrypted) {
            if (ctx->is_modern) {
                memcpy(&ctx->stage1.modern, ctx->mariko_bl + 0x20, sizeof(ctx->stage1.modern));
                ctx->pk11_size = ctx->stage1.modern.pk11_size;
            } else {
                memcpy(&ctx->stage1.legacy, ctx->mariko_bl + 0x20, sizeof(ctx->stage1.legacy));
                ctx->pk11_size = ctx->stage1.legacy.pk11_size;
            }
        } else {
            if (ctx->is_modern) {
                ctx->pk11_size = ctx->mariko_oem_header.bl_size - 0x20 - sizeof(ctx->stage1.modern);
            } else {
                ctx->pk11_size = ctx->mariko_oem_header.bl_size - 0x20 - sizeof(ctx->stage1.legacy);
            }
        }
    } else {
        if (ctx->is_modern) {
            if (fread(&ctx->stage1.modern, 1, sizeof(ctx->stage1.modern), ctx->file) != sizeof(ctx->stage1.modern)) {
                fprintf(stderr, "Failed to read PK11 Stage1!\n");
                exit(EXIT_FAILURE);
            }
            ctx->pk11_size = ctx->stage1.modern.pk11_size;
        } else {
            if (fread(&ctx->stage1.legacy, 1, sizeof(ctx->stage1.legacy), ctx->file) != sizeof(ctx->stage1.legacy)) {
                fprintf(stderr, "Failed to read PK11 Stage1!\n");
                exit(EXIT_FAILURE);
            }
            ctx->pk11_size = ctx->stage1.legacy.pk11_size;
        }
    }

    ctx->pk11 = calloc(1, ctx->pk11_size);
    if (ctx->pk11 == NULL) {
        fprintf(stderr, "Failed to allocate PK11!\n");
        exit(EXIT_FAILURE);
    }

    if (ctx->is_mariko) {
        if (ctx->is_modern) {
            memcpy(ctx->pk11, ctx->mariko_bl + 0x20 + sizeof(ctx->stage1.modern), ctx->pk11_size);
        } else {
            memcpy(ctx->pk11, ctx->mariko_bl + 0x20 + sizeof(ctx->stage1.legacy), ctx->pk11_size);
        }
    } else {
        if (fread(ctx->pk11, 1, ctx->pk11_size, ctx->file) != ctx->pk11_size) {
            fprintf(stderr, "Failed to read PK11!\n");
            exit(EXIT_FAILURE);
        }

        if (ctx->is_modern) {
            if (fread(&ctx->pk11_mac, 1, sizeof(ctx->pk11_mac), ctx->file) != sizeof(ctx->pk11_mac)) {
                fprintf(stderr, "Failed to read PK11 MAC!\n");
                exit(EXIT_FAILURE);
            }
        }
    }

    ctx->is_decrypted = ctx->pk11->magic == MAGIC_PK11;
    if (!ctx->is_mariko && !ctx->is_decrypted) {
        pk11_t dec_header;
        aes_ctx_t *crypt_ctx = NULL;
        if (ctx->is_modern) {
            for (unsigned int i = 6; i < 0x20 && !ctx->is_decrypted; i++) {
                ctx->key_rev = i;
                crypt_ctx = new_aes_ctx(ctx->tool_ctx->settings.keyset.package1_keys[i], 0x10, AES_MODE_CBC);
                aes_setiv(crypt_ctx, ctx->stage1.modern.iv, 0x10);
                aes_decrypt(crypt_ctx, &dec_header, ctx->pk11, sizeof(dec_header));
                if (dec_header.magic == MAGIC_PK11) {
                    aes_setiv(crypt_ctx, ctx->stage1.modern.iv, 0x10);
                    aes_decrypt(crypt_ctx, ctx->pk11, ctx->pk11, ctx->pk11_size);
                    ctx->is_decrypted = 1;
                }
                free_aes_ctx(crypt_ctx);
                crypt_ctx = NULL;
            }
        } else {
            for (unsigned int i = 0; i < 6 && !ctx->is_decrypted; i++) {
                ctx->key_rev = i;
                crypt_ctx = new_aes_ctx(ctx->tool_ctx->settings.keyset.package1_keys[i], 0x10, AES_MODE_CTR);
                aes_setiv(crypt_ctx, ctx->stage1.legacy.ctr, 0x10);
                aes_decrypt(crypt_ctx, &dec_header, ctx->pk11, sizeof(dec_header));
                if (dec_header.magic == MAGIC_PK11) {
                    aes_setiv(crypt_ctx, ctx->stage1.legacy.ctr, 0x10);
                    aes_decrypt(crypt_ctx, ctx->pk11, ctx->pk11, ctx->pk11_size);
                    ctx->is_decrypted = 1;
                }
                free_aes_ctx(crypt_ctx);
                crypt_ctx = NULL;
            }
        }
    }

    if (ctx->is_decrypted) {
        uint64_t pk11_size = 0x20 + pk11_get_warmboot_bin_size(ctx) + pk11_get_nx_bootloader_size(ctx) + pk11_get_secmon_size(ctx);
        pk11_size = align64(pk11_size, 0x10);
        if (pk11_size != ctx->pk11_size) {
            fprintf(stderr, "PK11 seems corrupt!\n");
            exit(EXIT_FAILURE);
        }
    }

    if (ctx->tool_ctx->action & ACTION_INFO) {
        pk11_print(ctx);
    }

    if (ctx->tool_ctx->action & ACTION_EXTRACT) {
        pk11_save(ctx);
    }
}

void pk11_print(pk11_ctx_t *ctx) {
    if (ctx->is_mariko) {
        printf("Mariko OEM Header:\n");
        memdump(stdout, "    Signature:                      ", &ctx->mariko_oem_header.rsa_sig, sizeof(ctx->mariko_oem_header.rsa_sig));
        memdump(stdout, "    Random Salt:                    ", &ctx->mariko_oem_header.salt, sizeof(ctx->mariko_oem_header.salt));
        memdump(stdout, "    OEM Bootloader Hash:            ", &ctx->mariko_oem_header.hash, sizeof(ctx->mariko_oem_header.hash));
        printf("    OEM Bootloader Version:         %02"PRIx32"\n", ctx->mariko_oem_header.bl_version);
        printf("    OEM Bootloader Size:            %08"PRIx32"\n", ctx->mariko_oem_header.bl_size);
        printf("    OEM Bootloader Load Address:    %08"PRIx32"\n", ctx->mariko_oem_header.bl_load_addr);
        printf("    OEM Bootloader Entrypoint:      %08"PRIx32"\n", ctx->mariko_oem_header.bl_entrypoint);
    }
    printf("Package1 Metadata:\n");
    {
        char build_date[sizeof(ctx->metadata.build_date) + 1] = {0};
        memcpy(build_date, ctx->metadata.build_date, sizeof(ctx->metadata.build_date));
        printf("    Build Date:                     %s\n", build_date);
    }
    memdump(stdout, "    Package1ldr Hash:               ", &ctx->metadata.ldr_hash, sizeof(uint32_t));
    memdump(stdout, "    Secure Monitor Hash:            ", &ctx->metadata.sm_hash,  sizeof(uint32_t));
    memdump(stdout, "    NX Bootloader Hash:             ", &ctx->metadata.bl_hash,  sizeof(uint32_t));
    printf("    Version:                        %02"PRIx32"\n", ctx->metadata.version);
    if (ctx->is_decrypted) {
        printf("PK11:\n");
        if (!ctx->is_mariko) {
            printf("    Key Revision:                   %02"PRIx32" (%s)\n", ctx->key_rev, get_key_revision_summary((uint8_t)ctx->key_rev));
        }
        printf("    PK11 Size:                      %08"PRIx32"\n", ctx->pk11_size);
        printf("    Warmboot.bin Size:              %08"PRIx32"\n", pk11_get_warmboot_bin_size(ctx));
        printf("    NX_Bootloader.bin Size          %08"PRIx32"\n", pk11_get_nx_bootloader_size(ctx));
        printf("    Secure_Monitor.bin Size:        %08"PRIx32"\n", pk11_get_secmon_size(ctx));
    }
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
    if (dirpath != NULL && dirpath->valid == VALIDITY_VALID && ctx->is_decrypted) {
        os_makedir(dirpath->os_path);

        /* Save Decrypted.bin */
        printf("Saving decrypted binary to %s/Decrypted.bin\n", dirpath->char_path);
        if (ctx->is_mariko) {
            char *decrypted_bin = malloc(sizeof(ctx->mariko_oem_header) + ctx->mariko_oem_header.bl_size);
            if (decrypted_bin == NULL) {
                fprintf(stderr, "Failed to allocate buffer!\n");
                exit(EXIT_FAILURE);
            }
            memcpy(decrypted_bin, &ctx->mariko_oem_header, sizeof(ctx->mariko_oem_header));
            memcpy(decrypted_bin + sizeof(ctx->mariko_oem_header), ctx->mariko_bl, ctx->mariko_oem_header.bl_size);
            save_buffer_to_directory_file(decrypted_bin, sizeof(ctx->mariko_oem_header) + ctx->mariko_oem_header.bl_size, dirpath, "Decrypted.bin");
            free(decrypted_bin);
        } else {
            char *decrypted_bin = malloc(sizeof(ctx->stage1) + ctx->pk11_size);
            if (decrypted_bin == NULL) {
                fprintf(stderr, "Failed to allocate buffer!\n");
                exit(EXIT_FAILURE);
            }
            memcpy(decrypted_bin, &ctx->stage1, sizeof(ctx->stage1));
            memcpy(decrypted_bin + sizeof(ctx->stage1), ctx->pk11, ctx->pk11_size);
            save_buffer_to_directory_file(decrypted_bin, sizeof(ctx->stage1) + ctx->pk11_size, dirpath, "Decrypted.bin");
            free(decrypted_bin);
        }

        /* Save Mariko_OEM_Bootloader.bin */
        if (ctx->is_mariko) {
            printf("Saving Mariko_OEM_Bootloader.bin to %s/Mariko_OEM_Bootloader.bin...\n", dirpath->char_path);
            save_buffer_to_directory_file(ctx->mariko_bl, ctx->mariko_oem_header.bl_size, dirpath, "Mariko_OEM_Bootloader.bin");
        }

        /* Save Warmboot.bin */
        printf("Saving Warmboot.bin to %s/Warmboot.bin...\n", dirpath->char_path);
        save_buffer_to_directory_file(pk11_get_warmboot_bin(ctx), pk11_get_warmboot_bin_size(ctx), dirpath, "Warmboot.bin");

        if (ctx->is_mariko) {

            uint32_t wb_size = pk11_get_warmboot_bin_size(ctx);

            unsigned char *wb_dec = malloc(wb_size);
            if (wb_dec == NULL) {
                fprintf(stderr, "Failed to allocate mariko warmboot binary!\n");
                exit(EXIT_FAILURE);
            }

            memcpy(wb_dec, pk11_get_warmboot_bin(ctx), wb_size);

            if (wb_size > 0x330) {
                aes_ctx_t *crypt_ctx = new_aes_ctx(ctx->tool_ctx->settings.keyset.mariko_bek, 0x10, AES_MODE_CBC);

                unsigned char iv[0x10] = {0};
                aes_setiv(crypt_ctx, iv, 0x10);

                aes_decrypt(crypt_ctx, wb_dec + 0x330, wb_dec + 0x330, wb_size - 0x330);

                free_aes_ctx(crypt_ctx);
            }

            printf("Saving Warmboot_Decrypted.bin to %s/Warmboot_Decrypted.bin...\n", dirpath->char_path);
            save_buffer_to_directory_file(wb_dec, wb_size, dirpath, "Warmboot_Decrypted.bin");

            free(wb_dec);
        }

        /* Save NX_Bootloader.bin */
        printf("Saving NX_Bootloader.bin to %s/NX_Bootloader.bin...\n", dirpath->char_path);
        save_buffer_to_directory_file(pk11_get_nx_bootloader(ctx), pk11_get_nx_bootloader_size(ctx), dirpath, "NX_Bootloader.bin");

        /* Save Secure_Monitor.bin */
        printf("Saving Secure_Monitor.bin to %s/Secure_Monitor.bin...\n", dirpath->char_path);
        save_buffer_to_directory_file(pk11_get_secmon(ctx), pk11_get_secmon_size(ctx), dirpath, "Secure_Monitor.bin");
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
    /* Support 8.0.0 INI1 embedded in Kernel */
    if (ctx->header.section_sizes[1] > 0) {
        ctx->ini1_ctx.header = (ini1_header_t *)(ctx->sections + ctx->header.section_sizes[0]);
    } else {
        ctx->ini1_ctx.header = (ini1_header_t *)(ctx->sections);
        for (offset = 0; offset < ctx->header.section_sizes[0] - 4; offset += 4) {
            if (*(uint32_t *)(ctx->sections + offset) == MAGIC_KRNLLDR_STRCT_END) {
                ctx->kernel_map = (kernel_map_t *)(ctx->sections + offset - sizeof(kernel_map_t));
                ctx->ini1_ctx.header = (ini1_header_t *)(ctx->sections + ctx->kernel_map->ini1_start_offset);
                break;
            }
        }
    }
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

static const char *pk21_get_section_name(int section, bool is_ini1_embedded) {
    switch (section) {
        case 0: return "Kernel";
        case 1:
            if (is_ini1_embedded)
                return "Empty";
            else
                return "INI1";
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

    bool is_ini1_embedded = ctx->header.section_sizes[1] == 0;
    for (unsigned int i = 0; i < 3; i++) {
        printf("    Section %"PRId32" (%s):\n", i, pk21_get_section_name(i, is_ini1_embedded));
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
        if (ctx->header.section_sizes[1] > 0)
            save_buffer_to_directory_file(ctx->sections + ctx->header.section_sizes[0], ctx->header.section_sizes[1], dirpath, "INI1.bin");
        else
            save_buffer_to_directory_file(ctx->sections + ctx->kernel_map->ini1_start_offset, ctx->ini1_ctx.header->size, dirpath, "INI1.bin");
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
