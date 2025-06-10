/*
* Copyright © 2025 Waydroid Project.
*
* Permission is hereby granted, free of charge, to any person obtaining
* a copy of this software and associated documentation files (the
* "Software"), to deal in the Software without restriction, including
* without limitation the rights to use, copy, modify, merge, publish,
* distribute, sublicense, and/or sell copies of the Software, and to
* permit persons to whom the Software is furnished to do so, subject to
* the following conditions:
*
* The above copyright notice and this permission notice (including the
* next paragraph) shall be included in all copies or substantial
* portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
* NONINFRINGEMENT.  IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
* BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
* ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
* CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE.
 */

#pragma once

#include <wayland-client.h>
#include <linux-dmabuf-unstable-v1-client-protocol.h>
#include <viewporter-client-protocol.h>
#include <presentation-time-client-protocol.h>
#include <xdg-shell-client-protocol.h>
#include <tablet-unstable-v2-client-protocol.h>
#include <pointer-constraints-unstable-v1-client-protocol.h>
#include <relative-pointer-unstable-v1-client-protocol.h>
#include <idle-inhibit-unstable-v1-client-protocol.h>
#include <fractional-scale-v1-client-protocol.h>
#include <wayland-android-client-protocol.h>

#define WAYLAND_TYPES_WITH_LISTENERS \
    X(wl, buffer) \
    X(wl, callback) \
    X(wl, data_device) \
    X(wl, data_offer) \
    X(wl, data_source) \
    X(wl, keyboard) \
    X(wl, output) \
    X(wl, pointer) \
    X(wl, registry) \
    X(wl, seat) \
    X(wl, shell_surface) \
    X(wl, shm) \
    X(wl, surface) \
    X(wl, touch) \
    X(wp, fractional_scale_v1) \
    X(wp, presentation) \
    X(wp, presentation_feedback) \
    X(xdg, popup) \
    X(xdg, surface) \
    X(xdg, toplevel) \
    X(xdg, wm_base) \
    X(zwp, confined_pointer_v1) \
    X(zwp, linux_buffer_params_v1) \
    X(zwp, linux_dmabuf_v1) \
    X(zwp, locked_pointer_v1) \
    X(zwp, relative_pointer_v1) \
    X(zwp, tablet_pad_group_v2) \
    X(zwp, tablet_pad_ring_v2) \
    X(zwp, tablet_pad_strip_v2) \
    X(zwp, tablet_pad_v2) \
    X(zwp, tablet_seat_v2) \
    X(zwp, tablet_tool_v2) \
    X(zwp, tablet_v2) \
    X(android, wlegl_server_buffer_handle)

#define WAYLAND_TYPES_WITHOUT_LISTENERS \
    X(wl, compositor) \
    X(wl, data_device_manager) \
    X(wl, display) \
    X(wl, region) \
    X(wl, shell) \
    X(wl, shm_pool) \
    X(wl, subcompositor) \
    X(wl, subsurface) \
    X(wp, fractional_scale_manager_v1) \
    X(wp, viewport) \
    X(wp, viewporter) \
    X(xdg, positioner) \
    X(zwp, idle_inhibit_manager_v1) \
    X(zwp, idle_inhibitor_v1) \
    X(zwp, pointer_constraints_v1) \
    X(zwp, relative_pointer_manager_v1) \
    X(zwp, tablet_manager_v2) \
    X(android, wlegl) \
    X(android, wlegl_handle)

#define WAYLAND_TYPES \
    WAYLAND_TYPES_WITHOUT_LISTENERS \
    WAYLAND_TYPES_WITH_LISTENERS