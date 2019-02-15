/*
 * Copyright (c) 2018 shchmue
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms and conditions of the GNU General Public License,
 * version 2, as published by the Free Software Foundation.
 *
 * This program is distributed in the hope it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

/*
    Shared font code adapted from
    https://github.com/switchbrew/switch-examples/tree/master/graphics/shared_font
*/

#include "Common.hpp"
#include "Key.hpp"

#include <machine/endian.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <filesystem>
#include <string>

#include <switch.h>

#include "sha256.h"

#ifdef RGBX8
    #define LIBNX_200
#endif

namespace Common {

    void sha256(const u8 *data, u8 *hash, size_t length) {
        struct sha256_state ctx;
        sha256_init(&ctx);
        sha256_update(&ctx, data, length);
        sha256_finalize(&ctx);
        sha256_finish(&ctx, hash);
    }

    byte_vector key_string_to_byte_vector(std::string key_string) {
        key_string = key_string.substr(key_string.find('=') + 2);
        byte_vector temp_key((key_string.size() - 1) / 2);
        for (size_t i = 0; i < temp_key.size() - 1; i += 8)
            *reinterpret_cast<u64 *>(temp_key.data() + i) = __bswap64(strtoul(key_string.substr(i * 2, 0x10).c_str(), NULL, 16));
        return temp_key;
    }
}