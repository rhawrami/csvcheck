#include <stdint.h>
#include <arm_neon.h>
#include <arm_acle.h>

#include "csvC_job.h"

// NOTE: the parsing algorithm below is essentially a copy of the 
// algorithm written in go assembly.
// link: https://github.com/rhawrami/vecsv/blob/main/internal/parser/scan.arm64.s
// And this algorithm is adopted from how simdjson parses JSON documents.
// link: https://github.com/simdjson/simdjson

static const uint8_t quote_char = '"';
static const uint8_t linefeed_char = '\n';
static const uint8_t carriagereturn_char = '\r';

static inline uint64_t prefix_xor_arm64(uint64_t x) {
    x = __rbitll(x);
    uint64_t scratch = x >> 1;
    x ^= scratch;
    scratch = x >> 2;
    x ^= scratch;
    scratch = x >> 4;
    x ^= scratch;
    scratch = x >> 8;
    x ^= scratch;
    scratch = x >> 16;
    x ^= scratch;
    scratch = x >> 32;
    x ^= scratch;

    return x;
}

_Bool compare_mask_reduce_arm64(FILE *f_out, parse_job *pj) {
    // comparison vectors
    uint8x16_t v_delim = vdupq_n_u8(pj->pj_delim_char);
    uint8x16_t v_linefeed = vdupq_n_u8(linefeed_char);
    uint8x16_t v_quote = vdupq_n_u8(quote_char);

    // reduction masks
    uint8x16_t m_0 = (uint8x16_t)vcombine_u64(
        vcreate_u64(0x1001100110011001ULL),
        vcreate_u64(0x1001100110011001ULL)
    );
    uint8x16_t m_1 = (uint8x16_t)vcombine_u64(
        vcreate_u64(0x2002200220022002ULL),
        vcreate_u64(0x2002200220022002ULL)
    );
    uint8x16_t m_2 = (uint8x16_t)vcombine_u64(
        vcreate_u64(0x4004400440044004ULL),
        vcreate_u64(0x4004400440044004ULL)
    );
    uint8x16_t m_3 = (uint8x16_t)vcombine_u64(
        vcreate_u64(0x8008800880088008ULL),
        vcreate_u64(0x8008800880088008ULL)
    );

    // intermittent results
    uint8x16_t res_0;
    uint8x16_t res_1;
    uint8x16_t res_2;
    uint8x16_t res_3;
    // 64B chunk
    uint8x16x4_t batch;
    // result masks
    uint64_t m_linefeed_and_delim;
    uint64_t m_quote;
    // within quote
    uint64_t in_quote = (uint64_t)pj->pj_in_quote;

    // reading over the length is safe as we guarantee that
    // the buffer is divisible by 64, and that the remaining
    // bytes are cleared
    for (size_t i = 0; i < pj->pj_buf_len; i += 64) {
        // load using `ld4` in order to use mask reduction
        batch = vld4q_u8(pj->pj_buf + i);

        // compare_mask_reduce DELIMITER and LINEFEED
        res_0 = vceqq_u8(batch.val[0], v_delim);
        res_1 = vceqq_u8(batch.val[1], v_delim);
        res_2 = vceqq_u8(batch.val[0], v_linefeed);
        res_3 = vceqq_u8(batch.val[1], v_linefeed);
        res_0 = vorrq_u8(res_0, res_2);
        res_1 = vorrq_u8(res_1, res_3);
        res_0 = vceqq_u8(batch.val[2], v_delim);
        res_1 = vceqq_u8(batch.val[3], v_delim);
        res_2 = vceqq_u8(batch.val[2], v_linefeed);
        res_3 = vceqq_u8(batch.val[3], v_linefeed);
        res_2 = vorrq_u8(res_0, res_2);
        res_3 = vorrq_u8(res_1, res_3);
        res_0 = vandq_u8(res_0, m_0);
        res_1 = vandq_u8(res_1, m_1);
        res_2 = vandq_u8(res_2, m_2);
        res_3 = vandq_u8(res_3, m_3);
        res_0 = (uint8x16_t)vaddq_u64((uint64x2_t)res_0, (uint64x2_t)res_1);
        res_2 = (uint8x16_t)vaddq_u64((uint64x2_t)res_2, (uint64x2_t)res_3);
        res_0 = (uint8x16_t)vaddq_u64((uint64x2_t)res_0, (uint64x2_t)res_2);
        res_0 = vpaddq_u8(res_0, res_0);
        m_linefeed_and_delim = vgetq_lane_u64((uint64x2_t)res_0, 0);

        // compare_mask_reduce QUOTE
        res_0 = vceqq_u8(batch.val[0], v_quote);
        res_1 = vceqq_u8(batch.val[1], v_quote);
        res_2 = vceqq_u8(batch.val[2], v_quote);
        res_3 = vceqq_u8(batch.val[3], v_quote);
        res_0 = vandq_u8(res_0, m_0);
        res_1 = vandq_u8(res_1, m_1);
        res_2 = vandq_u8(res_2, m_2);
        res_3 = vandq_u8(res_3, m_3);
        res_0 = (uint8x16_t)vaddq_u64((uint64x2_t)res_0, (uint64x2_t)res_1);
        res_2 = (uint8x16_t)vaddq_u64((uint64x2_t)res_2, (uint64x2_t)res_3);
        res_0 = (uint8x16_t)vaddq_u64((uint64x2_t)res_0, (uint64x2_t)res_2);
        res_0 = vpaddq_u8(res_0, res_0);
        m_quote = vgetq_lane_u64((uint64x2_t)res_0, 0);

        m_quote |= in_quote;
        m_quote = prefix_xor_arm_v8(m_quote);
        in_quote >>= 63;

        m_linefeed_and_delim &= (~m_quote);

        // arm64 doesn't have trailing zeroes instruction, so reverse and 
        // use leading zeroes
        m_linefeed_and_delim = __rbitll(m_linefeed_and_delim);

        while (m_linefeed_and_delim > 0) {
            pj->pj_on_field += 1;
            uint64_t pos = __clzll(m_linefeed_and_delim);
            uint8_t found_char = pj->pj_buf[(size_t)pos+i];

            if (found_char == linefeed_char) {
                if (pj->pj_on_field != pj->pj_n_fields) {
                    if (!pj_report_issue(f_out, pj, pji_FIELDCNT)) {
                        return false;
                    }
                }

                if (
                    ((size_t)pos+i > 0) &&
                    (   
                        // missing CR, and need CR
                        ((pj->pj_buf[(size_t)pos+i-1] ^ carriagereturn_char) && (uint8_t)(pj->pj_use_cr)) ||
                        // have CR, but don't need CR
                        !((pj->pj_buf[(size_t)pos+i-1] ^ carriagereturn_char) && !(uint8_t)(pj->pj_use_cr))
                    )
                ) {
                    // places correct issue type for both scenarios
                    if (!pj_report_issue(f_out, pj, pji_USEDCR - (pj_issue)pj->pj_use_cr)) {
                        return false;
                    }
                }

                pj->pj_on_row += 1;
                pj->pj_on_field = 0;
            } 
            
            m_linefeed_and_delim &= (UINT64_MAX >> pos); 
        }
    }

    pj->pj_in_quote = in_quote;

    return true;
}