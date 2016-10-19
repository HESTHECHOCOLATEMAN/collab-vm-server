/*
 * Copyright (C) 2014 Glyptodon LLC
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

#include "guac_cursor.h"
#include "guac_display.h"
#include "guac_surface.h"

#include <guacamole/client.h>
#include <guacamole/socket.h>

#include <stdlib.h>
#include <string.h>

/**
 * Synchronizes all surfaces within the given array to the given socket. If a
 * given surface is set to NULL, it will not be synchronized.
 *
 * @param layers The array of layers to free.
 * @param count The number of layers in the array.
 * @param socket The socket to synchronize each layer to.
 */
static void guac_common_display_dup_layers(guac_common_display_layer* layers,
        int count, GuacSocket& socket) {

    int i;

    /* Synchronize all surfaces in given array */
    for (i=0; i < count; i++) {

        guac_common_display_layer* current = layers++;

        /* Synchronize surface, if present */
        if (current->surface != NULL)
            guac_common_surface_dup(current->surface, socket);

    }

}

/**
 * Frees all layers and associated surfaces within the given array.
 *
 * @param layers The array of layers to synchronize.
 * @param count The number of layers in the array.
 *
 * @param client
 *     The client owning the layers wrapped by each of the layers in the array.
 */
static void guac_common_display_free_layers(guac_common_display_layer* layers,
        int count, GuacClient& client) {

    int i;

    /* Free each surface in given array */
    for (i=0; i < count; i++) {

        guac_common_display_layer* current = layers++;

        /* Free layer, if present */
        if (current->layer != NULL) {
			if (current->layer->index >= 0)
				client.FreeBuffer(current->layer);
            else
                client.FreeLayer(current->layer);
        }

        /* Free surface, if present */
        if (current->surface != NULL)
            guac_common_surface_free(current->surface);

    }

}

guac_common_display* guac_common_display_alloc(GuacClient& client,
        int width, int height)
{
    /* Allocate display */
    guac_common_display* display = new guac_common_display(client, width, height);
    if (display == NULL)
        return NULL;

    return display;
}

void guac_common_display_free(guac_common_display* display) {

    /* Free shared cursor */
    guac_common_cursor_free(display->cursor);

    /* Free default surface */
    guac_common_surface_free(display->default_surface);

    /* Synchronize all layers/buffers */
    guac_common_display_free_layers(display->buffers, display->buffers_size, display->client);
    guac_common_display_free_layers(display->layers, display->layers_size, display->client);

    free(display->buffers);
    free(display->layers);

    free(display);

}

void guac_common_display_dup(guac_common_display* display,
        GuacSocket& socket) {

    /* Sunchronize shared cursor */
    guac_common_cursor_dup(display->cursor, socket);

    /* Synchronize default surface */
    guac_common_surface_dup(display->default_surface, socket);

    /* Synchronize all layers/buffers */
    guac_common_display_dup_layers(display->layers, display->layers_size, socket);
    guac_common_display_dup_layers(display->buffers, display->buffers_size, socket);

}

/**
 * Returns a pointer to the layer having the given index within the given
 * layers array, reallocating and resizing that array if necessary. The resized
 * array and its new size will be returned through the provided pointers.
 *
 * @param layers_ptr Pointer to the layers array.
 *
 * @param size_ptr
 *     Pointer to an integer containing the number of entries in the layers
 *     array.
 *
 * @param index The array index of the layer to return.
 * @return The layer at the given index within the layer array.
 */
static guac_common_display_layer* guac_common_display_get_layer(
        guac_common_display_layer** layers_ptr, int* size_ptr, int index) {

    guac_common_display_layer* layers = *layers_ptr;
    int size = *size_ptr;

    /* Resize layers array if it's not big enough */
    if (index >= size) {

        int new_size;

        /* Resize layers array */
        *size_ptr = new_size = index*2;
        *layers_ptr = layers = (guac_common_display_layer*)
            realloc(layers, new_size * sizeof(guac_common_display_layer));

        /* Clear newly-allocated space */
        memset(layers + size, 0, new_size - size);

    }

    /* Return reference to requested layer */
    return layers + index;

}

void guac_common_display_flush(guac_common_display* display) {
    guac_common_surface_flush(display->default_surface);
}

guac_common_display_layer* guac_common_display_alloc_layer(
        guac_common_display* display, int width, int height) {

    guac_layer* layer;
    guac_common_display_layer* display_layer;

    /* Allocate Guacamole layer */
    layer = display->client.AllocLayer();

    /* Get slot for allocated layer */
    display_layer = guac_common_display_get_layer(
            &display->layers, &display->layers_size,
            layer->index - 1);

    /* Init display layer */
    display_layer->layer = layer;
    display_layer->surface = guac_common_surface_alloc(display->client.broadcast_socket_,
            layer, width, height);

    return display_layer;
}

guac_common_display_layer* guac_common_display_alloc_buffer(
        guac_common_display* display, int width, int height) {

    guac_layer* buffer;
    guac_common_display_layer* display_buffer;

    /* Allocate Guacamole buffer */
    buffer = display->client.AllocBuffer();

    /* Get slot for allocated buffer */
    display_buffer = guac_common_display_get_layer(
            &display->buffers, &display->buffers_size,
            -1 - buffer->index);

    /* Init display buffer */
    display_buffer->layer = buffer;
    display_buffer->surface = guac_common_surface_alloc(display->client.broadcast_socket_,
            buffer, width, height);

    return display_buffer;
}

void guac_common_display_free_layer(guac_common_display* display,
        guac_common_display_layer* layer) {

    /* Free associated layer and surface */
    display->client.FreeLayer(layer->layer);
    guac_common_surface_free(layer->surface);

    layer->layer = NULL;
    layer->surface = NULL;

}

void guac_common_display_free_buffer(guac_common_display* display,
        guac_common_display_layer* buffer) {

    /* Free associated layer and surface */
    display->client.FreeBuffer(buffer->layer);
    guac_common_surface_free(buffer->surface);

    buffer->layer = NULL;
    buffer->surface = NULL;

}

