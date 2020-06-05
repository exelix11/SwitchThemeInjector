#include <string.h>
#include <stdlib.h>
#include "rsa.h"
#include "sha.h"
#include "utils.h"
#include "types.h"

#define RSA_2048_BYTES 0x100
#define RSA_2048_BITS (RSA_2048_BYTES*8)

static void calculate_mgf1_and_xor(unsigned char *data, size_t data_size, const void *h_src, size_t h_src_size) {
    unsigned char h_buf[RSA_2048_BYTES] = {0};
    memcpy(h_buf, h_src, h_src_size);

    unsigned char mgf1_buf[0x20];
    size_t ofs = 0;
    unsigned int seed = 0;
    while (ofs < data_size) {
        for (unsigned int i = 0; i < sizeof(seed); i++) {
            h_buf[h_src_size + 3 - i] = (seed >> (8 * i)) & 0xFF;
        }
        sha256_hash_buffer(mgf1_buf, h_buf, h_src_size + 4);
        for (unsigned int i = ofs; i < data_size && i < ofs + 0x20; i++) {
            data[i] ^= mgf1_buf[i - ofs];
        }
        seed++;
        ofs += 0x20;
    }
}

/* Perform an RSA-PSS verify operation on data, with signature and N. */
int rsa2048_pss_verify(const void *data, size_t len, const unsigned char *signature, const unsigned char *modulus) {
    mbedtls_mpi signature_mpi;
    mbedtls_mpi modulus_mpi;
    mbedtls_mpi e_mpi;
    mbedtls_mpi message_mpi;

    mbedtls_mpi_init(&signature_mpi);
    mbedtls_mpi_init(&modulus_mpi);
    mbedtls_mpi_init(&e_mpi);
    mbedtls_mpi_init(&message_mpi);
    mbedtls_mpi_lset(&message_mpi, RSA_2048_BITS);

    unsigned char m_buf[RSA_2048_BYTES];
    unsigned char h_buf[0x24];
    const unsigned char E[3] = {1, 0, 1};

    mbedtls_mpi_read_binary(&e_mpi, E, 3);
    mbedtls_mpi_read_binary(&signature_mpi, signature, RSA_2048_BYTES);
    mbedtls_mpi_read_binary(&modulus_mpi, modulus, RSA_2048_BYTES);
    mbedtls_mpi_exp_mod(&message_mpi, &signature_mpi, &e_mpi, &modulus_mpi, NULL);

    if (mbedtls_mpi_write_binary(&message_mpi, m_buf, RSA_2048_BYTES) != 0) {
        FATAL_ERROR("Failed to export exponentiated RSA message!");
    }

    mbedtls_mpi_free(&signature_mpi);
    mbedtls_mpi_free(&modulus_mpi);
    mbedtls_mpi_free(&e_mpi);
    mbedtls_mpi_free(&message_mpi);

    /* There's no automated PSS verification as far as I can tell. */
    if (m_buf[RSA_2048_BYTES-1] != 0xBC) {
        return false;
    }

    memset(h_buf, 0, 0x24);
    memcpy(h_buf, m_buf + RSA_2048_BYTES - 0x20 - 0x1, 0x20);

    /* Decrypt maskedDB. */
    calculate_mgf1_and_xor(m_buf, RSA_2048_BYTES - 0x20 - 1, h_buf, 0x20);

    m_buf[0] &= 0x7F; /* Constant lmask for rsa-2048-pss. */

    /* Validate DB. */
    for (unsigned int i = 0; i < RSA_2048_BYTES - 0x20 - 0x20 - 1 - 1; i++) {
        if (m_buf[i] != 0) {
            return false;
        }
    }
    if (m_buf[RSA_2048_BYTES - 0x20 - 0x20 - 1 - 1] != 1) {
        return false;
    }

    /* Check hash correctness. */
    unsigned char validate_buf[8 + 0x20 + 0x20];
    unsigned char validate_hash[0x20];
    memset(validate_buf, 0, 0x48);
    sha256_hash_buffer(&validate_buf[8], data, len);
    memcpy(&validate_buf[0x28], &m_buf[RSA_2048_BYTES - 0x20 - 0x20 - 1], 0x20);
    sha256_hash_buffer(validate_hash, validate_buf, 0x48);
    return memcmp(h_buf, validate_hash, 0x20) == 0;
}

