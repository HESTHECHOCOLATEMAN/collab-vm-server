/*
 * Copyright (C) 2013 Glyptodon LLC
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#include "config.h"

#include "GuacClient.h"
#include "protocol.h"
#include "stream.h"
#include "timestamp.h"
#include "GuacUser.h"
#include "user-handlers.h"
#include "user-constants.h"

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

/* Guacamole instruction handler map */

__guac_instruction_handler_mapping __guac_instruction_handler_map[] = {
   {"sync",       __guac_handle_sync},
   {"mouse",      __guac_handle_mouse},
   {"key",        __guac_handle_key},
   {"clipboard",  __guac_handle_clipboard},
   {"disconnect", __guac_handle_disconnect},
   {"size",       __guac_handle_size},
   {"file",       __guac_handle_file},
   {"pipe",       __guac_handle_pipe},
   {"ack",        __guac_handle_ack},
   {"blob",       __guac_handle_blob},
   {"end",        __guac_handle_end},
   {NULL,         NULL}
};

int64_t __guac_parse_int(const char* str) {

    int sign = 1;
    int64_t num = 0;

    for (; *str != '\0'; str++) {

        if (*str == '-')
            sign = -sign;
        else
            num = num * 10 + (*str - '0');

    }

    return num * sign;

}

/* Guacamole instruction handlers */

int __guac_handle_sync(GuacUser* user, int argc, char** argv) {

    int frame_duration;

    guac_timestamp current = guac_timestamp_current();
    guac_timestamp timestamp = __guac_parse_int(argv[0]);

    /* Error if timestamp is in future */
    if (timestamp > user->client_->last_sent_timestamp)
        return -1;

    /* Update stored timestamp */
    user->last_received_timestamp = timestamp;

    /* Calculate length of frame, including network and processing lag */
    frame_duration = current - timestamp;

    /* Update lag statistics if at least one frame has been rendered */
    if (user->last_frame_duration != 0) {

        /* Approximate processing lag by summing the frame duration deltas */
        int processing_lag = user->processing_lag + frame_duration
                           - user->last_frame_duration;

        /* Adjust back to zero if cumulative error leads to a negative value */
        if (processing_lag < 0)
            processing_lag = 0;

        user->processing_lag = processing_lag;

    }

    /* Record duration of frame */
    user->last_frame_duration = frame_duration;

    if (user->sync_handler)
        return user->sync_handler(user, timestamp);
    return 0;
}

int __guac_handle_mouse(GuacUser* user, int argc, char** argv) {
    if (user->mouse_handler)
        return user->mouse_handler(
            user,
            atoi(argv[0]), /* x */
            atoi(argv[1]), /* y */
            atoi(argv[2])  /* mask */
        );
    return 0;
}

int __guac_handle_key(GuacUser* user, int argc, char** argv) {
    if (user->key_handler)
        return user->key_handler(
            user,
            atoi(argv[0]), /* keysym */
            atoi(argv[1])  /* pressed */
        );
    return 0;
}

static guac_stream* __get_input_stream(GuacUser* user, int stream_index) {

    /* Validate stream index */
    if (stream_index < 0 || stream_index >= GUAC_USER_MAX_STREAMS) {

        guac_stream dummy_stream;
        dummy_stream.index = stream_index;

        guac_protocol_send_ack(user->socket_, &dummy_stream,
                "Invalid stream index", GUAC_PROTOCOL_STATUS_CLIENT_BAD_REQUEST);
        return NULL;
    }

    return &(user->__input_streams[stream_index]);

}

static guac_stream* __get_open_input_stream(GuacUser* user, int stream_index) {

    guac_stream* stream = __get_input_stream(user, stream_index);

    /* Fail if no such stream */
    if (stream == NULL)
        return NULL;

    /* Validate initialization of stream */
    if (stream->index == GUAC_USER_CLOSED_STREAM_INDEX) {

        guac_stream dummy_stream;
        dummy_stream.index = stream_index;

        guac_protocol_send_ack(user->socket_, &dummy_stream,
                "Invalid stream index", GUAC_PROTOCOL_STATUS_CLIENT_BAD_REQUEST);
        return NULL;
    }

    return stream;

}

static guac_stream* __init_input_stream(GuacUser* user, int stream_index) {

    guac_stream* stream = __get_input_stream(user, stream_index);

    /* Fail if no such stream */
    if (stream == NULL)
        return NULL;

    /* Initialize stream */
    stream->index = stream_index;
    stream->data = NULL;
    stream->ack_handler = NULL;
    stream->blob_handler = NULL;
    stream->end_handler = NULL;

    return stream;

}

