#pragma once
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
// UF, version, opcode, sequence, status, payload length (little endian).
#define UF_NFC_HEADER 8U
#define UF_NFC_MAX_APDU 240U
#define UF_NFC_MAX_RESPONSE 256U
#define UF_NFC_VERSION 1U
#define UF_NFC_HELPER_VERSION "0.1.0"
enum { UfNfcInfo = 1, UfNfcActivate = 2, UfNfcTransceive = 3, UfNfcDeactivate = 4 };
enum {
    UfNfcOk = 0,
    UfNfcInvalid = 1,
    UfNfcUnsupported = 2,
    UfNfcNoCard = 3,
    UfNfcIo = 4,
    UfNfcTooLong = 5,
    UfNfcNotActive = 6
};
static inline size_t uf_nfc_payload_size(const uint8_t *data) {
    return (size_t)data[6] | ((size_t)data[7] << 8);
}
static inline bool uf_nfc_valid_request(const uint8_t *data, size_t size) {
    if (!data || size < UF_NFC_HEADER || size > UF_NFC_HEADER + UF_NFC_MAX_APDU)
        return false;
    if (data[0] != 'U' || data[1] != 'F' || data[2] != UF_NFC_VERSION || data[5] != 0)
        return false;
    if (data[3] < UfNfcInfo || data[3] > UfNfcDeactivate || uf_nfc_payload_size(data) != size - UF_NFC_HEADER)
        return false;
    return data[3] == UfNfcTransceive ? size > UF_NFC_HEADER : size == UF_NFC_HEADER;
}
static inline void uf_nfc_response_header(uint8_t *output, uint8_t opcode, uint8_t sequence, uint8_t status,
                                          size_t payload_size) {
    output[0] = 'U';
    output[1] = 'F';
    output[2] = UF_NFC_VERSION;
    output[3] = opcode | 0x80;
    output[4] = sequence;
    output[5] = status;
    output[6] = (uint8_t)payload_size;
    output[7] = (uint8_t)(payload_size >> 8);
}
