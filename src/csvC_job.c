#include <string.h>
#include <assert.h>

#include "csvC_job.h"

_Bool pj_fread(FILE *f, parse_job *pj) {
    // rm after testing
    assert(pj->pj_buf_cap / 64 == 0);

    size_t bytes_read = fread(pj->pj_buf, 1, pj->pj_buf_cap, f);
    if (bytes_read < pj->pj_buf_cap) {
        memset(
            pj->pj_buf + pj->pj_buf_len, 
            0, 
            // clear bytes in order to safely pass to fns parsing 64B chunks
            (pj->pj_buf_len + 63) / 64
        ); 
    }
    pj->pj_buf_len = bytes_read;
    if (feof(f)) {
        return true;
    }
    if (ferror(f)) {
        return false;
    }
    return true;
}

void pj_new_job(const char *csv_name, parse_job *pj) {
    pj->pj_name = csv_name;
    pj->pj_buf_len = 0;
    pj->pj_in_quote = false;
    pj->pj_n_fields = 0;
    pj->pj_n_rows = 0;
}

_Bool pj_report_issue(FILE *out, parse_job *pj, pj_issue issue) {
    int ret_val;
    switch (issue) {
        case pji_FIELDCNT:
            ret_val = fprintf(
                out, 
                "%s: Row %llu: field count != %llu\n", 
                pj->pj_name, 
                pj->pj_n_rows, 
                pj->pj_n_fields
            );
            break;
        case pji_NEEDEDCR:
            ret_val = fprintf(
                out,
                "%s: Row %llu: missing carriage-return (\\r)",
                pj->pj_name, 
                pj->pj_n_rows 
            );
            break;
        case pji_USEDCR:
            ret_val = fprintf(
                out,
                "%s: Row %llu: used carriage-return (\\r)",
                pj->pj_name, 
                pj->pj_n_rows 
            );
            break;
        case pji_ESCQUOTE:
            ret_val = fprintf(
                out,
                "%s: Row %llu: unescaped quote",
                pj->pj_name, 
                pj->pj_n_rows 
            );
            break;
    }

    return (_Bool)(ret_val >= 0);
}
