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

#include <hardware/hwcomposer.h>
#include <atomic>
#include <string>
#include <unordered_map>
#include <thread>

#include "wayland-hwc.h"
#include "gralloc_handler.h"
#include "resource_wrapper.h"

struct waydroid_mode;

using vsync_clock = std::chrono::steady_clock;

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
    wl_cursor_cursor_handler(waydroid_hwc_composer_device_1 *pdev);

    void set_cursor(display *display);
    int apply_cursor(waydroid_hwc_composer_device_1 *pdev, hwc_layer_1 *hwc_layer, size_t hwc_layer_index) override;
    int reset_cursor(waydroid_hwc_composer_device_1 *pdev) override;
    int on_cursor_enter(display *display) override;
};

struct waydroid_hwc_composer_device_1 : hwc_composer_device_1_t {
    const std::unordered_map<std::string, std::vector<std::string>> blacklisted_apps;
    const gralloc_handler gralloc_handler;
    const std::chrono::nanoseconds vsync_period;
    const bool should_compose;
    const bool multi_windows;

    std::atomic<bool> vsync_callback_enabled;
    std::atomic<vsync_clock::time_point> last_vsync;

    const unique_fd timeline_fd;
    int next_sync_point;
    std::unique_ptr<waydroid_mode> selected_mode;

    const std::unique_ptr<display> display;

    const hwc_procs_t *procs;

    pthread_t vsync_thread;
    pthread_t binder_thread;
    pthread_t egl_worker_thread;

    static std::unique_ptr<waydroid_hwc_composer_device_1> create();

    ~waydroid_hwc_composer_device_1();

  private:
    waydroid_hwc_composer_device_1() = default;
};

int apply_hwc_layer_to_window(waydroid_hwc_composer_device_1 *pdev, hwc_layer_1 *hwc_layer, size_t hwc_layer_index, window *window);
bool is_blacklisted(struct waydroid_hwc_composer_device_1* pdev, const std::string &app_id, const std::string &component);
