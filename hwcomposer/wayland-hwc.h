/*
 * Copyright © 2011 Benjamin Franzke
 * Copyright © 2010 Intel Corporation
 * Copyright © 2014 Collabora Ltd.
 * Copyright © 2021 Waydroid Project.
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

#include <cutils/native_handle.h>

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <stdbool.h>
#include <assert.h>
#include <unistd.h>
#include <sys/mman.h>
#include <signal.h>
#include <fcntl.h>
#include <getopt.h>
#include <errno.h>
#include <map>
#include <list>
#include <set>
#include <pthread.h>
#include <semaphore.h>
#include <hardware/hwcomposer.h>
#include <cutils/properties.h>
#include <vendor/waydroid/task/1.0/IWaydroidTask.h>

#define EGL_EGLEXT_PROTOTYPES
#include <EGL/egl.h>
#include <EGL/eglext.h>

#include <functional>
#include <unordered_map>
#include <unordered_set>

#include <wayland-util.h>
#include "wayland/wrapper.h"

using ::android::sp;
using ::vendor::waydroid::task::V1_0::IWaydroidTask;

namespace wl = ::wayland::wl;
namespace wp = ::wayland::wp;
namespace xdg = ::wayland::xdg;
namespace zwp = ::wayland::zwp;

enum {
    INPUT_TOUCH,
    INPUT_KEYBOARD,
    INPUT_POINTER,
    INPUT_TABLET,
    INPUT_TOTAL
};

static const char *INPUT_PIPE_NAME[INPUT_TOTAL] = {
    "/dev/input/wl_touch_events",
    "/dev/input/wl_keyboard_events",
    "/dev/input/wl_pointer_events",
    "/dev/input/wl_tablet_events"
};

enum class GrallocType {
    GRALLOC_ANDROID,
    GRALLOC_GBM,
    GRALLOC_CROS,
    GRALLOC_DEFAULT
};

#define MAX_TOUCHPOINTS 10

struct layerFrame {
    int x;
    int y;
};

struct waydroid_hwc_composer_device_1;

struct buffer_metadata {
    uint32_t height;
    uint32_t width;
    uint32_t pixel_stride;
    uint32_t format;
};
constexpr bool operator==(const buffer_metadata &lhs, const buffer_metadata &rhs) {
    return lhs.height == rhs.height
           && lhs.width == rhs.width
           && lhs.pixel_stride == rhs.pixel_stride
           && lhs.format == rhs.format;
}
constexpr bool operator!=(const buffer_metadata &lhs, const buffer_metadata &rhs) {
    return lhs.height != rhs.height
           || lhs.width != rhs.width
           || lhs.pixel_stride != rhs.pixel_stride
           || lhs.format != rhs.format;
}

struct buffer {
    wl::buffer<> wl_buffer;

    buffer_handle_t handle;
    buffer_metadata metadata;

    bool isShm;
    void *shm_data;
    int size;

    ~buffer();
};

enum class BufferTransform : int32_t {
    Normal = WL_OUTPUT_TRANSFORM_NORMAL,
    Rot_90 = WL_OUTPUT_TRANSFORM_90,
    Rot_180 = WL_OUTPUT_TRANSFORM_180,
    Rot_270 = WL_OUTPUT_TRANSFORM_270,
    Flip = WL_OUTPUT_TRANSFORM_FLIPPED,
    Flip_Rot_90 = WL_OUTPUT_TRANSFORM_FLIPPED_90,
    Flip_Rot_180 = WL_OUTPUT_TRANSFORM_FLIPPED_180,
    Flip_Rot_270 = WL_OUTPUT_TRANSFORM_FLIPPED_270,
};

BufferTransform hwc_transform_to_buffer_transform(uint32_t hwc_transform);

struct surface_context {
    wl::surface<> surface;
    wp::viewport viewport;

    surface_context() = default;
    surface_context(wl::surface<> surface, wp::viewport viewport);

    surface_context(surface_context &&other);
    surface_context &operator=(surface_context &&rhs);

    void attach_buffer(buffer& buf);
    void damage_surface(int32_t x, int32_t y, int32_t width, int32_t height);
    void set_buffer_transform(BufferTransform transform);
    void set_buffer_scale(double scale);
    // Requires the transformed rectangle
    void set_crop(hwc_frect_t crop);
    // TODO: This should not require scale. Scaling handling is broken
    void set_display_frame(hwc_rect_t rect, double scale);
};

struct window {
    struct layer : public surface_context {
        wl::subsurface subsurface;

        layer(wl::surface<> surface, wp::viewport viewport, wl::subsurface subsurface = {});

        layer(layer &&other);
        layer &operator=(layer &&rhs);

        void set_position(int32_t x, int32_t y);
    };

    struct display *display;

    /* Used for the background color */
    // TODO: Make wayland object wrappers usable here
    bool destroy_background_objects;
    struct wl_surface *surface;
    struct wp_viewport *viewport;
    wl::buffer<> bg_buffer;

    std::vector<layer> layers;

    wl::shell_surface<std::weak_ptr<window>> shell_surface;
    xdg::surface<std::weak_ptr<window>> xdg_surface;
    xdg::toplevel<std::weak_ptr<window>> xdg_toplevel;

    zwp::locked_pointer_v1<> locked_pointer;
    zwp::idle_inhibitor_v1 idle_inhibitor;

    std::unique_ptr<buffer> snapshot_buffer;

    std::string appID;
    std::string taskID;

    std::atomic<bool> configured;

    // Reset every hwc_set cycle
    wl::region input_region;
    int lastLayer;
    struct buffer *last_layer_buffer;

    ~window();

    static std::shared_ptr<window> create(struct display *display, std::string appID, std::string taskID, hwc_color_t color);

    window::layer &get_next_layer();
    window::layer &create_new_layer();
    void reset_per_set_state();

    void minimize();
    void set_maximize(bool enabled);
    void set_title(const char *title);
    void set_app_id(std::string appID);

  private:
    window() = default;
};