int __guac_handle_clipboard(GuacUser* user, int argc, char** argv) {

    /* Pull corresponding stream */
    int stream_index = atoi(argv[0]);
    guac_stream* stream = __init_input_stream(user, stream_index);
    if (stream == NULL)
        return 0;

    /* If supported, call handler */
    if (user->clipboard_handler)
        return user->clipboard_handler(
            user,
            stream,
            argv[1] /* mimetype */
        );

    /* Otherwise, abort */
    guac_protocol_send_ack(user->socket_, stream,
            "Clipboard unsupported", GUAC_PROTOCOL_STATUS_UNSUPPORTED);
    return 0;

}

int __guac_handle_size(GuacUser* user, int argc, char** argv) {
    if (user->size_handler)
        return user->size_handler(
            user,
            atoi(argv[0]), /* width */
            atoi(argv[1])  /* height */
        );
    return 0;
}

int __guac_handle_file(GuacUser* user, int argc, char** argv) {

    /* Pull corresponding stream */
    int stream_index = atoi(argv[0]);
    guac_stream* stream = __init_input_stream(user, stream_index);
    if (stream == NULL)
        return 0;

    /* If supported, call handler */
    if (user->file_handler)
        return user->file_handler(
            user,
            stream,
            argv[1], /* mimetype */
            argv[2]  /* filename */
        );

    /* Otherwise, abort */
    guac_protocol_send_ack(user->socket_, stream,
            "File transfer unsupported", GUAC_PROTOCOL_STATUS_UNSUPPORTED);
    return 0;
}

int __guac_handle_pipe(GuacUser* user, int argc, char** argv) {

    /* Pull corresponding stream */
    int stream_index = atoi(argv[0]);
    guac_stream* stream = __init_input_stream(user, stream_index);
    if (stream == NULL)
        return 0;

    /* If supported, call handler */
    if (user->pipe_handler)
        return user->pipe_handler(
            user,
            stream,
            argv[1], /* mimetype */
            argv[2]  /* name */
        );

    /* Otherwise, abort */
    guac_protocol_send_ack(user->socket_, stream,
            "Named pipes unsupported", GUAC_PROTOCOL_STATUS_UNSUPPORTED);
    return 0;
}

int __guac_handle_ack(GuacUser* user, int argc, char** argv) {

    guac_stream* stream;

    /* Validate stream index */
    int stream_index = atoi(argv[0]);
    if (stream_index < 0 || stream_index >= GUAC_USER_MAX_STREAMS)
        return 0;

    stream = &(user->__output_streams[stream_index]);

    /* Validate initialization of stream */
    if (stream->index == GUAC_USER_CLOSED_STREAM_INDEX)
        return 0;

    /* Call stream handler if defined */
    if (stream->ack_handler)
        return stream->ack_handler(user, stream, argv[1],
                (guac_protocol_status)atoi(argv[2]));

    /* Fall back to global handler if defined */
    if (user->ack_handler)
        return user->ack_handler(user, stream, argv[1],
			(guac_protocol_status)atoi(argv[2]));

    return 0;
}

int __guac_handle_blob(GuacUser* user, int argc, char** argv) {

    int stream_index = atoi(argv[0]);
    guac_stream* stream = __get_open_input_stream(user, stream_index);

    /* Fail if no such stream */
    if (stream == NULL)
        return 0;

    /* Call stream handler if defined */
    if (stream->blob_handler) {
        int length = guac_protocol_decode_base64(argv[1]);
        return stream->blob_handler(user, stream, argv[1],
            length);
    }

    /* Fall back to global handler if defined */
    if (user->blob_handler) {
        int length = guac_protocol_decode_base64(argv[1]);
        return user->blob_handler(user, stream, argv[1],
            length);
    }

    guac_protocol_send_ack(user->socket_, stream,
            "File transfer unsupported", GUAC_PROTOCOL_STATUS_UNSUPPORTED);
    return 0;
}

int __guac_handle_end(GuacUser* user, int argc, char** argv) {

    int result = 0;
    int stream_index = atoi(argv[0]);
    guac_stream* stream = __get_open_input_stream(user, stream_index);

    /* Fail if no such stream */
    if (stream == NULL)
        return 0;

    /* Call stream handler if defined */
    if (stream->end_handler)
        result = stream->end_handler(user, stream);

    /* Fall back to global handler if defined */
    if (user->end_handler)
        result = user->end_handler(user, stream);

    /* Mark stream as closed */
    stream->index = GUAC_USER_CLOSED_STREAM_INDEX;
    return result;
}

int __guac_handle_disconnect(GuacUser* user, int argc, char** argv) {
    guac_user_stop(user);
    return 0;
}

