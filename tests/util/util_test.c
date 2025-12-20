#include <font.h>
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

    // TODO: Edge cases 1.01 etc.

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
    assert(strcmp(suffixed_str_buff, "12" XSTR(FP1_CHAR) "K") == 0);   // "12.1K"

    suffixed_str_buff[0] = '\0';
    truncate_uint_to_suffixed_str(123123, 4, suffixed_str_buff);
    assert(strcmp(suffixed_str_buff, "123K") == 0);

    suffixed_str_buff[0] = '\0';
    truncate_uint_to_suffixed_str(123123, 5, suffixed_str_buff);
    assert(strcmp(suffixed_str_buff, "123" XSTR(FP1_CHAR) "K") == 0);  // "123.1K"

    suffixed_str_buff[0] = '\0';
    truncate_uint_to_suffixed_str(123123, 6, suffixed_str_buff);
    assert(strcmp(suffixed_str_buff, "123123") == 0);

    suffixed_str_buff[0] = '\0';
    truncate_uint_to_suffixed_str(123123, 7, suffixed_str_buff);
    assert(strcmp(suffixed_str_buff, "123123") == 0);

    suffixed_str_buff[0] = '\0';
    truncate_uint_to_suffixed_str(12345123, 6, suffixed_str_buff);
    assert(strcmp(suffixed_str_buff, "12" XSTR(FP3_CHAR) "45M") == 0); // Or "12.345M" ?

    suffixed_str_buff[0] = '\0';
    truncate_uint_to_suffixed_str(12123123, 5, suffixed_str_buff);
    assert(strcmp(suffixed_str_buff, "12" XSTR(FP1_CHAR) "2M") == 0); // "12.12M"

    suffixed_str_buff[0] = '\0';
    truncate_uint_to_suffixed_str(12123123, 4, suffixed_str_buff);
    assert(strcmp(suffixed_str_buff, "12" XSTR(FP1_CHAR) "M") == 0); // "12.1M"

    suffixed_str_buff[0] = '\0';
    truncate_uint_to_suffixed_str(54123123, 4, suffixed_str_buff);
    assert(strcmp(suffixed_str_buff, "54" XSTR(FP1_CHAR) "M") == 0); // "54.1M"

    suffixed_str_buff[0] = '\0';
    truncate_uint_to_suffixed_str(123123123, 4, suffixed_str_buff);
    assert(strcmp(suffixed_str_buff, "123M") == 0);

    suffixed_str_buff[0] = '\0';
    truncate_uint_to_suffixed_str(123123123, 6, suffixed_str_buff);
    assert(strcmp(suffixed_str_buff, "123" XSTR(FP1_CHAR) "2M") == 0);  // "123.12M"

    suffixed_str_buff[0] = '\0';
    truncate_uint_to_suffixed_str(987123123, 6, suffixed_str_buff);
    assert(strcmp(suffixed_str_buff, "987" XSTR(FP1_CHAR) "2M") == 0);  // "987.12M"

    suffixed_str_buff[0] = '\0';
    truncate_uint_to_suffixed_str(123123123, 7, suffixed_str_buff);
    assert(strcmp(suffixed_str_buff, "123" XSTR(FP1_CHAR) "23M") == 0); // 123.123M

    suffixed_str_buff[0] = '\0';
    truncate_uint_to_suffixed_str(123123123, 8, suffixed_str_buff);
    assert(strcmp(suffixed_str_buff, "123" XSTR(FP1_CHAR) "231M") == 0); // 123.1231M

    suffixed_str_buff[0] = '\0';
    truncate_uint_to_suffixed_str(123123123, 9, suffixed_str_buff);
    assert(strcmp(suffixed_str_buff, "123123123") == 0);

    suffixed_str_buff[0] = '\0';
    truncate_uint_to_suffixed_str(1123123123, 4, suffixed_str_buff);
    assert(strcmp(suffixed_str_buff, "1" XSTR(FP1_CHAR) "2B") == 0); // "1.12B"

    suffixed_str_buff[0] = '\0';
    truncate_uint_to_suffixed_str((uint32_t)3123123123, 4, suffixed_str_buff);
    assert(strcmp(suffixed_str_buff, "3" XSTR(FP1_CHAR) "2B") == 0); // "3.12B"

    suffixed_str_buff[0] = '\0';
    truncate_uint_to_suffixed_str(1234123123, 5, suffixed_str_buff);
    assert(strcmp(suffixed_str_buff, "1" XSTR(FP2_CHAR) "34B") == 0); // "1.234B"
    
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
    assert(strcmp(suffixed_str_buff, "4" XSTR(FP2_CHAR) "9B") == 0);

    // This is the only test that checks rounding down, don't add any more
    suffixed_str_buff[0] = '\0';
    truncate_uint_to_suffixed_str(UINT32_MAX, 5, suffixed_str_buff);
    assert(strcmp(suffixed_str_buff, "4" XSTR(FP2_CHAR) "94B") == 0); // "4.294B"

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