/*
 * Unit tests for the AUTOSAR DCM Training MVP.
 *
 * This suite is aligned to the requirements in Docs/requirements.md and the
 * behavioral design in Docs/design.md. It validates supported DIDs,
 * malformed requests, null pointers, invalid response capacity, and exact
 * response-length reporting.
 */

#include "dcm_mvp.h"
#include <stddef.h>
#include <stdio.h>
#include <string.h>

#define TEST_ASSERT(cond, message)                                               \
    do {                                                                         \
        if (!(cond)) {                                                           \
            printf("FAIL: %s\n", message);                                     \
            return;                                                             \
        }                                                                        \
    } while (0)

#define TEST_ASSERT_EQUAL_U16(expected, actual, message)                          \
    do {                                                                         \
        if ((expected) != (actual)) {                                            \
            printf("FAIL: %s (expected=%u, actual=%u)\n", message,             \
                   (unsigned int)(expected), (unsigned int)(actual));          \
            return;                                                             \
        }                                                                        \
    } while (0)

#define TEST_ASSERT_EQUAL_U8(expected, actual, message)                           \
    do {                                                                         \
        if ((expected) != (actual)) {                                            \
            printf("FAIL: %s (expected=0x%02X, actual=0x%02X)\n", message,    \
                   (unsigned int)(expected), (unsigned int)(actual));          \
            return;                                                             \
        }                                                                        \
    } while (0)

#define TEST_ASSERT_MEMEQ(expected, actual, length, message)                      \
    do {                                                                         \
        if (memcmp((expected), (actual), (length)) != 0) {                       \
            printf("FAIL: %s\n", message);                                     \
            return;                                                             \
        }                                                                        \
    } while (0)

static const uint8_t test_vin_payload[DCM_MVP_DID_VIN_DATA_LENGTH] =
{
    'V', 'I', 'N', '_', 'T', 'R', 'A', 'I', 'N', 'I', 'N', 'G', '_', 'T', 'E', 'S', 'T'
};

static const uint8_t test_spare_part_payload[DCM_MVP_DID_SPARE_PART_DATA_LENGTH] =
{
    'S', 'P', '_', 'T', 'R', 'A', 'I', 'N', 'I', 'N'
};

static void fill_sentinel(uint8_t *buffer, uint16_t length, uint8_t value)
{
    uint16_t i;
    for (i = 0u; i < length; ++i)
    {
        buffer[i] = value;
    }
}

static int buffer_unchanged(const uint8_t *buffer, uint16_t length, uint8_t sentinel)
{
    uint16_t i;
    for (i = 0u; i < length; ++i)
    {
        if (buffer[i] != sentinel)
        {
            return 0;
        }
    }
    return 1;
}

static void test_supported_did_f190(void)
{
    const uint8_t request[] = { 0x22u, 0xF1u, 0x90u };
    uint8_t response[20];
    uint16_t responseLength = 0u;

    DcmMvp_StatusType status = DcmMvp_ProcessRequest(
        request,
        (uint16_t)sizeof(request),
        response,
        (uint16_t)sizeof(response),
        &responseLength);

    TEST_ASSERT_EQUAL_U8(DCM_MVP_OK, status, "UT-01: wrong status");
    TEST_ASSERT_EQUAL_U16(20u, responseLength, "UT-01: wrong response length");
    TEST_ASSERT_EQUAL_U8(0x62u, response[0], "UT-01: wrong SID");
    TEST_ASSERT_EQUAL_U8(0xF1u, response[1], "UT-01: wrong DID high byte");
    TEST_ASSERT_EQUAL_U8(0x90u, response[2], "UT-01: wrong DID low byte");
    TEST_ASSERT_MEMEQ(test_vin_payload, &response[3], DCM_MVP_DID_VIN_DATA_LENGTH,
        "UT-01: payload mismatch");
}

