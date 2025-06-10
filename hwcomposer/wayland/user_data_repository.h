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

#include <mutex>
#include <memory>
#include <unordered_map>

#include <log/log.h>

struct display;
struct waydroid_hwc_composer_device_1;

namespace wayland {
    class user_data_repository {
        std::mutex lock;
        std::unordered_map<uint32_t, std::unique_ptr<void, void (*)(void *)>> user_data_map;

      public:
        template <class T, class... Args>
        void add_user_data(uint32_t id, Args&&... args) {
            std::lock_guard guard(lock);
            std::unique_ptr<void, void (*)(void *)> ptr {
                new T(std::forward<Args>(args)...),
                [](void *ptr) {
                    delete static_cast<T*>(ptr);
                }
            };
            auto res = user_data_map.emplace(id, std::move(ptr));
            if (!res.second) {
                ALOGE("user_data_repository::add_user_data called for already existing id");
                abort();
            }
        }

        void remove_user_data(uint32_t id) {
            std::lock_guard guard(lock);
            auto res = user_data_map.erase(id);
            if (res == 0) {
                ALOGE("user_data_repository::remove_user_data called for non-existent id");
                abort();
            }
        }

        template <class T>
        T get_as(uint32_t id) {
            std::lock_guard guard(lock);
            auto it = user_data_map.find(id);
            if (it != user_data_map.end()) {
                return *static_cast<T *>(it->second.get());
            } else {
                return {};
            }
        }
    };

    namespace detail {
        template<class T>
        struct user_data_outlives_wayland : std::false_type {};
        template<>
        struct user_data_outlives_wayland<display *> : std::true_type {};
        template<>
        struct user_data_outlives_wayland<waydroid_hwc_composer_device_1 *> : std::true_type {};

        template<class T, bool Outlives = user_data_outlives_wayland<T>::value>
        struct user_data_ops {
            static void *register_user_data(user_data_repository& repo, uint32_t id, T&& obj) {
                repo.add_user_data<T>(id, std::forward<T>(obj));
                return std::addressof(repo);
            }
            static T get_user_data(void *user_data_ptr, uint32_t id) {
                if (!user_data_ptr) {
                    return {};
                }

                auto repo = static_cast<user_data_repository *>(user_data_ptr);
                return repo->get_as<T>(id);
            }
            static void unregister_user_data(void *user_data_ptr, uint32_t id) {
                if (user_data_ptr) {
                    auto repo = static_cast<user_data_repository *>(user_data_ptr);
                    repo->remove_user_data(id);
                }
            }
        };
        template<class T>
        struct user_data_ops<T, true> {
            static void *register_user_data(user_data_repository&, uint32_t, T&& obj) {
                return obj;
            }
            static T get_user_data(void *user_data_ptr, uint32_t) {
                return static_cast<T>(user_data_ptr);
            }
            static void unregister_user_data(void *, uint32_t) {}
        };
    };

    template<class T>
    inline void *register_user_data(user_data_repository& repo, uint32_t id, T&& obj) {
        return detail::user_data_ops<T>::register_user_data(repo, id, std::forward<T>(obj));
    }
    template<class T>
    inline T get_user_data(void *user_data_ptr, void *obj) {
        return detail::user_data_ops<T>::get_user_data(user_data_ptr, wl_proxy_get_id(reinterpret_cast<wl_proxy *>(obj)));
    }
    template<class T>
    inline void unregister_user_data(void *user_data_ptr, uint32_t id) {
        detail::user_data_ops<T>::unregister_user_data(user_data_ptr, id);
    }
}