/* Perform an RSA-PKCS1 verify operation on data, with signature and N. */
int rsa2048_pkcs1_verify(const void *data, size_t len, const unsigned char *signature, const unsigned char *modulus) {
    mbedtls_mpi signature_mpi;
    mbedtls_mpi modulus_mpi;
    mbedtls_mpi e_mpi;
    mbedtls_mpi message_mpi;

    mbedtls_mpi_init(&signature_mpi);
    mbedtls_mpi_init(&modulus_mpi);
    mbedtls_mpi_init(&e_mpi);
    mbedtls_mpi_init(&message_mpi);
    mbedtls_mpi_lset(&message_mpi, RSA_2048_BITS);

    unsigned char m_buf[RSA_2048_BYTES];
    unsigned char h_buf[0x20];
    const unsigned char E[3] = {1, 0, 1};

    mbedtls_mpi_read_binary(&e_mpi, E, 3);
    mbedtls_mpi_read_binary(&signature_mpi, signature, RSA_2048_BYTES);
    mbedtls_mpi_read_binary(&modulus_mpi, modulus, RSA_2048_BYTES);
    mbedtls_mpi_exp_mod(&message_mpi, &signature_mpi, &e_mpi, &modulus_mpi, NULL);

    if (mbedtls_mpi_write_binary(&message_mpi, m_buf, RSA_2048_BYTES) != 0) {
        FATAL_ERROR("Failed to export exponentiated RSA message!");
    }

    mbedtls_mpi_free(&signature_mpi);
    mbedtls_mpi_free(&modulus_mpi);
    mbedtls_mpi_free(&e_mpi);
    mbedtls_mpi_free(&message_mpi);

    /* For RSA-2048, this prefix is just a constant. */
    const unsigned char pkcs1_hash_prefix[0xE0] = {
        0x00, 0x01, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
        0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
        0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
        0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
        0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
        0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
        0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
        0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
        0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
        0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
        0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
        0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
        0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x30, 0x31, 0x30,
        0x0D, 0x06, 0x09, 0x60, 0x86, 0x48, 0x01, 0x65, 0x03, 0x04, 0x02, 0x01, 0x05, 0x00, 0x04, 0x20
    };

    sha256_hash_buffer(h_buf, data, len);

    return memcmp(pkcs1_hash_prefix, m_buf, 0xE0) == 0 && memcmp(&m_buf[0xE0], h_buf, 0x20) == 0;
}

/* Perform an RSA-OAEP decryption (and verification) on data, with Signature and N. */
int rsa2048_oaep_decrypt_verify(void *out, size_t max_out_len, const unsigned char *signature, const unsigned char *modulus, const unsigned char *exponent, size_t exponent_len, const unsigned char *label_hash, size_t *out_len) {
    mbedtls_mpi signature_mpi;
    mbedtls_mpi modulus_mpi;
    mbedtls_mpi exp_mpi;
    mbedtls_mpi message_mpi;

    mbedtls_mpi_init(&signature_mpi);
    mbedtls_mpi_init(&modulus_mpi);
    mbedtls_mpi_init(&exp_mpi);
    mbedtls_mpi_init(&message_mpi);
    mbedtls_mpi_lset(&message_mpi, RSA_2048_BITS);

    unsigned char m_buf[RSA_2048_BYTES];

    mbedtls_mpi_read_binary(&exp_mpi, exponent, exponent_len);
    mbedtls_mpi_read_binary(&signature_mpi, signature, RSA_2048_BYTES);
    mbedtls_mpi_read_binary(&modulus_mpi, modulus, RSA_2048_BYTES);
    mbedtls_mpi_exp_mod(&message_mpi, &signature_mpi, &exp_mpi, &modulus_mpi, NULL);

    if (mbedtls_mpi_write_binary(&message_mpi, m_buf, RSA_2048_BYTES) != 0) {
        FATAL_ERROR("Failed to export exponentiated RSA message!");
    }

    mbedtls_mpi_free(&signature_mpi);
    mbedtls_mpi_free(&modulus_mpi);
    mbedtls_mpi_free(&exp_mpi);
    mbedtls_mpi_free(&message_mpi);

    /* There's no automated PSS verification as far as I can tell. */
    if (m_buf[0] != 0x00) {
        return false;
    }

    /* Unmask salt. */
    calculate_mgf1_and_xor(m_buf + 1, 0x20, m_buf + 0x21, RSA_2048_BYTES - 0x20 - 1);
    /* Unmask DB. */
    calculate_mgf1_and_xor(m_buf + 0x21, RSA_2048_BYTES - 0x20 - 1, m_buf + 1, 0x20);

    /* Validate label hash */
    const unsigned char *db = m_buf + 0x21;
    if (memcmp(db, label_hash, 0x20) != 0) {
        return false;
    }

    /* Validate message prefix. */
    const unsigned char *data = db + 0x20;
    size_t remaining = RSA_2048_BYTES - 0x20 - 1 - 0x20;
    while (*data == 0 && remaining) {
        data++;
        remaining--;
    }
    if (remaining == 0 || *data++ != 1) {
        return false;
    }
    remaining--;
    *out_len = remaining;
    if (remaining > max_out_len) {
        remaining = max_out_len;
    }
    memcpy(out, data, remaining);
    return true;
}