class open_windows {
    using Collection = std::map<std::string, std::shared_ptr<window>>;
    Collection windows;

  public:
    using key_type = Collection::key_type;
    using mapped_type = Collection::mapped_type;
    using size_type = Collection::size_type;
    using iterator = Collection::iterator;
    using const_iterator = Collection::const_iterator;
    using reverse_iterator = Collection::reverse_iterator;
    using const_reverse_iterator = Collection::const_reverse_iterator;

    iterator begin() {
        return windows.begin();
    }
    iterator end() {
        return windows.end();
    }

    iterator find(const key_type& key) {
        return windows.find(key);
    }

    mapped_type& operator[](const key_type& key) {
        return windows[key];
    }
    mapped_type& operator[](key_type&& key) {
        return windows[std::move(key)];
    }

    size_type size() const {
        return windows.size();
    }

    template<class Func>
    void update(Func func) {
        func();

        std::string windows_size_str = std::to_string(windows.size());
        property_set("waydroid.open_windows", windows_size_str.c_str());
    }

    window *add(waydroid_hwc_composer_device_1 *pdev, const std::string& key, const std::string& aid, const std::string& tid, hwc_color_t color = {0, 0, 0, 255});
    void add(const std::string& key, std::shared_ptr<window> window);
    void clear();
    void erase(const_iterator pos);
    void erase(const key_type& key);
    template<class Pred>
    void erase_if(Pred pred) {
        update([&](){
            for (auto it = windows.begin(), end = windows.end(); it != end;) {
                if (pred(*it)) {
                    it = windows.erase(it);
                } else {
                    ++it;
                }
            }
        });
    }
};

struct cursor_handler {
    virtual ~cursor_handler() = default;
    virtual std::unique_ptr<buffer> create_buffer(waydroid_hwc_composer_device_1 *pdev, const buffer_metadata& metadata, hwc_layer_1 *hwc_layer);
    virtual int apply_cursor(waydroid_hwc_composer_device_1 *pdev, hwc_layer_1 *hwc_layer, size_t hwc_layer_index) = 0;
    virtual int reset_cursor(waydroid_hwc_composer_device_1 *pdev) = 0;
    virtual int on_cursor_enter(display *display) = 0;
};