static void test_supported_did_f187(void)
{
    const uint8_t request[] = { 0x22u, 0xF1u, 0x87u };
    uint8_t response[13];
    uint16_t responseLength = 0u;

    DcmMvp_StatusType status = DcmMvp_ProcessRequest(
        request,
        (uint16_t)sizeof(request),
        response,
        (uint16_t)sizeof(response),
        &responseLength);

    TEST_ASSERT_EQUAL_U8(DCM_MVP_OK, status, "UT-02: wrong status");
    TEST_ASSERT_EQUAL_U16(13u, responseLength, "UT-02: wrong response length");
    TEST_ASSERT_EQUAL_U8(0x62u, response[0], "UT-02: wrong SID");
    TEST_ASSERT_EQUAL_U8(0xF1u, response[1], "UT-02: wrong DID high byte");
    TEST_ASSERT_EQUAL_U8(0x87u, response[2], "UT-02: wrong DID low byte");
    TEST_ASSERT_MEMEQ(test_spare_part_payload, &response[3], DCM_MVP_DID_SPARE_PART_DATA_LENGTH,
        "UT-02: payload mismatch");
}

static void test_unsupported_did_returns_nrc_0x31(void)
{
    const uint8_t request[] = { 0x22u, 0xF1u, 0x99u };
    uint8_t response[3];
    uint16_t responseLength = 0u;

    DcmMvp_StatusType status = DcmMvp_ProcessRequest(
        request,
        (uint16_t)sizeof(request),
        response,
        (uint16_t)sizeof(response),
        &responseLength);

    TEST_ASSERT_EQUAL_U8(DCM_MVP_OK, status, "UT-03: wrong status");
    TEST_ASSERT_EQUAL_U16(3u, responseLength, "UT-03: wrong response length");
    TEST_ASSERT_EQUAL_U8(0x7Fu, response[0], "UT-03: wrong negative response marker");
    TEST_ASSERT_EQUAL_U8(0x22u, response[1], "UT-03: wrong service ID in NRC");
    TEST_ASSERT_EQUAL_U8(0x31u, response[2], "UT-03: wrong NRC");
}

static void test_short_request_length_returns_nrc_0x13(void)
{
    uint8_t response[3];
    uint16_t responseLength = 0u;
    const uint8_t request_short1[] = { 0x22u };
    const uint8_t request_short2[] = { 0x22u, 0xF1u };

    DcmMvp_StatusType status1 = DcmMvp_ProcessRequest(
        request_short1,
        1u,
        response,
        (uint16_t)sizeof(response),
        &responseLength);

    TEST_ASSERT_EQUAL_U8(DCM_MVP_OK, status1, "UT-04a: wrong status");
    TEST_ASSERT_EQUAL_U16(3u, responseLength, "UT-04a: wrong response length");
    TEST_ASSERT_EQUAL_U8(0x7Fu, response[0], "UT-04a: wrong negative marker");
    TEST_ASSERT_EQUAL_U8(0x22u, response[1], "UT-04a: wrong service ID");
    TEST_ASSERT_EQUAL_U8(0x13u, response[2], "UT-04a: wrong NRC");

    responseLength = 0u;
    DcmMvp_StatusType status2 = DcmMvp_ProcessRequest(
        request_short2,
        2u,
        response,
        (uint16_t)sizeof(response),
        &responseLength);

    TEST_ASSERT_EQUAL_U8(DCM_MVP_OK, status2, "UT-04b: wrong status");
    TEST_ASSERT_EQUAL_U16(3u, responseLength, "UT-04b: wrong response length");
    TEST_ASSERT_EQUAL_U8(0x7Fu, response[0], "UT-04b: wrong negative marker");
    TEST_ASSERT_EQUAL_U8(0x22u, response[1], "UT-04b: wrong service ID");
    TEST_ASSERT_EQUAL_U8(0x13u, response[2], "UT-04b: wrong NRC");
}

