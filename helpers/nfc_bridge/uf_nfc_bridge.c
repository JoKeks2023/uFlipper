#include "bridge_protocol.h"
#include <furi.h>
#include <gui/gui.h>
#include <input/input.h>
#include <nfc/nfc.h>
#include <nfc/nfc_poller.h>
#include <nfc/protocols/iso14443_4a/iso14443_4a_poller.h>
#include <rpc/rpc_app.h>
#include <stdatomic.h>
#include <stdio.h>
#include <string.h>

#define FlagExit (1U << 0)
#define FlagResponse (1U << 1)
#define FlagRequest (1U << 2)

typedef struct {
    uint8_t bytes[UF_NFC_HEADER + UF_NFC_MAX_APDU];
    size_t size;
} HostRequest;
typedef struct {
    uint8_t bytes[UF_NFC_MAX_APDU];
    size_t size;
} CardRequest;
typedef struct {
    FuriThreadId thread;
    FuriMutex *rpc_lock;
    RpcAppSystem *rpc;
    FuriMessageQueue *host_requests;
    FuriMessageQueue *card_requests;
    Nfc *nfc;
    NfcPoller *poller;
    BitBuffer *tx;
    BitBuffer *rx;
    atomic_bool stop;
    atomic_bool active;
    atomic_bool exiting;
    bool exit_ack;
    uint8_t result_status;
    size_t result_size;
    uint8_t result[UF_NFC_MAX_RESPONSE];
    size_t frame_limit;
} Bridge;

// Called on the NFC worker. The card and ISO-DEP block numbering remain active
// between host requests. Do not return to discovery between dependent APDUs.
static NfcCommand card_event(NfcGenericEvent event, void *context) {
    Bridge *bridge = context;
    Iso14443_4aPollerEvent *card_event = event.event_data;
    if (atomic_load(&bridge->stop))
        return NfcCommandStop;
    if (card_event->type != Iso14443_4aPollerEventTypeReady) {
        bridge->result_status = UfNfcNoCard;
        bridge->result_size = 0;
        furi_thread_flags_set(bridge->thread, FlagResponse);
        return NfcCommandStop;
    }
    const Iso14443_4aData *data = nfc_poller_get_data(bridge->poller);
    const Iso14443_3aData *base = data->iso14443_3a_data;
    size_t frame_size = iso14443_4a_get_frame_size_max(data);
    bridge->frame_limit = frame_size > 4 ? MIN(frame_size - 4, UF_NFC_MAX_APDU) : 0;
    bridge->result_size = 1 + base->uid_len + 2;
    bridge->result[0] = base->uid_len;
    memcpy(bridge->result + 1, base->uid, base->uid_len);
    bridge->result[1 + base->uid_len] = (uint8_t)bridge->frame_limit;
    bridge->result[2 + base->uid_len] = 0;
    bridge->result_status = UfNfcOk;
    atomic_store(&bridge->active, true);
    furi_thread_flags_set(bridge->thread, FlagResponse);

    while (!atomic_load(&bridge->stop)) {
        CardRequest request;
        if (furi_message_queue_get(bridge->card_requests, &request, 50) != FuriStatusOk)
            continue;
        bridge->result_size = 0;
        if (request.size > bridge->frame_limit) {
            bridge->result_status = UfNfcTooLong;
        } else {
            bit_buffer_copy_bytes(bridge->tx, request.bytes, request.size);
            bit_buffer_reset(bridge->rx);
            Iso14443_4aError error = iso14443_4a_poller_send_block(event.instance, bridge->tx, bridge->rx);
            bridge->result_status = error == Iso14443_4aErrorNone ? UfNfcOk : UfNfcIo;
            if (error == Iso14443_4aErrorNone) {
                bridge->result_size = bit_buffer_get_size_bytes(bridge->rx);
                memcpy(bridge->result, bit_buffer_get_data(bridge->rx), bridge->result_size);
            } else {
                atomic_store(&bridge->active, false);
                atomic_store(&bridge->stop, true);
            }
        }
        furi_thread_flags_set(bridge->thread, FlagResponse);
    }
    atomic_store(&bridge->active, false);
    return NfcCommandStop;
}