struct display {
    pthread_t wayland_thread; // constant after init

    wl::display wl_display;
    wl::registry<struct display *> registry;
    wl::compositor compositor;
    wl::subcompositor subcompositor;
    wl::seat<struct display *> seat;
    wl::shell shell;
    wl::shm<> shm;
    wl::pointer<struct display *> pointer;
    wl::keyboard<struct display *> keyboard;
    wl::touch<struct display *> touch;
    wl::output<struct display *> output;
    wp::presentation<> presentation;
    wp::viewporter viewporter;
    wayland::android::wlegl android_wlegl;
    zwp::linux_dmabuf_v1<struct display *> dmabuf;
    xdg::wm_base<struct display *> wm_base;
    zwp::tablet_manager_v2 tablet_manager;
    zwp::tablet_seat_v2<struct display *> tablet_seat;
    zwp::pointer_constraints_v1 pointer_constraints;
    zwp::relative_pointer_manager_v1 relative_pointer_manager;
    zwp::relative_pointer_v1<struct display *> relative_pointer;
    zwp::idle_inhibit_manager_v1 idle_manager;
    wp::fractional_scale_manager_v1 fractional_scale_manager;
    wl::data_device_manager data_device_manager;
    wl::data_device<> data_device;

    wayland::user_data_repository user_data_repository;

    int system_version;
    GrallocType gtype;
    double scale;

    int input_fd[INPUT_TOTAL];
    int ptrPrvX;
    int ptrPrvY;
    double wheelAccumulatorX;
    double wheelAccumulatorY;
    bool wheelEvtIsDiscrete;
    bool reverseScroll;
    int touch_id[MAX_TOUCHPOINTS];
    std::map<struct wl_surface *, struct layerFrame> layers;

    open_windows windows;
    std::set<std::string> ignored_apps;
    std::recursive_mutex windowsMutex;

    std::map<int, struct wl_surface *> touch_surfaces;
    struct wl_surface *pointer_surface;
    struct wl_surface *tablet_surface;
    std::list<zwp::tablet_tool_v2<struct display *>> tablet_tools;
    std::map<struct zwp_tablet_tool_v2 *, uint16_t> tablet_tools_evt;
    uint32_t keyboard_enter_serial;
    std::string clipboard;
    std::list<std::string> clipboard_offer_mime_types;
    uint32_t pointer_enter_serial;
    struct {float x; float y;} cursor_hotspot;

    EGLDisplay egl_dpy;
    std::list<std::function<void()>> egl_work_queue;
    sem_t egl_go;
    sem_t egl_done;

    int width;
    int height;
    int full_width;
    int full_height;
    int req_width;
    int req_height;
    int refresh;

    std::unordered_set<uint32_t> formats;

    std::map<uint32_t, std::vector<uint64_t>> modifiers;
    std::map<uint32_t, std::string> layer_names;
    std::map<uint32_t, buffer_metadata> layer_handles_ext;
    buffer_metadata target_layer_handle_ext;
    std::unordered_map<buffer_handle_t, std::unique_ptr<buffer>> buffer_map;
    std::array<uint8_t, 239> keysDown;

    std::unique_ptr<cursor_handler> cursor_handler;
    bool supports_cursor_viewport;
    bool supports_cursor_hw_buffer;

    bool isMaximized;
    sp<IWaydroidTask> task;

    ~display();
};

void
handle_relative_motion(void *data, struct zwp_relative_pointer_v1*,
        uint32_t, uint32_t, wl_fixed_t dx, wl_fixed_t dy, wl_fixed_t, wl_fixed_t);

void
snapshot_inactive_app_window(struct display *display, struct window *window);

struct display *
create_display(const char* gralloc);
