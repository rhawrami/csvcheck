#include <string.h>
#include <assert.h>

#include "csvC_job.h"

_Bool pj_fread(FILE *f, parse_job *pj) {
    // rm after testing
    assert(pj->pj_buf_cap & 63 == 0);

    size_t bytes_read = fread(pj->pj_buf, 1, pj->pj_buf_cap, f);
    if (bytes_read < pj->pj_buf_cap) {
        memset(
            pj->pj_buf + bytes_read, 
            0, 
            // clear bytes in order to safely pass to fns parsing 64B chunks
            (bytes_read + 63) / 64 * 64
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
    pj->pj_on_row = 0;
    pj->pj_on_field = 0;
}

_Bool pj_report_issue(FILE *out, parse_job *pj, pj_issue issue) {
    const char *fmt_msg;

    switch (issue) {
        case pji_FIELDCNT:
            fmt_msg = "%s: Row %llu: field count != %llu\n";
            break;
        case pji_NEEDEDCR:
            fmt_msg = "%s: Row %llu: missing carriage-return (\\r)\n";
            break;
        case pji_USEDCR:
            fmt_msg = "%s: Row %llu: used carriage-return (\\r)\n";
            break;
        case pji_ESCQUOTE:
            fmt_msg = "%s: Row %llu: unescaped quote\n";
            break;
        default:
            fmt_msg = "DEFAULT\n";
    }

    int ret_val = fprintf(
        out,
        fmt_msg,
        pj->pj_name, 
        pj->pj_on_row, 
        pj->pj_n_fields
    );


    return (_Bool)(ret_val >= 0);
}