static void test_long_request_length_returns_nrc_0x13(void)
{
    const uint8_t request[] = { 0x22u, 0xF1u, 0x90u, 0xAAu };
    uint8_t response[3];
    uint16_t responseLength = 0u;

    DcmMvp_StatusType status = DcmMvp_ProcessRequest(
        request,
        4u,
        response,
        (uint16_t)sizeof(response),
        &responseLength);

    TEST_ASSERT_EQUAL_U8(DCM_MVP_OK, status, "UT-05: wrong status");
    TEST_ASSERT_EQUAL_U16(3u, responseLength, "UT-05: wrong response length");
    TEST_ASSERT_EQUAL_U8(0x7Fu, response[0], "UT-05: wrong negative marker");
    TEST_ASSERT_EQUAL_U8(0x22u, response[1], "UT-05: wrong service ID");
    TEST_ASSERT_EQUAL_U8(0x13u, response[2], "UT-05: wrong NRC");
}

static void test_wrong_sid_returns_nrc_0x13(void)
{
    const uint8_t request[] = { 0x19u, 0xF1u, 0x90u };
    uint8_t response[3];
    uint16_t responseLength = 0u;

    DcmMvp_StatusType status = DcmMvp_ProcessRequest(
        request,
        (uint16_t)sizeof(request),
        response,
        (uint16_t)sizeof(response),
        &responseLength);

    TEST_ASSERT_EQUAL_U8(DCM_MVP_OK, status, "UT-06: wrong status");
    TEST_ASSERT_EQUAL_U16(3u, responseLength, "UT-06: wrong response length");
    TEST_ASSERT_EQUAL_U8(0x7Fu, response[0], "UT-06: wrong negative marker");
    TEST_ASSERT_EQUAL_U8(0x22u, response[1], "UT-06: wrong service ID");
    TEST_ASSERT_EQUAL_U8(0x13u, response[2], "UT-06: wrong NRC");
}

static void test_null_pointers_return_invalid_param(void)
{
    const uint8_t request[] = { 0x22u, 0xF1u, 0x90u };
    uint8_t response[20];
    uint16_t responseLength = 0xABCDu;

    TEST_ASSERT_EQUAL_U8(DCM_MVP_INVALID_PARAM,
        DcmMvp_ProcessRequest(NULL, (uint16_t)sizeof(request), response, sizeof(response), &responseLength),
        "UT-07: request pointer null should fail");
    TEST_ASSERT_EQUAL_U16(0u, responseLength, "UT-07: responseLength should be zeroed");

    responseLength = 0xABCDu;
    TEST_ASSERT_EQUAL_U8(DCM_MVP_INVALID_PARAM,
        DcmMvp_ProcessRequest(request, (uint16_t)sizeof(request), NULL, sizeof(response), &responseLength),
        "UT-08: response pointer null should fail");
    TEST_ASSERT_EQUAL_U16(0u, responseLength, "UT-08: responseLength should be zeroed");

    fill_sentinel(response, (uint16_t)sizeof(response), 0xA5u);
    TEST_ASSERT_EQUAL_U8(DCM_MVP_INVALID_PARAM,
        DcmMvp_ProcessRequest(request, (uint16_t)sizeof(request), response, sizeof(response), NULL),
        "UT-09: responseLength pointer null should fail");
    TEST_ASSERT(buffer_unchanged(response, (uint16_t)sizeof(response), 0xA5u),
        "UT-09: response should remain untouched");
}

