/*
 * AUTOSAR DCM Training MVP - Diagnostic DID Read Service
 * Public interface for DcmMvp_ProcessRequest
 *
 * This is a training implementation of a simplified UDS ReadDataByIdentifier
 * service (0x22). It is not a complete ISO 14229 implementation or production
 * AUTOSAR DCM component.
 *
 * Requirement: DCM-MVP-001
 * See: requirements.md, design.md
 */

#ifndef DCM_MVP_H
#define DCM_MVP_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ============================================================================
 * API Status Type
 * ============================================================================
 */

typedef enum
{
    DCM_MVP_OK = 0u,
    DCM_MVP_INVALID_PARAM,
    DCM_MVP_BUFFER_TOO_SMALL
} DcmMvp_StatusType;

/* ============================================================================
 * Service and Protocol Constants
 * ============================================================================
 */

/* Service identifiers */
#define DCM_MVP_SID_READ_DID              (0x22u)
#define DCM_MVP_SID_POSITIVE_RESPONSE     (0x62u)
#define DCM_MVP_SID_NEGATIVE_RESPONSE     (0x7Fu)

/* Negative Response Codes (NRC) */
#define DCM_MVP_NRC_REQUEST_OUT_OF_RANGE  (0x31u)
#define DCM_MVP_NRC_INVALID_FORMAT        (0x13u)

/* Protocol field lengths */
#define DCM_MVP_REQUEST_LENGTH            (3u)
#define DCM_MVP_DID_LENGTH                (2u)
#define DCM_MVP_NEGATIVE_RESPONSE_LENGTH  (3u)

/* Supported DID identifiers */
#define DCM_MVP_DID_VIN                   (0xF190u)
#define DCM_MVP_DID_SPARE_PART            (0xF187u)

/* Data payload lengths for supported DIDs */
#define DCM_MVP_DID_VIN_DATA_LENGTH       (17u)
#define DCM_MVP_DID_SPARE_PART_DATA_LENGTH (10u)

/* Calculated positive response lengths (SID + DID + data) */
#define DCM_MVP_RESPONSE_LENGTH_VIN       (1u + DCM_MVP_DID_LENGTH + DCM_MVP_DID_VIN_DATA_LENGTH)
#define DCM_MVP_RESPONSE_LENGTH_SPARE_PART (1u + DCM_MVP_DID_LENGTH + DCM_MVP_DID_SPARE_PART_DATA_LENGTH)

/* ============================================================================
 * Public API
 * ============================================================================
 */

/*
 * Process a UDS ReadDataByIdentifier (0x22) request
 *
 * Accepts a request containing [SID, DID_HIGH, DID_LOW] and constructs either
 * a positive response [0x62, DID_HIGH, DID_LOW, data...] or a negative response
 * [0x7F, 0x22, NRC].
 *
 * Parameters:
 *   request           - Pointer to request buffer (caller-owned); must contain
 *                       request data if requestLength > 0
 *   requestLength     - Length of request data (must be exactly 3 bytes for valid requests)
 *   response          - Pointer to output buffer (caller-owned); module writes response here
 *   responseCapacity  - Available space in response buffer
 *   responseLength    - Pointer to variable; on success or capacity error, set to the
 *                       number of bytes written (or 0 if not written)
 *
 * Return value:
 *   DCM_MVP_OK                - Response successfully constructed; *responseLength set to bytes written
 *   DCM_MVP_INVALID_PARAM     - A required pointer (request, response, or responseLength) is null;
 *                               *responseLength set to 0 if that pointer is non-null
 *   DCM_MVP_BUFFER_TOO_SMALL  - Response capacity insufficient; no bytes written; *responseLength set to 0
 *
 * Behavior:
 *   - For each call, if responseLength is non-null, it is first set to 0 (or the final response length)
 *   - If any required pointer is null, no response bytes are written
 *   - If requestLength != 3, a negative response (NRC 0x13) is constructed if capacity permits
 *   - If SID != 0x22, a negative response (NRC 0x13) is constructed if capacity permits
 *   - If SID == 0x22 and DID is not configured, a negative response (NRC 0x31) is constructed if capacity permits
 *   - On insufficient capacity, no response bytes are written and status is BUFFER_TOO_SMALL
 *   - DIDs are decoded in network byte order (high byte first)
 *   - All operations are deterministic and use only caller-provided buffers and fixed configuration
 */
DcmMvp_StatusType DcmMvp_ProcessRequest(
    const uint8_t *request,
    uint16_t requestLength,
    uint8_t *response,
    uint16_t responseCapacity,
    uint16_t *responseLength);

#ifdef __cplusplus
}
#endif

#endif /* DCM_MVP_H */
