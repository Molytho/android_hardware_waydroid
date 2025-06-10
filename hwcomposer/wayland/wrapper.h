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

#include <type_traits>

#include "wayland_types_helper.h"
#include "user_data_repository.h"

#define DEFINE_DELETE_FUNC(type)            \
    inline void delete_wayland_object(struct type *ptr) { \
        type##_destroy(ptr);                \
    }

#define DEFINE_LISTENER_TYPE_HELPER(name) \
   template<>                             \
   struct listener_type_helper<struct name> : std::true_type { \
        using listener_type = struct name##_listener; \
        static int add_listener(struct name *obj, const listener_type& listener, void *data) { \
            return name##_add_listener(obj, &listener, data); \
        } \
   };

#define DEFINE_TYPE_ALIAS_WITH_LISTENER(ns, name) \
    namespace ns { \
        template<class T = void> \
        using name = detail::object_wrapper<struct ns##_##name, T>; \
    }

#define DEFINE_TYPE_ALIAS_WITHOUT_LISTENER(ns, name) \
    namespace ns { \
        using name = detail::object_wrapper<struct ns##_##name, void>; \
    }

namespace wayland {
    namespace detail {
        /* ---------- Delete helper ---------- */
        // For wl_display this is normally called "wl_display_disconnect".
        // Define wl_display_destroy here so that our macro works
        inline void wl_display_destroy(struct wl_display *ptr) {
            wl_display_flush(ptr);
            wl_display_disconnect(ptr);
        }

        #define X(x, y) DEFINE_DELETE_FUNC(x##_##y)
        WAYLAND_TYPES
        #undef X

        /* ---------- Listener helper ----------- */
        template<class T>
        struct listener_type_helper : std::false_type {};

        #define X(x, y) DEFINE_LISTENER_TYPE_HELPER(x##_##y)
        WAYLAND_TYPES_WITH_LISTENERS
        #undef X

        /* ---------- Object wrapper ----------- */
        template<class T, class UserData>
        class remove_impl;

        template<class T, class UserData = void, bool HasListener = listener_type_helper<T>::value>
        class add_listener_impl;


        template<class T, class UserData>
        class object_wrapper : remove_impl<T, UserData>, public add_listener_impl<T, UserData> {
            friend remove_impl<T, UserData>;
            friend add_listener_impl<T, UserData>;

            T *m_obj;

          public:
            constexpr object_wrapper(T *obj = nullptr) : m_obj{obj} { }
            ~object_wrapper() {
                reset();
            }

            object_wrapper(object_wrapper &&other) : m_obj{other.m_obj} {
                other.m_obj = nullptr;
            }
            object_wrapper &operator=(object_wrapper &&rhs) {
                reset(rhs.m_obj);
                rhs.m_obj = nullptr;
                return *this;
            }

            object_wrapper &operator=(T *rhs) {
                reset(rhs);
                return *this;
            }

            using remove_impl<T, UserData>::reset;

            constexpr operator T*() {
                assert(m_obj);
                return m_obj;
            }
            constexpr operator const T*() const {
                assert(m_obj);
                return m_obj;
            }

            wl_proxy *proxy() {
                return reinterpret_cast<wl_proxy*>(m_obj);
            }
        };


        template<class T, class UserData>
        class remove_impl {
            using Derived = object_wrapper<T, UserData>;

            Derived *derived() {
                return static_cast<Derived *>(this);
            }

          public:
            void reset(T *obj = nullptr) {
                if (derived()->m_obj) {
                    unregister_user_data<UserData>(wl_proxy_get_user_data(derived()->proxy()), wl_proxy_get_id(derived()->proxy()));
                    delete_wayland_object(derived()->m_obj);
                }
                derived()->m_obj = obj;
            }
        };

        template<class T>
        class remove_impl<T, void> {
            using Derived = object_wrapper<T, void>;

            Derived *derived() {
                return static_cast<Derived *>(this);
            }

          public:
            void reset(T *obj = nullptr) {
                if (derived()->m_obj) {
                    delete_wayland_object(derived()->m_obj);
                }
                derived()->m_obj = obj;
            }
        };


        template<class T, class UserData, bool HasListener>
        class add_listener_impl {
            static_assert(std::is_void<UserData>::value);
        };

        // No user data specialization
        template<class T>
        class add_listener_impl<T, void, true> {
            using Derived = object_wrapper<T, void>;
            using listener_helper = listener_type_helper<T>;
            using listener_type = typename listener_helper::listener_type;

            Derived *derived() {
                return static_cast<Derived *>(this);
            }

          public:
            void add_listener(const listener_type& listener) {
                if (listener_helper::add_listener(derived()->m_obj, listener, nullptr) != 0) {
                    ALOGE("Failed to add listener to wl object");
                    abort();
                }
            }
        };

        // With user data specialization
        template<class T, class UserData>
        class add_listener_impl<T, UserData, true> {
            using Derived = object_wrapper<T, UserData>;
            using listener_helper = listener_type_helper<T>;
            using listener_type = typename listener_helper::listener_type;

            Derived *derived() {
                return static_cast<Derived *>(this);
            }

          public:
            void add_listener(const listener_type& listener, user_data_repository& repo, UserData data) {
                void *user_data = register_user_data<UserData>(repo, wl_proxy_get_id(derived()->proxy()), std::move(data));
                if (listener_helper::add_listener(derived()->m_obj, listener, user_data) != 0) {
                    ALOGE("Failed to add listener to wl object");
                    abort();
                }
            }
        };
    }

    #define X(x, y) DEFINE_TYPE_ALIAS_WITHOUT_LISTENER(x, y)
    WAYLAND_TYPES_WITHOUT_LISTENERS
    #undef X

    #define X(x, y) DEFINE_TYPE_ALIAS_WITH_LISTENER(x, y)
    WAYLAND_TYPES_WITH_LISTENERS
    #undef X
}

#undef DEFINE_DELETE_FUNC
#undef DEFINE_LISTENER_TYPE_HELPER
#undef DEFINE_TYPE_ALIAS_WITHOUT_LISTENER
#undef DEFINE_TYPE_ALIAS_WITH_LISTENER