static void test_capacity_too_small_for_positive_response(void)
{
    const uint8_t request_vin[] = { 0x22u, 0xF1u, 0x90u };
    const uint8_t request_spare[] = { 0x22u, 0xF1u, 0x87u };
    uint8_t response_vin[20];
    uint8_t response_spare[13];
    uint16_t responseLength = 0xABCDu;

    fill_sentinel(response_vin, (uint16_t)sizeof(response_vin), 0x55u);
    TEST_ASSERT_EQUAL_U8(DCM_MVP_BUFFER_TOO_SMALL,
        DcmMvp_ProcessRequest(request_vin, (uint16_t)sizeof(request_vin), response_vin, 19u, &responseLength),
        "UT-10a: VIN capacity too small");
    TEST_ASSERT_EQUAL_U16(0u, responseLength, "UT-10a: responseLength should be zeroed");
    TEST_ASSERT(buffer_unchanged(response_vin, (uint16_t)sizeof(response_vin), 0x55u),
        "UT-10a: VIN response buffer should remain unchanged");

    fill_sentinel(response_spare, (uint16_t)sizeof(response_spare), 0x66u);
    responseLength = 0xABCDu;
    TEST_ASSERT_EQUAL_U8(DCM_MVP_BUFFER_TOO_SMALL,
        DcmMvp_ProcessRequest(request_spare, (uint16_t)sizeof(request_spare), response_spare, 12u, &responseLength),
        "UT-10b: spare-part capacity too small");
    TEST_ASSERT_EQUAL_U16(0u, responseLength, "UT-10b: responseLength should be zeroed");
    TEST_ASSERT(buffer_unchanged(response_spare, (uint16_t)sizeof(response_spare), 0x66u),
        "UT-10b: spare-part response buffer should remain unchanged");
}

static void test_capacity_too_small_for_negative_response(void)
{
    const uint8_t request[] = { 0x22u, 0xF1u, 0x99u };
    uint8_t response[3];
    uint16_t responseLength = 0xABCDu;

    fill_sentinel(response, (uint16_t)sizeof(response), 0x77u);
    TEST_ASSERT_EQUAL_U8(DCM_MVP_BUFFER_TOO_SMALL,
        DcmMvp_ProcessRequest(request, (uint16_t)sizeof(request), response, 2u, &responseLength),
        "UT-11: negative response capacity too small");
    TEST_ASSERT_EQUAL_U16(0u, responseLength, "UT-11: responseLength should be zeroed");
    TEST_ASSERT(buffer_unchanged(response, (uint16_t)sizeof(response), 0x77u),
        "UT-11: response buffer should remain unchanged");
}

static void test_exact_capacity_for_positive_and_negative_cases(void)
{
    const uint8_t request_vin[] = { 0x22u, 0xF1u, 0x90u };
    const uint8_t request_spare[] = { 0x22u, 0xF1u, 0x87u };
    const uint8_t request_unsupported[] = { 0x22u, 0xF1u, 0x99u };
    uint8_t response_vin[20];
    uint8_t response_spare[13];
    uint8_t response_neg[3];
    uint16_t responseLength = 0u;

    DcmMvp_StatusType status1 = DcmMvp_ProcessRequest(
        request_vin,
        (uint16_t)sizeof(request_vin),
        response_vin,
        (uint16_t)sizeof(response_vin),
        &responseLength);
    TEST_ASSERT_EQUAL_U8(DCM_MVP_OK, status1, "UT-12: VIN status");
    TEST_ASSERT_EQUAL_U16(20u, responseLength, "UT-12: VIN response length");
    TEST_ASSERT_EQUAL_U8(0x62u, response_vin[0], "UT-12: VIN SID");

    responseLength = 0u;
    DcmMvp_StatusType status2 = DcmMvp_ProcessRequest(
        request_spare,
        (uint16_t)sizeof(request_spare),
        response_spare,
        (uint16_t)sizeof(response_spare),
        &responseLength);
    TEST_ASSERT_EQUAL_U8(DCM_MVP_OK, status2, "UT-12: spare-part status");
    TEST_ASSERT_EQUAL_U16(13u, responseLength, "UT-12: spare-part response length");
    TEST_ASSERT_EQUAL_U8(0x62u, response_spare[0], "UT-12: spare-part SID");

    responseLength = 0u;
    DcmMvp_StatusType status3 = DcmMvp_ProcessRequest(
        request_unsupported,
        (uint16_t)sizeof(request_unsupported),
        response_neg,
        (uint16_t)sizeof(response_neg),
        &responseLength);
    TEST_ASSERT_EQUAL_U8(DCM_MVP_OK, status3, "UT-12: negative status");
    TEST_ASSERT_EQUAL_U16(3u, responseLength, "UT-12: negative response length");
    TEST_ASSERT_EQUAL_U8(0x7Fu, response_neg[0], "UT-12: negative marker");
    TEST_ASSERT_EQUAL_U8(0x22u, response_neg[1], "UT-12: negative service ID");
    TEST_ASSERT_EQUAL_U8(0x31u, response_neg[2], "UT-12: negative NRC");
}

