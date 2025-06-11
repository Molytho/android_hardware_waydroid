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


#include <cstddef>
#include <utility>

#include "wrapper.h"
#include "../resource_wrapper.h"

struct display;

namespace wayland {
    class shm_pool {
        unique_fd m_fd {};
        mmap_wrapper m_mmap {};
        wl::shm_pool m_pool {};

        shm_pool(unique_fd fd, mmap_wrapper mmap, wl::shm_pool pool) : m_fd(std::move(fd)), m_mmap(std::move(mmap)), m_pool(std::move(pool)) {}

      public:
        shm_pool() = default;

        shm_pool(shm_pool&&) = default;
        shm_pool &operator=(shm_pool&&) = default;

        int fd() {
            return m_fd;
        }

        void *data() {
            return m_mmap.data();
        }

        size_t size() {
            return m_mmap.size();
        }

        const wl::shm_pool& pool() {
            return m_pool;
        }

        std::tuple<unique_fd, mmap_wrapper, wl::shm_pool> destruct() {
            return { std::move(m_fd), std::move(m_mmap), std::move(m_pool) };
        }

        static shm_pool create(display *display, const char *name, off_t size);

        operator wl_shm_pool *() {
            return m_pool;
        }

        explicit operator bool() const {
            return static_cast<bool>(m_pool);
        }
    };
}