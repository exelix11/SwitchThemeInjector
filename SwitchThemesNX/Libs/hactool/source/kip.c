#include <string.h>
#include <stdio.h>
#include "kip.h"
#include "npdm.h"
#include "cJSON.h"

void ini1_process(ini1_ctx_t *ctx) {
    /* Read *just* safe amount. */
    ini1_header_t raw_header; 
    fseeko64(ctx->file, 0, SEEK_SET);
    if (fread(&raw_header, 1, sizeof(raw_header), ctx->file) != sizeof(raw_header)) {
        fprintf(stderr, "Failed to read INI1 header!\n");
        exit(EXIT_FAILURE);
    }
    
    if (raw_header.magic != MAGIC_INI1 || raw_header.num_processes > INI1_MAX_KIPS) {
        printf("Error: INI1 is corrupt!\n");
        exit(EXIT_FAILURE);
    }

    ctx->header = malloc(raw_header.size);
    if (ctx->header == NULL) {
        fprintf(stderr, "Failed to allocate INI1 header!\n");
        exit(EXIT_FAILURE);
    }
    
    fseeko64(ctx->file, 0, SEEK_SET);
    if (fread(ctx->header, 1, raw_header.size, ctx->file) != raw_header.size) {
        fprintf(stderr, "Failed to read INI1!\n");
        exit(EXIT_FAILURE);
    }
    
    uint64_t offset = 0;
    for (unsigned int i = 0; i < ctx->header->num_processes; i++) {
        ctx->kips[i].tool_ctx = ctx->tool_ctx;
        ctx->kips[i].header = (kip1_header_t *)&ctx->header->kip_data[offset];
        if (ctx->kips[i].header->magic != MAGIC_KIP1) {
            fprintf(stderr, "INI1 is corrupted!\n");
            exit(EXIT_FAILURE);
        }
        offset += kip1_get_size(&ctx->kips[i]);
    }
    
    if (ctx->tool_ctx->action & ACTION_INFO) {
        ini1_print(ctx);
    }
    
    if (ctx->tool_ctx->action & ACTION_EXTRACT) {
        ini1_save(ctx);
    }
}

void ini1_print(ini1_ctx_t *ctx) {
    printf("INI1:\n");
    printf("    Number of Processes:            %02"PRIx32"\n", ctx->header->num_processes);
    printf("    Size:                           %08"PRIx32"\n", ctx->header->size);
    printf("\n");
    for (unsigned int i = 0; i < ctx->header->num_processes; i++) {
        printf("Process %02"PRIx32":\n", i);
        kip1_print(&ctx->kips[i], 1);
        printf("\n");
    }
    printf("\n");
}

void ini1_save(ini1_ctx_t *ctx) {
    filepath_t *dirpath = NULL;
    if (ctx->tool_ctx->file_type == FILETYPE_INI1 && ctx->tool_ctx->settings.out_dir_path.enabled) {
        dirpath = &ctx->tool_ctx->settings.out_dir_path.path;
    }
    if (dirpath == NULL || dirpath->valid != VALIDITY_VALID) {
        dirpath = &ctx->tool_ctx->settings.ini1_dir_path;
    }
    if (dirpath != NULL && dirpath->valid == VALIDITY_VALID) {
        os_makedir(dirpath->os_path);
        for (unsigned int i = 0; i < ctx->header->num_processes; i++) {
            char padded_name[0x20];
            memset(&padded_name, 0, sizeof(padded_name));
            memcpy(&padded_name, ctx->kips[i].header->name, sizeof(ctx->kips[i].header->name));
            strcat(padded_name, ".kip1");
            printf("Saving %s to %s/%s...\n", padded_name, dirpath->char_path, padded_name);
            save_buffer_to_directory_file(ctx->kips[i].header, kip1_get_size(&ctx->kips[i]), dirpath, padded_name);
            if (ctx->tool_ctx->action & ACTION_SAVEINIJSON) {
                printf("SAVING INI JSON!\n");
                memset(&padded_name, 0, sizeof(padded_name));
                memcpy(&padded_name, ctx->kips[i].header->name, sizeof(ctx->kips[i].header->name));
                strcat(padded_name, ".json");
                filepath_t json_path;
                filepath_init(&json_path);
                filepath_copy(&json_path, dirpath);
                filepath_append(&json_path, padded_name);
                FILE *f_json = os_fopen(json_path.os_path, OS_MODE_WRITE);
                if (f_json == NULL) {
                    fprintf(stderr, "Failed to open %s!\n", json_path.char_path);
                    return;
                }
                const char *json = kip1_get_json(&ctx->kips[i]);
                if (fwrite(json, 1, strlen(json), f_json) != strlen(json)) {
                    fprintf(stderr, "Failed to write JSON file!\n");
                    exit(EXIT_FAILURE);
                }
                fclose(f_json);
            }
        }
    }
}

