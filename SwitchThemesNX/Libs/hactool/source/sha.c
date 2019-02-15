#include <stdlib.h>
#include <stdio.h>
#include "sha.h"
#include "types.h"
#include "utils.h"

/* Allocate new context. */
sha_ctx_t *new_sha_ctx(hash_type_t type, int hmac) {
    sha_ctx_t *ctx;
    
    if ((ctx = malloc(sizeof(*ctx))) == NULL) {
        FATAL_ERROR("Failed to allocate sha_ctx_t!");
    }
    
    mbedtls_md_init(&ctx->digest);
    
    if (mbedtls_md_setup(&ctx->digest, mbedtls_md_info_from_type(type), hmac)) {
        FATAL_ERROR("Failed to set up hash context!");
    }
    
    if (mbedtls_md_starts(&ctx->digest)) {
        FATAL_ERROR("Failed to start hash context!");
    }
    
    return ctx;
}

/* Free an allocated context. */
void free_sha_ctx(sha_ctx_t *ctx) {
    /* Explicitly allow NULL. */
    if (ctx == NULL) {
        return;
    }
    
    mbedtls_md_free(&ctx->digest);
    free(ctx);
}

/* Update digest with new data. */
void sha_update(sha_ctx_t *ctx, const void *data, size_t l) {
    mbedtls_md_update(&ctx->digest, data, l);
}

/* Read hash from context. */
void sha_get_hash(sha_ctx_t *ctx, unsigned char *hash) {
    mbedtls_md_finish(&ctx->digest, hash);
}

/* SHA256 digest. */
void sha256_hash_buffer(unsigned char *digest, const void *data, size_t l) {
    sha_ctx_t *sha_ctx = new_sha_ctx(HASH_TYPE_SHA256, 0);
    sha_update(sha_ctx, data, l);
    sha_get_hash(sha_ctx, digest);
    free_sha_ctx(sha_ctx);
}

/* SHA256-HMAC digest. */
void sha256_get_buffer_hmac(void *digest, const void *secret, size_t s_l, const void *data, size_t d_l) {
    sha_ctx_t *ctx;
    
    if ((ctx = malloc(sizeof(*ctx))) == NULL) {
        FATAL_ERROR("Failed to allocate sha_ctx_t!");
    }
    
    mbedtls_md_init(&ctx->digest);
    
    if (mbedtls_md_setup(&ctx->digest, mbedtls_md_info_from_type(HASH_TYPE_SHA256), 1)) {
        FATAL_ERROR("Failed to set up hash context!");
    }
    
    if (mbedtls_md_hmac_starts(&ctx->digest, secret, s_l)) {
        FATAL_ERROR("Failed to set up HMAC secret context!");
    }
    
    if (mbedtls_md_hmac_update(&ctx->digest, data, d_l)) {
        FATAL_ERROR("Failed processing HMAC input!");
    }
    
    if (mbedtls_md_hmac_finish(&ctx->digest, digest)) {
        FATAL_ERROR("Failed getting HMAC output!");
    }
    
    mbedtls_md_free(&ctx->digest);
    free(ctx);
}