static void test_reversed_did_bytes_are_not_accepted(void)
{
    const uint8_t request[] = { 0x22u, 0x90u, 0xF1u };
    uint8_t response[3];
    uint16_t responseLength = 0u;

    DcmMvp_StatusType status = DcmMvp_ProcessRequest(
        request,
        (uint16_t)sizeof(request),
        response,
        (uint16_t)sizeof(response),
        &responseLength);

    TEST_ASSERT_EQUAL_U8(DCM_MVP_OK, status, "UT-14: wrong status for reversed DID");
    TEST_ASSERT_EQUAL_U16(3u, responseLength, "UT-14: wrong response length");
    TEST_ASSERT_EQUAL_U8(0x7Fu, response[0], "UT-14: wrong negative marker");
    TEST_ASSERT_EQUAL_U8(0x22u, response[1], "UT-14: wrong service ID");
    TEST_ASSERT_EQUAL_U8(0x31u, response[2], "UT-14: wrong NRC");
}

static void test_zero_capacity_buffer_rejected(void)
{
    const uint8_t request[] = { 0x22u, 0xF1u, 0x90u };
    uint8_t response[1];
    uint16_t responseLength = 0xABCDu;

    TEST_ASSERT_EQUAL_U8(DCM_MVP_BUFFER_TOO_SMALL,
        DcmMvp_ProcessRequest(request, (uint16_t)sizeof(request), response, 0u, &responseLength),
        "UT-15: zero-capacity request should fail");
    TEST_ASSERT_EQUAL_U16(0u, responseLength, "UT-15: responseLength should be zeroed");
}

static void test_deterministic_behavior(void)
{
    const uint8_t request[] = { 0x22u, 0xF1u, 0x90u };
    uint8_t response1[20];
    uint8_t response2[20];
    uint16_t length1 = 0u;
    uint16_t length2 = 0u;

    DcmMvp_StatusType status1 = DcmMvp_ProcessRequest(
        request,
        (uint16_t)sizeof(request),
        response1,
        (uint16_t)sizeof(response1),
        &length1);

    DcmMvp_StatusType status2 = DcmMvp_ProcessRequest(
        request,
        (uint16_t)sizeof(request),
        response2,
        (uint16_t)sizeof(response2),
        &length2);

    TEST_ASSERT_EQUAL_U8(status1, status2, "UT-13: statuses differ");
    TEST_ASSERT_EQUAL_U16(length1, length2, "UT-13: lengths differ");
    TEST_ASSERT_MEMEQ(response1, response2, length1, "UT-13: response bytes differ");
}

int main(void)
{
    printf("Running DCM MVP unit tests...\n");

    test_supported_did_f190();
    test_supported_did_f187();
    test_unsupported_did_returns_nrc_0x31();
    test_short_request_length_returns_nrc_0x13();
    test_long_request_length_returns_nrc_0x13();
    test_wrong_sid_returns_nrc_0x13();
    test_null_pointers_return_invalid_param();
    test_capacity_too_small_for_positive_response();
    test_capacity_too_small_for_negative_response();
    test_exact_capacity_for_positive_and_negative_cases();
    test_reversed_did_bytes_are_not_accepted();
    test_zero_capacity_buffer_rejected();
    test_deterministic_behavior();

    printf("All DCM MVP unit tests passed.\n");
    return 0;
}
