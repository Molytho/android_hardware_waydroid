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

#include <unistd.h>
#include <sys/mman.h>

class unique_fd {
    int m_fd;

  public:
    constexpr unique_fd(int fd = -1) : m_fd(fd) {}
    ~unique_fd() {
        reset();
    }

    void reset(int fd = -1) {
        if (m_fd >= 0) {
            close(m_fd);
        }
        m_fd = fd;
    }

    unique_fd(unique_fd&& other) : m_fd(other.m_fd) {
        other.m_fd = -1;
    }
    unique_fd &operator=(unique_fd&& rhs) {
        reset(rhs.m_fd);
        rhs.m_fd = -1;
        return *this;
    }

    unique_fd &operator=(int fd) {
        reset(fd);
        return *this;
    }

    operator int() const {
        return m_fd;
    }

    explicit operator bool() const {
        return m_fd >= 0;
    }
};


class mmap_wrapper {
    void *m_addr {MAP_FAILED};
    size_t m_size {0};

    constexpr mmap_wrapper(void *addr, size_t size) : m_addr(addr), m_size(size) {}

    void reset(void *addr = nullptr, size_t size = 0) {
        if (m_addr != MAP_FAILED) {
            munmap(m_addr, m_size);
        }
        m_addr = addr;
        m_size = size;
    }

  public:
    mmap_wrapper() = default;
    ~mmap_wrapper() {
        reset();
    }

    mmap_wrapper(mmap_wrapper&& other) : m_addr(other.m_addr), m_size(other.m_size) {
        other.m_addr = MAP_FAILED;
        other.m_size = 0;
    }
    mmap_wrapper &operator=(mmap_wrapper&& rhs) {
        reset(rhs.m_addr, rhs.m_size);
        rhs.m_addr = MAP_FAILED;
        rhs.m_size = 0;
        return *this;
    }

    void *data() {
        return m_addr;
    }
    size_t size() {
        return m_size;
    }

    explicit operator bool() const {
        return m_addr != MAP_FAILED;
    }

    static mmap_wrapper map(void *addr, size_t len, int prot, int flags, int fd, off_t off) {
        void *res = mmap(addr, len, prot, flags, fd, off);
        if (res != MAP_FAILED) {
            return {res, len};
        } else {
            return {};
        }
    }
};