char *kip1_get_json(kip1_ctx_t *ctx) {
    cJSON *kip_json = cJSON_CreateObject();
    char *output_str = NULL;
    char work_buffer[0x300] = {0};
    
    /* Add KIP1 header fields. */
    strcpy(work_buffer, ctx->header->name);
    cJSON_AddStringToObject(kip_json, "name", work_buffer);
    cJSON_AddU64ToObject(kip_json, "title_id", ctx->header->title_id);
    cJSON_AddU32ToObject(kip_json, "main_thread_stack_size", ctx->header->section_headers[1].attribute);
    cJSON_AddNumberToObject(kip_json, "main_thread_priority", ctx->header->main_thread_priority);
    cJSON_AddNumberToObject(kip_json, "default_cpu_id", ctx->header->default_core);
    cJSON_AddNumberToObject(kip_json, "process_category", ctx->header->process_category);
    
     /* Add KAC. */
    cJSON *kac_json = kac_get_json(ctx->header->capabilities, sizeof(ctx->header->capabilities) / sizeof(uint32_t));
    cJSON_AddItemToObject(kip_json, "kernel_capabilities", kac_json);
    
    output_str = cJSON_Print(kip_json);
    
    cJSON_Delete(kip_json);
    return output_str;
}

static void kip1_blz_uncompress(void *hdr_end) {
    uint32_t addl_size = ((uint32_t *)hdr_end)[-1];
    uint32_t header_size = ((uint32_t *)hdr_end)[-2];
    uint32_t cmp_and_hdr_size = ((uint32_t *)hdr_end)[-3];
    
    unsigned char *cmp_start = (unsigned char *)(((uintptr_t)hdr_end) - cmp_and_hdr_size);
    uint32_t cmp_ofs = cmp_and_hdr_size - header_size;
    uint32_t out_ofs = cmp_and_hdr_size + addl_size;
    
    while (out_ofs) {
        unsigned char control = cmp_start[--cmp_ofs];
        for (unsigned int i = 0; i < 8; i++) {
            if (control & 0x80) {
                if (cmp_ofs < 2) {
                    fprintf(stderr, "KIP1 decompression out of bounds!\n");
                    exit(EXIT_FAILURE);
                }
                cmp_ofs -= 2;
                uint16_t seg_val = ((unsigned int)cmp_start[cmp_ofs+1] << 8) | cmp_start[cmp_ofs];
                uint32_t seg_size = ((seg_val >> 12) & 0xF) + 3;
                uint32_t seg_ofs = (seg_val & 0x0FFF) + 3;
                if (out_ofs < seg_size) {
                    /* Kernel restricts segment copy to stay in bounds. */
                    seg_size = out_ofs;
                }
                out_ofs -= seg_size;
                
                for (unsigned int j = 0; j < seg_size; j++) {
                    cmp_start[out_ofs + j] = cmp_start[out_ofs + j + seg_ofs];
                }
            } else {
                /* Copy directly. */
                if (cmp_ofs < 1) {
                    fprintf(stderr, "KIP1 decompression out of bounds!\n");
                    exit(EXIT_FAILURE);
                }
                cmp_start[--out_ofs] = cmp_start[--cmp_ofs];
            }
            control <<= 1;
            if (out_ofs == 0) {
                return;
            }
        }
    }
}

static void *kip1_uncompress(kip1_ctx_t *ctx, uint64_t *size) {
    /* Make new header with correct sizes, fixed flags. */
    kip1_header_t new_header = *ctx->header;
    for (unsigned int i = 0; i < 3; i++) {
        new_header.section_headers[i].compressed_size = new_header.section_headers[i].out_size;
    }
    new_header.flags &= 0xF8;
    
    *size = kip1_get_size_from_header(&new_header);
    unsigned char *new_kip = calloc(1, *size);
    if (new_kip == NULL) {
        fprintf(stderr, "Failed to allocate uncompressed KIP1!\n");
        exit(EXIT_FAILURE);
    }
    *((kip1_header_t *)new_kip) = new_header;
    
    uint64_t new_offset = 0x100;
    uint64_t old_offset = 0x100;
    for (unsigned int i = 0; i < 3; i++) {
        // Copy in section data */
        memcpy(new_kip + new_offset, (unsigned char *)ctx->header + old_offset, ctx->header->section_headers[i].compressed_size);
        kip1_blz_uncompress(new_kip + new_offset + ctx->header->section_headers[i].compressed_size);
        new_offset += ctx->header->section_headers[i].out_size;
        old_offset += ctx->header->section_headers[i].compressed_size;
    }
    
    return new_kip;
}

