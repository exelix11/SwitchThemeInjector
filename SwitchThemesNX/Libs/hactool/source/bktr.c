#include "bktr.h"
#include "utils.h"

bktr_relocation_bucket_t *bktr_get_relocation_bucket(bktr_relocation_block_t *block, uint32_t i) {
    return (bktr_relocation_bucket_t *)((uint8_t *)block->buckets + (sizeof(bktr_relocation_bucket_t) + sizeof(bktr_relocation_entry_t)) * i);
}

/* Get a relocation entry from offset and relocation block. */
bktr_relocation_entry_t *bktr_get_relocation(bktr_relocation_block_t *block, uint64_t offset) {
    /* Weak check for invalid offset. */
    if (offset > block->total_size) {
        fprintf(stderr, "Too big offset looked up in BKTR relocation table!\n");
        exit(EXIT_FAILURE);
    }
    
    uint32_t bucket_num = 0;
    for (unsigned int i = 1; i < block->num_buckets; i++) {
        if (block->bucket_virtual_offsets[i] <= offset) {
            bucket_num++;
        }
    }
    
    bktr_relocation_bucket_t *bucket = bktr_get_relocation_bucket(block, bucket_num);
    
    if (bucket->num_entries == 1) { /* Check for edge case, short circuit. */
        return &bucket->entries[0];
    }
    
    /* Binary search. */
    uint32_t low = 0, high = bucket->num_entries - 1;
    while (low <= high) {
        uint32_t mid = (low + high) / 2;
        if (bucket->entries[mid].virt_offset > offset) { /* Too high. */
            high = mid - 1;
        } else { /* block->entries[mid].offset <= offset. */
            /* Check for success. */
            if (mid == bucket->num_entries - 1 || bucket->entries[mid+1].virt_offset > offset) {
                return &bucket->entries[mid];
            }
            low = mid + 1;
        }
    }
    fprintf(stderr, "Failed to find offset %012"PRIx64" in BKTR relocation table!\n", offset);
    exit(EXIT_FAILURE);
}

bktr_subsection_bucket_t *bktr_get_subsection_bucket(bktr_subsection_block_t *block, uint32_t i) {
    return (bktr_subsection_bucket_t *)((uint8_t *)block->buckets + (sizeof(bktr_subsection_bucket_t) + sizeof(bktr_subsection_entry_t)) * i);
}

/* Get a subsection entry from offset and subsection block .*/
bktr_subsection_entry_t *bktr_get_subsection(bktr_subsection_block_t *block, uint64_t offset) {
    /* If offset is past the virtual, we're reading from the BKTR_HEADER subsection. */
    bktr_subsection_bucket_t *last_bucket = bktr_get_subsection_bucket(block, block->num_buckets - 1);
    if (offset >= last_bucket->entries[last_bucket->num_entries].offset) {
        return &last_bucket->entries[last_bucket->num_entries];
    }
    
    uint32_t bucket_num = 0;
    for (unsigned int i = 1; i < block->num_buckets; i++) {
        if (block->bucket_physical_offsets[i] <= offset) {
            bucket_num++;
        }
    }
    
    bktr_subsection_bucket_t *bucket = bktr_get_subsection_bucket(block, bucket_num);
    
    if (bucket->num_entries == 1) { /* Check for edge case, short circuit. */
        return &bucket->entries[0];
    }
    
    /* Binary search. */
    uint32_t low = 0, high = bucket->num_entries - 1;
    while (low <= high) {
        uint32_t mid = (low + high) / 2;
        if (bucket->entries[mid].offset > offset) { /* Too high. */
            high = mid - 1;
        } else { /* block->entries[mid].offset <= offset. */
            /* Check for success. */
            if (mid == bucket->num_entries - 1 || bucket->entries[mid+1].offset > offset) {
                return &bucket->entries[mid];
            }
            low = mid + 1;
        }
    }
    fprintf(stderr, "Failed to find offset %012"PRIx64" in BKTR subsection table!\n", offset);
    exit(EXIT_FAILURE);
}
