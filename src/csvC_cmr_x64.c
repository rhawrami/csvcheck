#include <stdint.h>
#include <x86intrin.h>


#include "csvC_job.h"
#include "csvC_cmr_shared.h"

// NOTE: the parsing algorithm below is essentially a copy of the 
// algorithm written in go assembly.
// link: https://github.com/rhawrami/vecsv/blob/main/internal/parser/scan.amd64.s
// And this algorithm is adopted from how simdjson parses JSON documents.
// link: https://github.com/simdjson/simdjson

_Bool compare_mask_reduce_x64_SSE2(FILE *f_out, parse_job *pj) {
    // comparison vectors
    __m128i v_delim = _mm_set1_epi8((char)pj->pj_delim_char);
    __m128i v_linefeed = _mm_set1_epi8((char)linefeed_char);
    __m128i v_quote = _mm_set1_epi8((char)quote_char);

    __m128i grp_0;
    __m128i grp_1;
    __m128i grp_2;
    __m128i grp_3;
    __m128i res_0;
    __m128i res_1;
    __m128i res_2;
    __m128i res_3;

    uint64_t m_linefeed_and_delim = 0;
    uint64_t m_quote = 0;

    uint64_t in_quote = (uint64_t)pj->pj_in_quote;

    for (size_t i = 0; i < pj->pj_buf_len; i += 64) {
        grp_0 = _mm_loadu_epi8(pj->pj_buf + i);
        grp_1 = _mm_loadu_epi8(pj->pj_buf + i + 16);
        grp_2 = _mm_loadu_epi8(pj->pj_buf + i + 32);
        grp_3 = _mm_loadu_epi8(pj->pj_buf + i + 48);

        // compare_mask_reduce DELIMITER and LINEFEED        
        res_0 = _mm_cmpeq_epi8(grp_0, v_delim);
        res_1 = _mm_cmpeq_epi8(grp_1, v_delim);
        res_2 = _mm_cmpeq_epi8(grp_0, v_linefeed);
        res_3 = _mm_cmpeq_epi8(grp_1, v_linefeed);
        res_0 = _mm_or_si128(res_0, res_1);
        res_1 = _mm_or_si128(res_2, res_3);
        res_0 = _mm_cmpeq_epi8(grp_2, v_delim);
        res_1 = _mm_cmpeq_epi8(grp_3, v_delim);
        res_2 = _mm_cmpeq_epi8(grp_2, v_linefeed);
        res_3 = _mm_cmpeq_epi8(grp_3, v_linefeed);
        res_2 = _mm_or_si128(res_0, res_1);
        res_3 = _mm_or_si128(res_2, res_3);
        m_linefeed_and_delim |= (uint64_t)_mm_movemask_epi8(res_0);
        m_linefeed_and_delim |= ((uint64_t)_mm_movemask_epi8(res_1) << 16);
        m_linefeed_and_delim |= ((uint64_t)_mm_movemask_epi8(res_2) << 32);
        m_linefeed_and_delim |= ((uint64_t)_mm_movemask_epi8(res_3) << 48);

        // compare_mask_reduce QUOTE
        res_0 = _mm_cmpeq_epi8(grp_0, v_quote);
        res_1 = _mm_cmpeq_epi8(grp_1, v_quote);
        res_2 = _mm_cmpeq_epi8(grp_2, v_quote);
        res_3 = _mm_cmpeq_epi8(grp_3, v_quote);
        m_quote |= (uint64_t)_mm_movemask_epi8(res_0);
        m_quote |= ((uint64_t)_mm_movemask_epi8(res_1) << 16);
        m_quote |= ((uint64_t)_mm_movemask_epi8(res_2) << 32);
        m_quote |= ((uint64_t)_mm_movemask_epi8(res_3) << 48);

        m_quote |= in_quote;
        m_quote = prefix_xor(m_quote);
        in_quote >>= 63;

        m_linefeed_and_delim &= (~m_quote);

        while (m_linefeed_and_delim > 0) {
            pj->pj_on_field += 1;
            uint64_t pos = _tzcnt_u64(m_linefeed_and_delim);
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
            
            m_linefeed_and_delim &= (UINT64_MAX << pos); 
        }

    }

    pj->pj_in_quote = in_quote;

    return true;
}

void compare_mask_reduce_x64_AVX2(FILE *f_out, parse_job *pj) {

}

void compare_mask_reduce_x64_AVX512(FILE *f_out, parse_job *pj) {
    
}