void kip1_process(kip1_ctx_t *ctx) {
    /* Read *just* safe amount. */
    kip1_header_t raw_header; 
    fseeko64(ctx->file, 0, SEEK_SET);
    if (fread(&raw_header, 1, sizeof(raw_header), ctx->file) != sizeof(raw_header)) {
        fprintf(stderr, "Failed to read KIP1 header!\n");
        exit(EXIT_FAILURE);
    }
    
    if (raw_header.magic != MAGIC_KIP1) {
        printf("Error: KIP1 is corrupt!\n");
        exit(EXIT_FAILURE);
    }

    uint64_t size = kip1_get_size_from_header(&raw_header);
    ctx->header = malloc(size);
    if (ctx->header == NULL) {
        fprintf(stderr, "Failed to allocate KIP1!\n");
        exit(EXIT_FAILURE);
    }
    
    fseeko64(ctx->file, 0, SEEK_SET);
    if (fread(ctx->header, 1, size, ctx->file) != size) {
        fprintf(stderr, "Failed to read KIP1!\n");
        exit(EXIT_FAILURE);
    }
    
    if (ctx->tool_ctx->action & ACTION_INFO) {
        kip1_print(ctx, 0);
    }
    
    if (ctx->tool_ctx->action & ACTION_EXTRACT) {
        kip1_save(ctx);
    }
}

void kip1_print(kip1_ctx_t *ctx, int suppress) {
    if (!suppress) printf("KIP1:\n");
    printf("    Title ID:                       %016"PRIx64"\n", ctx->header->title_id);
    char padded_name[13];
    memset(&padded_name, 0, sizeof(padded_name));
    memcpy(&padded_name, ctx->header->name, sizeof(ctx->header->name));
    printf("    Name:                           %s\n", padded_name);
    printf("    Process Category:               %s\n", npdm_get_proc_category(ctx->header->process_category));
    printf("    Main Thread Priority:           %"PRId8"\n", ctx->header->main_thread_priority);
    printf("    Default CPU Core:               %"PRId8"\n", ctx->header->default_core);
    printf("    Is 64 Bit:                      %s\n", (ctx->header->flags & (1 << 3)) ? "True" : "False");
    printf("    Is Address Space 64 Bit:        %s\n", (ctx->header->flags & (1 << 4)) ? "True" : "False");
    printf("    Sections:\n");
    printf("        .text:                      %08"PRIx32"-%08"PRIx32"\n", ctx->header->section_headers[0].out_offset, ctx->header->section_headers[0].out_offset + align(ctx->header->section_headers[0].out_size, 0x1000));
    printf("        .rodata:                    %08"PRIx32"-%08"PRIx32"\n", ctx->header->section_headers[1].out_offset, ctx->header->section_headers[1].out_offset + align(ctx->header->section_headers[1].out_size, 0x1000));
    printf("        .rwdata:                    %08"PRIx32"-%08"PRIx32"\n", ctx->header->section_headers[2].out_offset, ctx->header->section_headers[2].out_offset + align(ctx->header->section_headers[2].out_size, 0x1000));
    printf("        .bss:                       %08"PRIx32"-%08"PRIx32"\n", ctx->header->section_headers[3].out_offset, ctx->header->section_headers[3].out_offset + align(ctx->header->section_headers[3].out_size, 0x1000));
    printf("    Kernel Access Control:\n");
    kac_print(ctx->header->capabilities, 0x20);
    printf("\n");
}

void kip1_save(kip1_ctx_t *ctx) {
    filepath_t *json_path = &ctx->tool_ctx->settings.npdm_json_path;
    filepath_t *uncmp_path = &ctx->tool_ctx->settings.uncompressed_path;
    if (ctx->tool_ctx->file_type == FILETYPE_KIP1) {
        if (json_path->valid == VALIDITY_VALID) {
            FILE *f_json = os_fopen(json_path->os_path, OS_MODE_WRITE);
            if (f_json == NULL) {
                fprintf(stderr, "Failed to open %s!\n", json_path->char_path);
                return;
            }
            char *json = kip1_get_json(ctx);
            if (json == NULL) {
                fprintf(stderr, "Failed to allocate KIP1 JSON\n");
                exit(EXIT_FAILURE);
            }
            if (fwrite(json, 1, strlen(json), f_json) != strlen(json)) {
                fprintf(stderr, "Failed to write JSON file!\n");
                exit(EXIT_FAILURE);
            }
            cJSON_free(json);
            fclose(f_json);
        } else if (uncmp_path->valid == VALIDITY_VALID) {
            FILE *f_uncmp = os_fopen(uncmp_path->os_path, OS_MODE_WRITE);
            if (f_uncmp == NULL) {
                fprintf(stderr, "Failed to open %s!\n", uncmp_path->char_path);
                return;
            }
            uint64_t sz = 0;
            void *uncmp = kip1_uncompress(ctx, &sz);
            if (fwrite(uncmp, 1, sz, f_uncmp) != sz) {
                fprintf(stderr, "Failed to write uncompressed kip!\n");
                exit(EXIT_FAILURE);
            }
            free(uncmp);
            fclose(f_uncmp);
        }
    }
}