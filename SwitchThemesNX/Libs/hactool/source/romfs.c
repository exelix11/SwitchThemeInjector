#include <stdio.h>
#include "types.h"
#include "utils.h"
#include "ivfc.h"

/* RomFS functions... */
static void romfs_visit_file(romfs_ctx_t *ctx, uint32_t file_offset, filepath_t *dir_path) {
    filepath_t *cur_path = calloc(1, sizeof(filepath_t));
    if (cur_path == NULL) {
        fprintf(stderr, "Failed to allocate filepath!\n");
        exit(EXIT_FAILURE);
    }

    while (file_offset != ROMFS_ENTRY_EMPTY) {
        romfs_fentry_t *entry = romfs_get_fentry(ctx->files, file_offset);

        filepath_copy(cur_path, dir_path);
        if (entry->name_size) {
            filepath_append_n(cur_path, entry->name_size, "%s", entry->name);
        }

        /* If we're extracting... */
        if ((ctx->tool_ctx->action & ACTION_LISTROMFS) == 0) {
            printf("Saving %s...\n", cur_path->char_path);
            save_file_section(ctx->file, ctx->romfs_offset + ctx->header.data_offset + entry->offset, entry->size, cur_path);
        } else {
            printf("rom:%s\n", cur_path->char_path);
        }

        file_offset = entry->sibling;
    }

    free(cur_path);
}

static void romfs_visit_dir(romfs_ctx_t *ctx, uint32_t dir_offset, filepath_t *parent_path) {
    romfs_direntry_t *entry = romfs_get_direntry(ctx->directories, dir_offset);
    filepath_t *cur_path = calloc(1, sizeof(filepath_t));
    if (cur_path == NULL) {
        fprintf(stderr, "Failed to allocate filepath!\n");
        exit(EXIT_FAILURE);
    }

    filepath_copy(cur_path, parent_path);
    if (entry->name_size) {
        filepath_append_n(cur_path, entry->name_size, "%s", entry->name);
    }

    /* If we're actually extracting the romfs, make directory. */
    if ((ctx->tool_ctx->action & ACTION_LISTROMFS) == 0) {
        os_makedir(cur_path->os_path);
    }

    if (entry->file != ROMFS_ENTRY_EMPTY) {
        romfs_visit_file(ctx, entry->file, cur_path);
    }
    if (entry->child != ROMFS_ENTRY_EMPTY) {
        romfs_visit_dir(ctx, entry->child, cur_path);
    }
    if (entry->sibling != ROMFS_ENTRY_EMPTY) {
        romfs_visit_dir(ctx, entry->sibling, parent_path);
    }

    free(cur_path);
}

void romfs_process(romfs_ctx_t *ctx) {
    ctx->romfs_offset = 0;
    fseeko64(ctx->file, ctx->romfs_offset, SEEK_SET);
    if (fread(&ctx->header, 1, sizeof(romfs_hdr_t), ctx->file) != sizeof(romfs_hdr_t)) {
        fprintf(stderr, "Failed to read RomFS header!\n");
        return;
    }

    if ((ctx->tool_ctx->action & (ACTION_EXTRACT | ACTION_LISTROMFS)) && ctx->header.header_size == ROMFS_HEADER_SIZE) {
        /* Pre-load the file/data entry caches. */
        ctx->directories = calloc(1, ctx->header.dir_meta_table_size);
        if (ctx->directories == NULL) {
            fprintf(stderr, "Failed to allocate RomFS directory cache!\n");
            exit(EXIT_FAILURE);
        }

        fseeko64(ctx->file, ctx->romfs_offset + ctx->header.dir_meta_table_offset, SEEK_SET);
        if (fread(ctx->directories, 1, ctx->header.dir_meta_table_size, ctx->file) != ctx->header.dir_meta_table_size) {
            fprintf(stderr, "Failed to read RomFS directory cache!\n");
            exit(EXIT_FAILURE);
        }

        ctx->files = calloc(1, ctx->header.file_meta_table_size);
        if (ctx->files == NULL) {
            fprintf(stderr, "Failed to allocate RomFS file cache!\n");
            exit(EXIT_FAILURE);
        }
        fseeko64(ctx->file, ctx->romfs_offset + ctx->header.file_meta_table_offset, SEEK_SET);
        if (fread(ctx->files, 1, ctx->header.file_meta_table_size, ctx->file) != ctx->header.file_meta_table_size) {
            fprintf(stderr, "Failed to read RomFS file cache!\n");
            exit(EXIT_FAILURE);
        }
    }

    /* If there's ever anything meaningful to print about RomFS, uncomment and implement.
     *
     * if (ctx->tool_ctx->action & ACTION_INFO) {
     *    romfs_print(ctx);
     * }
     */

    if (ctx->tool_ctx->action & ACTION_EXTRACT) {
        romfs_save(ctx);
    }

}

void romfs_save(romfs_ctx_t *ctx) {
    if (ctx->tool_ctx->action & ACTION_LISTROMFS) {
        filepath_t fakepath;
        filepath_init(&fakepath);
        filepath_set(&fakepath, "");

        printf("Visiting romfs\n");

        romfs_visit_dir(ctx, 0, &fakepath);
    } else {
        /* Extract to directory. */
        filepath_t *dirpath = NULL;
        if (ctx->tool_ctx->settings.romfs_dir_path.enabled) {
            dirpath = &ctx->tool_ctx->settings.romfs_dir_path.path;
        }
        if ((dirpath == NULL || dirpath->valid != VALIDITY_VALID) && (ctx->tool_ctx->file_type == FILETYPE_ROMFS && ctx->tool_ctx->settings.out_dir_path.enabled)) {
            dirpath = &ctx->tool_ctx->settings.out_dir_path.path;
        }
        if (dirpath != NULL && dirpath->valid == VALIDITY_VALID) {
            os_makedir(dirpath->os_path);
            romfs_visit_dir(ctx, 0, dirpath);
        }
    }

}

void romfs_print(romfs_ctx_t *ctx) {
    /* Is there anything meaningful to print here? */
    fprintf(stderr, "Error: RomFS printing not implemented.\n");
}
