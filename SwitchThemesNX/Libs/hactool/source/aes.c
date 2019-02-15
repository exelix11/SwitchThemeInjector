#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "aes.h"
#include "types.h"
#include "utils.h"

/* Allocate a new context. */
aes_ctx_t *new_aes_ctx(const void *key, unsigned int key_size, aes_mode_t mode) {
    aes_ctx_t *ctx;
    
    if ((ctx = malloc(sizeof(*ctx))) == NULL) {
        FATAL_ERROR("Failed to allocate aes_ctx_t!");
    }

    mbedtls_cipher_init(&ctx->cipher_dec);
    mbedtls_cipher_init(&ctx->cipher_enc);
    
    if (mbedtls_cipher_setup(&ctx->cipher_dec, mbedtls_cipher_info_from_type(mode))
        || mbedtls_cipher_setup(&ctx->cipher_enc, mbedtls_cipher_info_from_type(mode))) {
        FATAL_ERROR("Failed to set up AES context!");
    }
        
    if (mbedtls_cipher_setkey(&ctx->cipher_dec, key, key_size * 8, AES_DECRYPT)
        || mbedtls_cipher_setkey(&ctx->cipher_enc, key, key_size * 8, AES_ENCRYPT)) {
        FATAL_ERROR("Failed to set key for AES context!");
    }
    
    return ctx;
}

/* Free an allocated context. */
void free_aes_ctx(aes_ctx_t *ctx) {
    /* Explicitly allow NULL. */
    if (ctx == NULL) {
        return;
    }
    
    mbedtls_cipher_free(&ctx->cipher_dec);
    mbedtls_cipher_free(&ctx->cipher_enc);
    free(ctx);
}

/* Set AES CTR or IV for a context. */
void aes_setiv(aes_ctx_t *ctx, const void *iv, size_t l) {
    if (mbedtls_cipher_set_iv(&ctx->cipher_dec, iv, l)
        || mbedtls_cipher_set_iv(&ctx->cipher_enc, iv, l)) {
        FATAL_ERROR("Failed to set IV for AES context!");
    }
}

/* Calculate CMAC. */
void aes_calculate_cmac(void *dst, void *src, size_t size, const void *key) {
    mbedtls_cipher_context_t m_ctx;
    mbedtls_cipher_init(&m_ctx);
    if (mbedtls_cipher_setup(&m_ctx, mbedtls_cipher_info_from_type(MBEDTLS_CIPHER_AES_128_ECB))
        || mbedtls_cipher_cmac_starts(&m_ctx, key, 0x80)
        || mbedtls_cipher_cmac_update(&m_ctx, src, size)
        || mbedtls_cipher_cmac_finish(&m_ctx, dst)) {
            FATAL_ERROR("Failed to calculate CMAC!");
    }
}


/* Encrypt with context. */
void aes_encrypt(aes_ctx_t *ctx, void *dst, const void *src, size_t l) {
    size_t out_len = 0;
    
    /* Prepare context */
    mbedtls_cipher_reset(&ctx->cipher_enc);
    
    /* XTS doesn't need per-block updating */
    if (mbedtls_cipher_get_cipher_mode(&ctx->cipher_enc) == MBEDTLS_MODE_XTS)
        mbedtls_cipher_update(&ctx->cipher_enc, (const unsigned char * )src, l, (unsigned char *)dst, &out_len);
    else
    {
        unsigned int blk_size = mbedtls_cipher_get_block_size(&ctx->cipher_enc);
        
        /* Do per-block updating */
        for (int offset = 0; (unsigned int)offset < l; offset += blk_size)
        {
            int len = ((unsigned int)(l - offset) > blk_size) ? blk_size : (unsigned int) (l - offset);
            mbedtls_cipher_update(&ctx->cipher_enc, (const unsigned char * )src + offset, len, (unsigned char *)dst + offset, &out_len);
        }
    }
    
    /* Flush all data */
    mbedtls_cipher_finish(&ctx->cipher_enc, NULL, NULL);
}

/* Decrypt with context. */
void aes_decrypt(aes_ctx_t *ctx, void *dst, const void *src, size_t l) 
{
    bool src_equals_dst = false; 

    if (src == dst)
    {
        src_equals_dst = true;

        dst = malloc(l);
        if (dst == NULL) {
            fprintf(stderr, "Error: AES buffer allocation failure!\n");
            exit(EXIT_FAILURE);
        }
    }

    size_t out_len = 0;
    
    /* Prepare context */
    mbedtls_cipher_reset(&ctx->cipher_dec);
    
    /* XTS doesn't need per-block updating */
    if (mbedtls_cipher_get_cipher_mode(&ctx->cipher_dec) == MBEDTLS_MODE_XTS)
        mbedtls_cipher_update(&ctx->cipher_dec, (const unsigned char * )src, l, (unsigned char *)dst, &out_len);
    else
    {
        unsigned int blk_size = mbedtls_cipher_get_block_size(&ctx->cipher_dec);
        
        /* Do per-block updating */
        for (int offset = 0; (unsigned int)offset < l; offset += blk_size)
        {
            int len = ((unsigned int)(l - offset) > blk_size) ? blk_size : (unsigned int) (l - offset);
            mbedtls_cipher_update(&ctx->cipher_dec, (const unsigned char * )src + offset, len, (unsigned char *)dst + offset, &out_len);
        }
    }
    
    /* Flush all data */
    mbedtls_cipher_finish(&ctx->cipher_dec, NULL, NULL);

    if (src_equals_dst)
    {
        memcpy((void*)src, dst, l);
        free(dst);
    }
}


static void get_tweak(unsigned char *tweak, size_t sector) {
    for (int i = 0xF; i >= 0; i--) { /* Nintendo LE custom tweak... */
        tweak[i] = (unsigned char)(sector & 0xFF);
        sector >>= 8;
    }
}

/* Encrypt with context for XTS. */
void aes_xts_encrypt(aes_ctx_t *ctx, void *dst, const void *src, size_t l, size_t sector, size_t sector_size) {
    unsigned char tweak[0x10];

    if (l % sector_size != 0) {
        FATAL_ERROR("Length must be multiple of sectors!");
    }

    for (size_t i = 0; i < l; i += sector_size) {
        /* Workaround for Nintendo's custom sector...manually generate the tweak. */
        get_tweak(tweak, sector++);
        aes_setiv(ctx, tweak, 16);
        aes_encrypt(ctx, (char *)dst + i, (const char *)src + i, sector_size);
    }
}

/* Decrypt with context for XTS. */
void aes_xts_decrypt(aes_ctx_t *ctx, void *dst, const void *src, size_t l, size_t sector, size_t sector_size) {
    unsigned char tweak[0x10];

    if (l % sector_size != 0) {
        FATAL_ERROR("Length must be multiple of sectors!");
    }

    for (size_t i = 0; i < l; i += sector_size) {
        /* Workaround for Nintendo's custom sector...manually generate the tweak. */
        get_tweak(tweak, sector++);
        aes_setiv(ctx, tweak, 16);
        aes_decrypt(ctx, (char *)dst + i, (const char *)src + i, sector_size);
    }
}