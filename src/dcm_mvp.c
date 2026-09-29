/*
 * AUTOSAR DCM Training MVP - Diagnostic DID Read Service Implementation
 *
 * Implements a simplified UDS ReadDataByIdentifier (0x22) service processor.
 * Accepts a three-byte request [SID, DID_HIGH, DID_LOW] and returns either
 * a positive response with configured data or a negative response.
 *
 * Requirement: DCM-MVP-001
 * See: requirements.md, design.md
 */

#include "dcm_mvp.h"
#include <string.h>

/* ============================================================================
 * DID Configuration: Fixed static payloads for supported DIDs
 * ============================================================================
 */

typedef struct
{
    uint16_t did;
    const uint8_t *data;
    uint16_t dataLength;
} DcmMvp_DidConfig;

/* Fictional synthetic test payloads for training MVP */
static const uint8_t dcm_mvp_vin_payload[DCM_MVP_DID_VIN_DATA_LENGTH] =
{
    'V', 'I', 'N', '_', 'T', 'R', 'A', 'I', 'N', 'I', 'N', 'G', '_', 'T', 'E', 'S', 'T'
};

static const uint8_t dcm_mvp_spare_part_payload[DCM_MVP_DID_SPARE_PART_DATA_LENGTH] =
{
    'S', 'P', '_', 'T', 'R', 'A', 'I', 'N', 'I', 'N'
};

/* DID lookup table */
static const DcmMvp_DidConfig dcm_mvp_did_table[] =
{
    {
        .did = DCM_MVP_DID_VIN,
        .data = dcm_mvp_vin_payload,
        .dataLength = DCM_MVP_DID_VIN_DATA_LENGTH
    },
    {
        .did = DCM_MVP_DID_SPARE_PART,
        .data = dcm_mvp_spare_part_payload,
        .dataLength = DCM_MVP_DID_SPARE_PART_DATA_LENGTH
    }
};

static const uint16_t dcm_mvp_did_table_size = 
    sizeof(dcm_mvp_did_table) / sizeof(dcm_mvp_did_table[0]);

/* ============================================================================
 * Internal Helper Functions
 * ============================================================================
 */

/*
 * Look up a DID in the configuration table
 * Returns pointer to configuration if found, NULL otherwise
 */
static const DcmMvp_DidConfig *dcm_mvp_lookup_did(uint16_t did)
{
    uint16_t i;
    for (i = 0u; i < dcm_mvp_did_table_size; ++i)
    {
        if (dcm_mvp_did_table[i].did == did)
        {
            return &dcm_mvp_did_table[i];
        }
    }
    return NULL;
}

/*
 * Decode a DID from two bytes in network byte order (high byte first)
 */
static uint16_t dcm_mvp_decode_did(const uint8_t *did_bytes)
{
    uint16_t did = ((uint16_t)did_bytes[0] << 8u) | (uint16_t)did_bytes[1];
    return did;
}

/*
 * Construct a positive response in the output buffer
 * Assumes buffer has been validated for sufficient capacity
 * Returns the number of bytes written
 */
static uint16_t dcm_mvp_construct_positive_response(
    uint8_t *response,
    const uint8_t *request_did_bytes,
    const DcmMvp_DidConfig *did_config)
{
    uint16_t offset = 0u;
    
    /* Positive response SID */
    response[offset++] = DCM_MVP_SID_POSITIVE_RESPONSE;
    
    /* DID bytes (high byte first) */
    response[offset++] = request_did_bytes[0];
    response[offset++] = request_did_bytes[1];
    
    /* Data payload */
    if (did_config != NULL && did_config->data != NULL)
    {
        (void)memcpy(&response[offset], did_config->data, did_config->dataLength);
        offset += did_config->dataLength;
    }
    
    return offset;
}

/*
 * Construct a negative response in the output buffer
 * Returns the number of bytes written (always 3 for negative responses)
 */
static uint16_t dcm_mvp_construct_negative_response(
    uint8_t *response,
    uint8_t nrc)
{
    response[0] = DCM_MVP_SID_NEGATIVE_RESPONSE;
    response[1] = DCM_MVP_SID_READ_DID;
    response[2] = nrc;
    
    return DCM_MVP_NEGATIVE_RESPONSE_LENGTH;
}

/* ============================================================================
 * Public API Implementation
 * ============================================================================
 */

DcmMvp_StatusType DcmMvp_ProcessRequest(
    const uint8_t *request,
    uint16_t requestLength,
    uint8_t *response,
    uint16_t responseCapacity,
    uint16_t *responseLength)
{
    uint16_t did;
    const DcmMvp_DidConfig *did_config;
    uint16_t response_len;
    
    /* Step 1: Initialize output and validate required pointers */
    if (responseLength != NULL)
    {
        *responseLength = 0u;
    }
    
    if (request == NULL || response == NULL || responseLength == NULL)
    {
        return DCM_MVP_INVALID_PARAM;
    }
    
    /* Step 2: Validate request length is exactly 3 bytes */
    if (requestLength != DCM_MVP_REQUEST_LENGTH)
    {
        /* Invalid request length => negative response NRC 0x13 */
        response_len = dcm_mvp_construct_negative_response(
            response,
            DCM_MVP_NRC_INVALID_FORMAT);
        
        if (response_len > responseCapacity)
        {
            return DCM_MVP_BUFFER_TOO_SMALL;
        }
        
        *responseLength = response_len;
        return DCM_MVP_OK;
    }
    
    /* Step 3: Validate service ID is 0x22 */
    if (request[0] != DCM_MVP_SID_READ_DID)
    {
        /* Invalid SID => negative response NRC 0x13 */
        response_len = dcm_mvp_construct_negative_response(
            response,
            DCM_MVP_NRC_INVALID_FORMAT);
        
        if (response_len > responseCapacity)
        {
            return DCM_MVP_BUFFER_TOO_SMALL;
        }
        
        *responseLength = response_len;
        return DCM_MVP_OK;
    }
    
    /* Step 4: Decode DID (high byte first) */
    did = dcm_mvp_decode_did(&request[1]);
    
    /* Step 5: Look up DID in configuration */
    did_config = dcm_mvp_lookup_did(did);
    
    if (did_config == NULL)
    {
        /* Unsupported DID => negative response NRC 0x31 */
        response_len = dcm_mvp_construct_negative_response(
            response,
            DCM_MVP_NRC_REQUEST_OUT_OF_RANGE);
        
        if (response_len > responseCapacity)
        {
            return DCM_MVP_BUFFER_TOO_SMALL;
        }
        
        *responseLength = response_len;
        return DCM_MVP_OK;
    }
    
    /* Step 6: Verify capacity before writing any response byte */
    response_len = 1u + DCM_MVP_DID_LENGTH + did_config->dataLength;
    
    if (response_len > responseCapacity)
    {
        return DCM_MVP_BUFFER_TOO_SMALL;
    }
    
    /* Step 7: Construct positive response and set output length */
    response_len = dcm_mvp_construct_positive_response(
        response,
        &request[1],
        did_config);
    
    *responseLength = response_len;
    
    return DCM_MVP_OK;
}
