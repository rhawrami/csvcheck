#include <sys/stat.h>
#include <stdlib.h>

#include "unity.h"
#include "csvC_job.h"

void setUp(void) {}
void tearDown(void) {}

// test csvC_pj_fread() when the file size is larger than
// the buffer size (i.e., multiple reads are necessary)
void test_csvC_pj_fread_BufferIsSmallerThanFile(void) {
    size_t buf_size = 1024;
    const char *td_file_name = "td_CRLF_comma_1000_10.csv";
    char td_file_path[128];
    
#ifdef TESTDATA_DIR
    snprintf(td_file_path, sizeof(td_file_path), "%s/%s", TESTDATA_DIR, td_file_name);
#endif

    struct stat st;
    size_t f_size;
    uint8_t *buf;
    FILE *td_file; 
    
    td_file = fopen(td_file_path, "r");
    if (td_file == NULL) {
        TEST_FAIL_MESSAGE("unable to open file");
    }
    
    if (stat(td_file_path, &st)) {
        TEST_FAIL_MESSAGE("unable to get file size");
    }
    f_size = (size_t)st.st_size;
    if (f_size <= buf_size) {
        TEST_FAIL_MESSAGE("file size is not larger than buffer size");
    }

    buf = malloc(buf_size * sizeof(uint8_t));
    if (buf == NULL) {
        TEST_FAIL_MESSAGE("unable to malloc initial buffer");
    }

    parse_job pj = {
        .pj_name = td_file_name,
        .pj_buf = buf,
        .pj_buf_cap = buf_size,
        .pj_buf_len = 0,
    };

    size_t n_full_reads = f_size / buf_size;

    // for safe reads, no errors should be returned, and
    // EOF should not have been hit yet
    for (size_t i = 0; i < n_full_reads; i++) {
        _Bool ret_val = pj_fread(td_file, &pj);
        TEST_ASSERT(ret_val);
        TEST_ASSERT_EQUAL_size_t(buf_size, pj.pj_buf_len);
        TEST_ASSERT(!pj_hit_EOF(&pj));
    }

    // assuming there's a remainder for a final read
    if ((f_size % (buf_size)) != 0) {
        _Bool ret_val = pj_fread(td_file, &pj);

        TEST_ASSERT(ret_val);
        TEST_ASSERT(pj_hit_EOF(&pj));
        TEST_ASSERT_EQUAL_size_t(f_size - (buf_size * n_full_reads), pj.pj_buf_len);

        // if length not div by 64, then the remaining 64 bytes in a chunk should be cleared
        if ((pj.pj_buf_len & 63) != 0) {
            for (size_t i = pj.pj_buf_len; i < ((pj.pj_buf_len + 63) / 64 * 64); i++) {
                TEST_ASSERT_EQUAL_UINT8(0, pj.pj_buf[i]);
            }
        }
    }

    free(buf);
}

int main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_csvC_pj_fread_BufferIsSmallerThanFile);
    return UNITY_END();
}
