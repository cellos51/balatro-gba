#include <util.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>


void test_truncate_uint_to_suffixed_str()
{
    /*
     * I want to avoid testing the rounding so it can be easily changed
     * so all tests are numbers that are rounded down regardless of rounding method.
     * That way the function can be modified to round to nearest integer easily.
     */

    char suffixed_str_buff[UINT_MAX_DIGITS + 1] = {'\0'};

    truncate_uint_to_suffixed_str(100, 3, suffixed_str_buff);
    assert(strcmp(suffixed_str_buff, "100") == 0);

    suffixed_str_buff[0] = '\0';
    truncate_uint_to_suffixed_str(1000, 3, suffixed_str_buff);
    assert(strcmp(suffixed_str_buff, "1K") == 0);
    
    suffixed_str_buff[0] = '\0';
    truncate_uint_to_suffixed_str(1000, 2, suffixed_str_buff);
    assert(strcmp(suffixed_str_buff, "1K") == 0);

    suffixed_str_buff[0] = '\0';
    truncate_uint_to_suffixed_str(1000, 4, suffixed_str_buff);
    assert(strcmp(suffixed_str_buff, "1000") == 0);

    suffixed_str_buff[0] = '\0';
    truncate_uint_to_suffixed_str(1000, 5, suffixed_str_buff);
    assert(strcmp(suffixed_str_buff, "1000") == 0);

    suffixed_str_buff[0] = '\0';
    truncate_uint_to_suffixed_str(12123, 4, suffixed_str_buff);
    assert(strcmp(suffixed_str_buff, "12K") == 0);

    suffixed_str_buff[0] = '\0';
    truncate_uint_to_suffixed_str(123123, 4, suffixed_str_buff);
    assert(strcmp(suffixed_str_buff, "123K") == 0);

    suffixed_str_buff[0] = '\0';
    truncate_uint_to_suffixed_str(123123, 5, suffixed_str_buff);
    assert(strcmp(suffixed_str_buff, "123K") == 0);

    suffixed_str_buff[0] = '\0';
    truncate_uint_to_suffixed_str(123123, 6, suffixed_str_buff);
    assert(strcmp(suffixed_str_buff, "123123") == 0);

    suffixed_str_buff[0] = '\0';
    truncate_uint_to_suffixed_str(123123, 7, suffixed_str_buff);
    assert(strcmp(suffixed_str_buff, "123123") == 0);

    suffixed_str_buff[0] = '\0';
    truncate_uint_to_suffixed_str(12345123, 6, suffixed_str_buff);
    assert(strcmp(suffixed_str_buff, "12345K") == 0);

    suffixed_str_buff[0] = '\0';
    truncate_uint_to_suffixed_str(12123123, 5, suffixed_str_buff);
    assert(strcmp(suffixed_str_buff, "12M") == 0);

    suffixed_str_buff[0] = '\0';
    truncate_uint_to_suffixed_str(12123123, 4, suffixed_str_buff);
    assert(strcmp(suffixed_str_buff, "12M") == 0);

    suffixed_str_buff[0] = '\0';
    truncate_uint_to_suffixed_str(54123123, 4, suffixed_str_buff);
    assert(strcmp(suffixed_str_buff, "54M") == 0);

    suffixed_str_buff[0] = '\0';
    truncate_uint_to_suffixed_str(123123123, 4, suffixed_str_buff);
    assert(strcmp(suffixed_str_buff, "123M") == 0);

    suffixed_str_buff[0] = '\0';
    truncate_uint_to_suffixed_str(123123123, 6, suffixed_str_buff);
    assert(strcmp(suffixed_str_buff, "123M") == 0);

    suffixed_str_buff[0] = '\0';
    truncate_uint_to_suffixed_str(987123123, 6, suffixed_str_buff);
    assert(strcmp(suffixed_str_buff, "987M") == 0);

    suffixed_str_buff[0] = '\0';
    truncate_uint_to_suffixed_str(123123123, 7, suffixed_str_buff);
    assert(strcmp(suffixed_str_buff, "123123K") == 0);

    suffixed_str_buff[0] = '\0';
    truncate_uint_to_suffixed_str(1123123123, 4, suffixed_str_buff);
    assert(strcmp(suffixed_str_buff, "1B") == 0);

    suffixed_str_buff[0] = '\0';
    truncate_uint_to_suffixed_str((uint32_t)3123123123, 4, suffixed_str_buff);
    assert(strcmp(suffixed_str_buff, "3B") == 0);

    suffixed_str_buff[0] = '\0';
    truncate_uint_to_suffixed_str(1234123123, 5, suffixed_str_buff);
    assert(strcmp(suffixed_str_buff, "1234M") == 0);
    
    suffixed_str_buff[0] = '\0';
    truncate_uint_to_suffixed_str(1123123123, 10, suffixed_str_buff);
    assert(strcmp(suffixed_str_buff, "1123123123") == 0);

    suffixed_str_buff[0] = '\0';
    truncate_uint_to_suffixed_str(1123123123, UINT_MAX_DIGITS, suffixed_str_buff);
    assert(strcmp(suffixed_str_buff, "1123123123") == 0);

    suffixed_str_buff[0] = '\0';
    truncate_uint_to_suffixed_str(1123123123, 100, suffixed_str_buff);
    assert(strcmp(suffixed_str_buff, "1123123123") == 0);

    suffixed_str_buff[0] = '\0';
    truncate_uint_to_suffixed_str(UINT32_MAX, 4, suffixed_str_buff);
    assert(strcmp(suffixed_str_buff, "4B") == 0);

    // This is the only test that checks rounding down, don't add any more
    suffixed_str_buff[0] = '\0';
    truncate_uint_to_suffixed_str(UINT32_MAX, 5, suffixed_str_buff);
    assert(strcmp(suffixed_str_buff, "4294M") == 0);

    char max_uint_str_buff[UINT_MAX_DIGITS + 1] = {'\0'};
    snprintf(max_uint_str_buff, sizeof(max_uint_str_buff), "%lu", UINT32_MAX);

    suffixed_str_buff[0] = '\0';
    truncate_uint_to_suffixed_str(UINT32_MAX, UINT_MAX_DIGITS, suffixed_str_buff);
    assert(strcmp(suffixed_str_buff, max_uint_str_buff) == 0);
}

int main()
{
    test_truncate_uint_to_suffixed_str();
    return 0;
}