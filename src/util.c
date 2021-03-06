/**
 *
 * Copyright (c) 2020, Jared Wolff
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this
 *    list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its
 *    contributors may be used to endorse or promote products derived from
 *    this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */

#include "util.h"

#include "ble.h"

#include "nrf_log.h"

// TODO: get this without SDH
void util_print_device_address(bool with_delim)
{

    ble_gap_addr_t gap_addr;
    sd_ble_gap_addr_get(&gap_addr);
    static char gap_addr_str[18];

    // Convert address to readable string
    if (with_delim)
    {
        addr_strhex_delim(gap_addr.addr, BLE_GAP_ADDR_LEN, gap_addr_str);
    }
    else
    {
        addr_strhex_no_delim(gap_addr.addr, BLE_GAP_ADDR_LEN, gap_addr_str);
    }

    NRF_LOG_INFO("Address: %s", gap_addr_str);
}

// TODO: simplify this if possible.
void util_get_device_address(char *addr)
{
    ble_gap_addr_t gap_addr;
    sd_ble_gap_addr_get(&gap_addr);

    // Convert address to readable string
    addr_strhex_delim(gap_addr.addr, BLE_GAP_ADDR_LEN, addr);
}

void addr_strhex_delim(uint8_t *addr, int size, char *out)
{
    static char hex_str[] = "0123456789abcdef";
    unsigned int i;

    int factor = 3;

    for (i = 0; i < size; i++)
    {
        (out)[i * factor + 0] = hex_str[(addr[i] >> 4) & 0x0F];
        (out)[i * factor + 1] = hex_str[(addr[i]) & 0x0F];
        (out)[i * factor + 2] = ':';
    }
    (out)[(i - 1) * factor + 2] = '\0';
}

void addr_strhex_no_delim(uint8_t *addr, int size, char *out)
{
    static char hex_str[] = "0123456789abcdef";
    unsigned int i;

    int factor = 2;

    for (i = 0; i < size; i++)
    {
        (out)[i * factor + 0] = hex_str[(addr[i] >> 4) & 0x0F];
        (out)[i * factor + 1] = hex_str[(addr[i]) & 0x0F];
    }
    (out)[(i - 1) * factor + 2] = '\0';
}