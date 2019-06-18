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

#include "KeyLocation.hpp"

#include "Common.hpp"
#include "xxhash64.h"

#include <algorithm>
#include <unordered_map>

#include <switch.h>

void KeyLocation::get_from_memory(u64 tid, u8 seg_mask) {
    Handle debug_handle = INVALID_HANDLE;
    u64 d[8];

    // if not a kernel process, get pid from pm:dmnt
    if ((tid > 0x0100000000000005) && (tid != 0x0100000000000028)) {
        u64 pid;
        pmdmntGetTitlePid(&pid, tid);

        if (R_FAILED(svcDebugActiveProcess(&debug_handle, pid)) ||
            R_FAILED(svcGetDebugEvent(reinterpret_cast<u8 *>(&d), debug_handle)))
        {
            return;
        }
    } else { // otherwise query svc for the process list
        u64 pids[300];
        u32 num_processes;

        svcGetProcessList(&num_processes, pids, 300);
        u32 i;
        for (i = 0; i < num_processes - 1; i++) {
            if (R_SUCCEEDED(svcDebugActiveProcess(&debug_handle, pids[i])) &&
                R_SUCCEEDED(svcGetDebugEvent(reinterpret_cast<u8 *>(&d), debug_handle)) &&
                (d[2] == tid))
            {
                break;
            }
            if (debug_handle) svcCloseHandle(debug_handle);
        }
        if (i == num_processes - 1) {
            if (debug_handle) svcCloseHandle(debug_handle);
            return;
        }
    }

    MemoryInfo mem_info = {};

    u32 page_info;
    u64 addr = 0;
	
    u64 last_text_addr = 0;
	
	// locate "real" .text segment as Atmosphere emuNAND has two
    for (;;) {
        svcQueryDebugProcessMemory(&mem_info, &page_info, debug_handle, addr);
        if  ((mem_info.perm & Perm_X) &&
            ((mem_info.type & 0xff) >= MemType_CodeStatic) &&
            ((mem_info.type & 0xff) < MemType_Heap))
        {
            last_text_addr = mem_info.addr;
        }
        addr = mem_info.addr + mem_info.size;
        if (addr == 0) break;
    }

	addr = last_text_addr;
    for (u8 segment = 1; segment < BIT(3); )
    {
        svcQueryDebugProcessMemory(&mem_info, &page_info, debug_handle, addr);
        // weird code to allow for bitmasking segments
        if  ((mem_info.perm & Perm_R) &&
            ((mem_info.type & 0xff) >= MemType_CodeStatic) &&
            ((mem_info.type & 0xff) < MemType_Heap) &&
            ((segment <<= 1) >> 1 & seg_mask) > 0)
        {
            data.resize(data.size() + mem_info.size);
            if(R_FAILED(svcReadDebugProcessMemory(data.data() + data.size() - mem_info.size, debug_handle, mem_info.addr, mem_info.size))) {
                if (debug_handle) svcCloseHandle(debug_handle);
                return;
            }
        }
        addr = mem_info.addr + mem_info.size;
        if (addr == 0) break;
    }
    
    svcCloseHandle(debug_handle);
}

void KeyLocation::get_keyblobs() {
    FsStorage boot0;
    fsOpenBisStorage(&boot0, 0);
    data.resize(0x200 * KNOWN_KEYBLOBS);
    fsStorageRead(&boot0, KEYBLOB_OFFSET, data.data(), data.size());
    fsStorageClose(&boot0);
}

void KeyLocation::find_keys(std::vector<Key *> &keys) {
    if (data.size() == 0)
        return;

    u8 temp_hash[0x20];
    size_t key_indices_left = keys.size();
    u64 hash = 0;
    std::unordered_map<u64, size_t> hash_index;
    for (size_t i = 0; i < keys.size(); i++)
        hash_index[keys[i]->xx_hash] = i;

    // hash every length-sized byte chunk in data until it matches a key hash
    for (size_t i = 0; i < data.size() - 0x10; i++) {
        hash = XXHash64::hash(data.data() + i, 0x10, 0);
        auto search = hash_index.find(hash);
        if (search == hash_index.end()) {
            continue;
        }
        size_t key_index = hash_index[hash];
        u8 key_length = keys[key_index]->length;
        // double-check sha256 since xxhash64 isn't as collision-safe
        Common::sha256(data.data() + i, temp_hash, key_length);
        if (!std::equal(keys[key_index]->hash.begin(), keys[key_index]->hash.end(), temp_hash))
            continue;
        std::copy(data.begin() + i, data.begin() + i + key_length, std::back_inserter(keys[key_index]->key));
        keys[key_index]->is_found = true;
        key_indices_left--;
        if (key_indices_left == 0)
            return;
        hash_index.erase(hash);
        i += key_length - 1;
    }
}