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

#include "cursor_handler.h"
#include "hwcomposer.h"

void subsurface_cursor_handler::clear_previous_subsurface_if_needed(waydroid_hwc_composer_device_1 *pdev) {
    /* If should_compose is set, remaining subsurface are cleared in post_processing.
     * In this case we can skip it here */
    if (!pdev->should_compose && !window_key.empty()) {
        auto window_it = pdev->display->windows.find(window_key);
        if (window_it == pdev->display->windows.end()) {
            // Window was closed this hwc_set
            return;
        }

        assert(window_it->second->layers.size() == 2);
        auto &last_layer = window_it->second->layers[window_it->second->layers.size() - 1];
        wl_surface_attach(last_layer.surface, nullptr, 0, 0);
        wl_surface_commit(last_layer.surface);
    }
    window_key = {};
}

int subsurface_cursor_handler::apply_cursor(waydroid_hwc_composer_device_1* pdev, hwc_layer_1* hwc_layer, size_t hwc_layer_index) {
    if (!pdev->display->pointer_surface) {
        if (hwc_layer->acquireFenceFd != -1) {
            close(hwc_layer->acquireFenceFd);
        }
        clear_previous_subsurface_if_needed(pdev);
        return 0;
    }

    auto window_it = std::find_if(pdev->display->windows.begin(), pdev->display->windows.end(), [&](const auto &it){
        auto &window = it.second;
        return window->surface == pdev->display->pointer_surface
               || std::any_of(window->layers.begin(), window->layers.end(), [&](const auto &layer) {
                      return layer.surface == pdev->display->pointer_surface;
                  });
    });
    if (window_it == pdev->display->windows.end()) {
        if (hwc_layer->acquireFenceFd != -1) {
            close(hwc_layer->acquireFenceFd);
        }
        clear_previous_subsurface_if_needed(pdev);
        return 0;
    }


    int res = apply_hwc_layer_to_window(pdev, hwc_layer, hwc_layer_index, window_it->second.get());
    if (res == 0) {
        window_key = window_it->first;
        return 0;
    } else {
        window_key = {};
        return res;
    }
}

int subsurface_cursor_handler::reset_cursor(waydroid_hwc_composer_device_1* pdev) {
    clear_previous_subsurface_if_needed(pdev);
    return 0;
}

int subsurface_cursor_handler::on_cursor_enter(display* display) {
    if (display->pointer) {
        wl_pointer_set_cursor(display->pointer, display->pointer_enter_serial,
                              nullptr,
                              0,
                              0);
    }
    return 0;
}


wl_cursor_cursor_handler::wl_cursor_cursor_handler(display *display) {
    cursor_surface_context.surface = wl_compositor_create_surface(display->compositor);
    if (display->viewporter && display->supports_cursor_viewport) {
        cursor_surface_context.viewport =
                wp_viewporter_get_viewport(display->viewporter, cursor_surface_context.surface);
    }
}

void wl_cursor_cursor_handler::set_cursor(display* display) {
    assert(display->pointer);
    wl_pointer_set_cursor (display->pointer, display->pointer_enter_serial,
                          cursor_surface_context.surface,
                          round(display->cursor_hotspot.x / display->scale),
                          round(display->cursor_hotspot.y / display->scale));
}

int wl_cursor_cursor_handler::apply_cursor(waydroid_hwc_composer_device_1* pdev, hwc_layer_1* hwc_layer, size_t hwc_layer_index) {
    if (pdev->display->pointer) {
        if (apply_hwc_layer_to_surface_context(pdev, hwc_layer, hwc_layer_index, cursor_surface_context) != 0) {
            ALOGE("Failed to prepare cursur surface");
            return -1;
        }
        set_cursor(pdev->display.get());
    }
    return 0;
}

int wl_cursor_cursor_handler::reset_cursor(waydroid_hwc_composer_device_1* pdev) {
    if (pdev->display->pointer) {
        wl_pointer_set_cursor(pdev->display->pointer, pdev->display->pointer_enter_serial,
                              nullptr,
                              0,
                              0);
    }
    return 0;
}

int wl_cursor_cursor_handler::on_cursor_enter(display* display) {
    set_cursor(display);
    return 0;
}