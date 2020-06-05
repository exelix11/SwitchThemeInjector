#include <string.h>
#include "pfs0.h"

void pfs0_process(pfs0_ctx_t *ctx) {
    /* Read *just* safe amount. */
    pfs0_header_t raw_header;
    fseeko64(ctx->file, 0, SEEK_SET);
    if (fread(&raw_header, 1, sizeof(raw_header), ctx->file) != sizeof(raw_header)) {
        fprintf(stderr, "Failed to read PFS0 header!\n");
        exit(EXIT_FAILURE);
    }

    if (raw_header.magic != MAGIC_PFS0) {
        printf("Error: PFS0 is corrupt!\n");
        exit(EXIT_FAILURE);
    }

    uint64_t header_size = pfs0_get_header_size(&raw_header);
    ctx->header = malloc(header_size);
    if (ctx->header == NULL) {
        fprintf(stderr, "Failed to allocate PFS0 header!\n");
        exit(EXIT_FAILURE);
    }

    fseeko64(ctx->file, 0, SEEK_SET);
    if (fread(ctx->header, 1, header_size, ctx->file) != header_size) {
        fprintf(stderr, "Failed to read PFS0 header!\n");
        exit(EXIT_FAILURE);
    }

    /* Weak file validation. */
    uint64_t max_size = 0x1ULL;
    max_size <<= 48; /* Switch file sizes are capped at 48 bits. */
    uint64_t cur_ofs = 0;
    for (unsigned int i = 0; i < ctx->header->num_files; i++) {
        pfs0_file_entry_t *cur_file = pfs0_get_file_entry(ctx->header, i);
        cur_ofs += cur_file->size;
    }

    for (unsigned int i = 0; i < ctx->header->num_files; i++) {
        pfs0_file_entry_t *cur_file = pfs0_get_file_entry(ctx->header, i);
        if (strcmp(pfs0_get_file_name(ctx->header, i), "main.npdm") == 0) {
            /* We might have found the exefs... */

            ctx->npdm = malloc(cur_file->size);
            if (ctx->npdm == NULL) {
                fprintf(stderr, "Failed to allocate NPDM!\n");
                exit(EXIT_FAILURE);
            }
            fseeko64(ctx->file, pfs0_get_header_size(ctx->header) + cur_file->offset, SEEK_SET);
            if (fread(ctx->npdm, 1, cur_file->size, ctx->file) != cur_file->size) {
                fprintf(stderr, "Failed to read NPDM!\n");
                exit(EXIT_FAILURE);
            }

            if (ctx->npdm->magic == MAGIC_META) {
                ctx->is_exefs = 1;
            }
        }
    }

    if (ctx->tool_ctx->action & ACTION_INFO) {
        pfs0_print(ctx);
    }

    if (ctx->tool_ctx->action & ACTION_EXTRACT) {
        pfs0_save(ctx);
    }

}

static void pfs0_save_file(pfs0_ctx_t *ctx, uint32_t i, filepath_t *dirpath) {
    if (i >= ctx->header->num_files) {
        fprintf(stderr, "Could not save file %"PRId32"!\n", i);
        exit(EXIT_FAILURE);
    }
    pfs0_file_entry_t *cur_file = pfs0_get_file_entry(ctx->header, i);

    if (strlen(pfs0_get_file_name(ctx->header, i)) >= MAX_PATH - strlen(dirpath->char_path) - 1) {
        fprintf(stderr, "Filename too long in PFS0!\n");
        exit(EXIT_FAILURE);
    }

    filepath_t filepath;
    filepath_copy(&filepath, dirpath);
    filepath_append(&filepath, "%s", pfs0_get_file_name(ctx->header, i));

    printf("Saving %s to %s...\n", pfs0_get_file_name(ctx->header, i), filepath.char_path);
    uint64_t ofs = pfs0_get_header_size(ctx->header) + cur_file->offset;
    save_file_section(ctx->file, ofs, cur_file->size, &filepath);
}


void pfs0_save(pfs0_ctx_t *ctx) {
    /* Extract to directory. */
    filepath_t *dirpath = NULL;
    if (ctx->is_exefs && ctx->tool_ctx->settings.exefs_dir_path.enabled) {
        dirpath = &ctx->tool_ctx->settings.exefs_dir_path.path;
    }
    if ((dirpath == NULL || dirpath->valid != VALIDITY_VALID) && (ctx->tool_ctx->file_type == FILETYPE_PFS0 && ctx->tool_ctx->settings.out_dir_path.enabled)) {
        dirpath = &ctx->tool_ctx->settings.out_dir_path.path;
    }
    if (dirpath == NULL || dirpath->valid != VALIDITY_VALID) {
        dirpath = &ctx->tool_ctx->settings.pfs0_dir_path;
    }
    if (dirpath != NULL && dirpath->valid == VALIDITY_VALID) {
        os_makedir(dirpath->os_path);
        for (uint32_t i = 0; i < ctx->header->num_files; i++) {
            pfs0_save_file(ctx, i, dirpath);
        }
    }
}

void pfs0_print(pfs0_ctx_t *ctx) {
    printf("\n%s:\n", ctx->is_exefs ? "ExeFS" : "PFS0");
    print_magic("Magic:                              ", ctx->header->magic);
    if (ctx->is_exefs) {
        printf("Title ID:                           %016"PRIx64"\n", npdm_get_aci0(ctx->npdm)->title_id);
    }
    printf("Number of files:                    %"PRId32"\n", ctx->header->num_files);
    if (ctx->header->num_files > 0 && ctx->header->num_files < 15) { /* Arbitrary. */
        printf("Files:");
        for (unsigned int i = 0; i < ctx->header->num_files; i++) {
            pfs0_file_entry_t *cur_file = pfs0_get_file_entry(ctx->header, i);
            printf("%spfs0:/%-32s %012"PRIx64"-%012"PRIx64"\n", i == 0 ? "                              " : "                                    ", pfs0_get_file_name(ctx->header, i), cur_file->offset, cur_file->offset + cur_file->size);
        }
    }
    if (ctx->is_exefs) {
        npdm_process(ctx->npdm, ctx->tool_ctx);
    }
}
