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

#include "wayland-hwc.h"

class subsurface_cursor_handler : public cursor_handler {
    std::string window_key;

    void clear_previous_subsurface_if_needed(waydroid_hwc_composer_device_1 *pdev);

  public:
    int apply_cursor(waydroid_hwc_composer_device_1 *pdev, hwc_layer_1 *hwc_layer, size_t hwc_layer_index) override;
    int reset_cursor(waydroid_hwc_composer_device_1 *pdev) override;
    int on_cursor_enter(display *display) override;
};

class wl_cursor_cursor_handler : public cursor_handler {
    surface_context cursor_surface_context {};

  public:
    wl_cursor_cursor_handler(display *display);

    std::unique_ptr<buffer> create_buffer(waydroid_hwc_composer_device_1 *pdev, const buffer_metadata& metadata, hwc_layer_1 *hwc_layer) override;
    void set_cursor(display *display);
    int apply_cursor(waydroid_hwc_composer_device_1 *pdev, hwc_layer_1 *hwc_layer, size_t hwc_layer_index) override;
    int reset_cursor(waydroid_hwc_composer_device_1 *pdev) override;
    int on_cursor_enter(display *display) override;
};