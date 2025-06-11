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

#include "shm_pool.h"

#include <sys/syscall.h>
#include <log/log.h>

#include "../wayland-hwc.h"

namespace wayland {
    shm_pool shm_pool::create(display *display, const char *name, off_t size) {
        unique_fd fd = static_cast<int>(syscall(SYS_memfd_create, name, MFD_ALLOW_SEALING));
        if (!fd) {
            ALOGE("memfd_create failed with %u: %s", errno, strerror(errno));
            return {};
        }

        int res = ftruncate(fd, size);
        if (res != 0) {
            ALOGE("ftruncate failed with %u: %s", errno, strerror(errno));
            return {};
        }

        mmap_wrapper mmap = mmap_wrapper::map(nullptr, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
        if (!mmap) {
            ALOGE("mmap failed with %u: %s", errno, strerror(errno));
            return {};
        }

        wl::shm_pool pool = wl_shm_create_pool(display->shm, fd, size);
        if (!pool) {
            ALOGE("wl_shm_create_pool failed with %u: %s", errno, strerror(errno));
            return {};
        }

        return { std::move(fd), std::move(mmap), std::move(pool) };
    }
}