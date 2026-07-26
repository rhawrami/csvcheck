import random

QUOTE_CHAR = b'"'
LINEFEED_CHAR = b'\n'
CARRIAGERETURNLINEFEED_CHAR = b'\r\n'
COMMA_CHAR = b','
TAB_CHAR = b'\t'
COLON_CHAR = b':'


def gen_rand_letter_string(quotes: int) -> bytes:
    n_chars = random.randint(0, 15)
    if not n_chars:
        return b''
    b_string = b''
    if not quotes:
        b_string += b'"'
    for _ in range(n_chars):
        if random.randint(0, 10) & 1 == 0:
            b_string += bytes([random.randrange(ord('A'), ord('Z') + 1)])
        else:
            b_string += bytes([random.randrange(ord('a'), ord('z') + 1)])
    if not quotes:
            b_string += b'"'
    return b_string


def gen_rand_integer_string(ignore: int) -> bytes:
    b_string = b''
    for _ in range(15):
        b_string += bytes([random.randrange(ord('0'), ord('9') + 1)])
    return b_string


def gen_dataset(file_name: str, 
                sep_char: bytes, 
                newline_chars: bytes, 
                n_rows: int,
                n_fields: int):
    with open(file_name, 'wb') as f:
        fns = dict()
        for i in range(n_fields):
            fns[i] = gen_rand_integer_string if random.randint(0, 10) & 1 == 0 else gen_rand_letter_string
        for _ in range(n_rows):
            b_string = b''
            for field in range(n_fields):
                # field entries of at most 15 chars
                b_string += fns[field](random.randint(0, 10) & 3)
                if field != (n_fields - 1):
                    b_string += sep_char
            b_string += newline_chars
            f.write(b_string)  


def main():
    # file names will follow the format:
    #   td_{separator}_{newline}_{number of rows}_{number of fields per row}.csv

    # crlf | commas | 1000 rows | 10 fields
    gen_dataset(
        'td_CRLF_comma_1000_10.csv',
        COMMA_CHAR,
        CARRIAGERETURNLINEFEED_CHAR,
        1_000,
        10
    )

    # lf | tabs | 10_000 rows | 8 fields
    gen_dataset(
        'td_LF_tab_10000_8.csv',
        TAB_CHAR,
        LINEFEED_CHAR,
        10_000,
        8
    )

    # lf | colon | 50_000 rows | 13 fields
    gen_dataset(
        'td_lf_colon_50000_13.csv',
        COLON_CHAR,
        LINEFEED_CHAR,
        50_000,
        13
    )

    return

if __name__ == '__main__':
    main()