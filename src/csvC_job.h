#ifndef CSVCHECK_STATE
#define CSVCHECK_STATE 777

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <stdio.h>

// pj_issue defines what type of issue was found when parsing a CSV file.
typedef enum pj_issue pj_issue;

enum pj_issue {
    pji_FIELDCNT, // Field count unequal to first row field count.
    pji_NEEDEDCR, // Used LF when needed CRLF.
    pji_USEDCR,   // Used CRLF when needed only LF.
    pji_ESCQUOTE  // Needed quote escape.
};

// parse_job defines what is needed to parse and check a CSV file.
typedef struct parse_job parse_job;

struct parse_job {
    const char *pj_name;       // CSV name.
    uint8_t    *pj_buf;        // Data buffer.
    size_t      pj_buf_len;    // Data buffer length.
    size_t      pj_buf_cap;    // Data buffer capacity.
    size_t      pj_n_fields;   // Number of fields in first row.
    size_t      pj_n_rows;     // Numbers of rows read thus far.
    uint8_t     pj_delim_char; // Delimiter character.
    _Bool       pj_in_quote;   // Currently within quote (1 if yes).
    _Bool       pj_use_cr;     // Use carriage return for new lines.
};

// pj_fread reads from file `f`, and updates the new buffer length; returns
// false if an error is encountered; returns true otherwise; reaching EOF can
// be determined if true is returned, and `pj_buf_len` is set to 0.
_Bool pj_fread(FILE *f, parse_job *pj);

// pj_new_job clears a parsing job's state in order for a new file to 
// be able to be parsed.
void pj_new_job(const char *csv_name, parse_job *pj);

// pj_report_issue writes an issue to the file stream, returning false if
// unable to write to the stream.
_Bool pj_report_issue(FILE *out, parse_job *pj, pj_issue issue);

#endif