static void stop_card(Bridge *bridge) {
    atomic_store(&bridge->stop, true);
    if (bridge->poller) {
        nfc_poller_stop(bridge->poller);
        nfc_poller_free(bridge->poller);
        bridge->poller = NULL;
    }
    atomic_store(&bridge->active, false);
    furi_message_queue_reset(bridge->card_requests);
    furi_thread_flags_clear(FlagResponse);
}

static void rpc_event(const RpcAppSystemEvent *event, void *context) {
    Bridge *bridge = context;
    furi_mutex_acquire(bridge->rpc_lock, FuriWaitForever);
    if (event->type == RpcAppEventTypeSessionClose) {
        bridge->rpc = NULL;
        atomic_store(&bridge->exiting, true);
        atomic_store(&bridge->stop, true);
        furi_thread_flags_set(bridge->thread, FlagExit);
    } else if (event->type == RpcAppEventTypeAppExit) {
        bridge->exit_ack = true;
        atomic_store(&bridge->exiting, true);
        atomic_store(&bridge->stop, true);
        furi_thread_flags_set(bridge->thread, FlagExit);
    } else if (event->type == RpcAppEventTypeDataExchange) {
        HostRequest request;
        bool valid = event->data.type == RpcAppSystemEventDataTypeBytes &&
                     uf_nfc_valid_request(event->data.bytes.ptr, event->data.bytes.size);
        if (valid) {
            request.size = event->data.bytes.size;
            memcpy(request.bytes, event->data.bytes.ptr, request.size);
            valid = furi_message_queue_put(bridge->host_requests, &request, 0) == FuriStatusOk;
        }
        // Accept transport delivery immediately; correlated protocol response follows.
        if (bridge->rpc)
            rpc_system_app_confirm(bridge->rpc, valid);
        if (valid)
            furi_thread_flags_set(bridge->thread, FlagRequest);
    } else if (bridge->rpc) {
        rpc_system_app_confirm(bridge->rpc, false);
    }
    furi_mutex_release(bridge->rpc_lock);
}

static void respond(Bridge *bridge, const HostRequest *request, uint8_t status, const uint8_t *payload,
                    size_t size) {
    uint8_t response[UF_NFC_HEADER + UF_NFC_MAX_RESPONSE];
    uf_nfc_response_header(response, request->bytes[3], request->bytes[4], status, size);
    if (size)
        memcpy(response + UF_NFC_HEADER, payload, size);
    furi_mutex_acquire(bridge->rpc_lock, FuriWaitForever);
    if (bridge->rpc)
        rpc_system_app_exchange_data(bridge->rpc, response, UF_NFC_HEADER + size);
    furi_mutex_release(bridge->rpc_lock);
}

static void handle_request(Bridge *bridge, const HostRequest *request) {
    uint8_t command = request->bytes[3];
    if (command == UfNfcInfo) {
        const uint8_t info[] = "uf-nfc/" UF_NFC_HELPER_VERSION ";iso14443-4a;short-apdu;max=240";
        respond(bridge, request, UfNfcOk, info, sizeof(info) - 1);
        return;
    }
    if (command == UfNfcDeactivate) {
        stop_card(bridge);
        respond(bridge, request, UfNfcOk, NULL, 0);
        return;
    }
    if (command == UfNfcActivate) {
        stop_card(bridge);
        atomic_store(&bridge->stop, false);
        bridge->poller = nfc_poller_alloc(bridge->nfc, NfcProtocolIso14443_4a);
        nfc_poller_start(bridge->poller, card_event, bridge);
    } else if (command == UfNfcTransceive) {
        if (!atomic_load(&bridge->active)) {
            respond(bridge, request, UfNfcNotActive, NULL, 0);
            return;
        }
        CardRequest card_request = {.size = uf_nfc_payload_size(request->bytes)};
        memcpy(card_request.bytes, request->bytes + UF_NFC_HEADER, card_request.size);
        furi_thread_flags_clear(FlagResponse);
        furi_message_queue_put(bridge->card_requests, &card_request, FuriWaitForever);
    }
    uint32_t flags = furi_thread_flags_wait(FlagResponse | FlagExit, FuriFlagWaitAny, 3000);
    if ((flags & FuriFlagError) || (flags & FlagExit)) {
        stop_card(bridge);
        respond(bridge, request, command == UfNfcActivate ? UfNfcNoCard : UfNfcIo, NULL, 0);
    } else {
        respond(bridge, request, bridge->result_status, bridge->result, bridge->result_size);
        if (bridge->result_status != UfNfcOk && bridge->result_status != UfNfcTooLong)
            stop_card(bridge);
    }
}

static void draw(Canvas *canvas, void *context) {
    UNUSED(context);
    canvas_set_font(canvas, FontPrimary);
    canvas_draw_str(canvas, 3, 16, "uFlipper NFC 0.1");
    canvas_set_font(canvas, FontSecondary);
    canvas_draw_str(canvas, 3, 32, "Experimental App-RPC bridge");
    canvas_draw_str(canvas, 3, 45, "Control from uFlipper");
    canvas_draw_str(canvas, 3, 59, "Back: stop and exit");
}
static void input(InputEvent *event, void *context) {
    Bridge *bridge = context;
    if (event->key == InputKeyBack && event->type == InputTypeShort) {
        atomic_store(&bridge->exiting, true);
        atomic_store(&bridge->stop, true);
        furi_thread_flags_set(bridge->thread, FlagExit);
    }
}
int32_t uf_nfc_bridge_main(void *args) {
    Bridge *bridge = calloc(1, sizeof(Bridge));
    bridge->thread = furi_thread_get_current_id();
    bridge->rpc_lock = furi_mutex_alloc(FuriMutexTypeNormal);
    bridge->host_requests = furi_message_queue_alloc(1, sizeof(HostRequest));
    bridge->card_requests = furi_message_queue_alloc(1, sizeof(CardRequest));
    bridge->nfc = nfc_alloc();
    bridge->tx = bit_buffer_alloc(UF_NFC_MAX_RESPONSE);
    bridge->rx = bit_buffer_alloc(UF_NFC_MAX_RESPONSE);
    atomic_init(&bridge->stop, true);
    atomic_init(&bridge->active, false);
    atomic_init(&bridge->exiting, false);
    uint32_t rpc_address = 0;
    if (args && sscanf(args, "RPC %lx", &rpc_address) == 1 && rpc_address) {
        bridge->rpc = (RpcAppSystem *)rpc_address;
        rpc_system_app_set_callback(bridge->rpc, rpc_event, bridge);
        rpc_system_app_send_started(bridge->rpc);
    }
    ViewPort *viewport = view_port_alloc();
    view_port_draw_callback_set(viewport, draw, bridge);
    view_port_input_callback_set(viewport, input, bridge);
    Gui *gui = furi_record_open(RECORD_GUI);
    gui_add_view_port(gui, viewport, GuiLayerFullscreen);
    while (!atomic_load(&bridge->exiting)) {
        HostRequest request;
        if (furi_message_queue_get(bridge->host_requests, &request, 100) == FuriStatusOk)
            handle_request(bridge, &request);
    }
    stop_card(bridge);
    furi_mutex_acquire(bridge->rpc_lock, FuriWaitForever);
    if (bridge->rpc) {
        if (bridge->exit_ack)
            rpc_system_app_confirm(bridge->rpc, true);
        rpc_system_app_send_exited(bridge->rpc);
        rpc_system_app_set_callback(bridge->rpc, NULL, NULL);
    }
    furi_mutex_release(bridge->rpc_lock);
    gui_remove_view_port(gui, viewport);
    view_port_free(viewport);
    furi_record_close(RECORD_GUI);
    bit_buffer_free(bridge->rx);
    bit_buffer_free(bridge->tx);
    nfc_free(bridge->nfc);
    furi_message_queue_free(bridge->card_requests);
    furi_message_queue_free(bridge->host_requests);
    furi_mutex_free(bridge->rpc_lock);
    free(bridge);
    return 0;
}
