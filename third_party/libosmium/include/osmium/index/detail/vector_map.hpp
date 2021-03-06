#ifndef OSMIUM_INDEX_DETAIL_VECTOR_MAP_HPP
#define OSMIUM_INDEX_DETAIL_VECTOR_MAP_HPP

/*

This file is part of Osmium (http://osmcode.org/libosmium).

Copyright 2013-2016 Jochen Topf <jochen@topf.org> and others (see README).

Boost Software License - Version 1.0 - August 17th, 2003

Permission is hereby granted, free of charge, to any person or organization
obtaining a copy of the software and accompanying documentation covered by
this license (the "Software") to use, reproduce, display, distribute,
execute, and transmit the Software, and to prepare derivative works of the
Software, and to permit third-parties to whom the Software is furnished to
do so, all subject to the following:

The copyright notices in the Software and this entire statement, including
the above license grant, this restriction and the following disclaimer,
must be included in all copies of the Software, in whole or in part, and
all derivative works of the Software, unless such copies or derivative
works are solely in the form of machine-executable object code generated by
a source language processor.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE, TITLE AND NON-INFRINGEMENT. IN NO EVENT
SHALL THE COPYRIGHT HOLDERS OR ANYONE DISTRIBUTING THE SOFTWARE BE LIABLE
FOR ANY DAMAGES OR OTHER LIABILITY, WHETHER IN CONTRACT, TORT OR OTHERWISE,
ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
DEALINGS IN THE SOFTWARE.

*/

#include <algorithm>
#include <cstddef>
#include <stdexcept>
#include <utility>

#include <osmium/index/index.hpp>
#include <osmium/index/map.hpp>
#include <osmium/io/detail/read_write.hpp>

namespace osmium {

    namespace index {

        namespace map {

            template <typename TVector, typename TId, typename TValue>
            class VectorBasedDenseMap : public Map<TId, TValue> {

                TVector m_vector;

            public:

                typedef TValue element_type;
                typedef TVector vector_type;
                typedef typename vector_type::iterator iterator;
                typedef typename vector_type::const_iterator const_iterator;

                VectorBasedDenseMap() :
                    m_vector() {
                }

                explicit VectorBasedDenseMap(int fd) :
                    m_vector(fd) {
                }

                ~VectorBasedDenseMap() noexcept final = default;

                void reserve(const size_t size) final {
                    m_vector.reserve(size);
                }

                void set(const TId id, const TValue value) final {
                    if (size() <= id) {
                        m_vector.resize(id+1);
                    }
                    m_vector[id] = value;
                }

                const TValue get(const TId id) const final {
                    try {
                        const TValue& value = m_vector.at(id);
                        if (value == osmium::index::empty_value<TValue>()) {
                            not_found_error(id);
                        }
                        return value;
                    } catch (std::out_of_range&) {
                        not_found_error(id);
                    }
                }

                size_t size() const final {
                    return m_vector.size();
                }

                size_t byte_size() const {
                    return m_vector.size() * sizeof(element_type);
                }

                size_t used_memory() const final {
                    return sizeof(TValue) * size();
                }

                void clear() final {
                    m_vector.clear();
                    m_vector.shrink_to_fit();
                }

                void dump_as_array(const int fd) final {
                    osmium::io::detail::reliable_write(fd, reinterpret_cast<const char*>(m_vector.data()), byte_size());
                }

                iterator begin() {
                    return m_vector.begin();
                }

                iterator end() {
                    return m_vector.end();
                }

                const_iterator cbegin() const {
                    return m_vector.cbegin();
                }

                const_iterator cend() const {
                    return m_vector.cend();
                }

                const_iterator begin() const {
                    return m_vector.cbegin();
                }

                const_iterator end() const {
                    return m_vector.cend();
                }

            }; // class VectorBasedDenseMap


            template <typename TId, typename TValue, template<typename...> class TVector>
            class VectorBasedSparseMap : public Map<TId, TValue> {

            public:

                typedef typename std::pair<TId, TValue> element_type;
                typedef TVector<element_type> vector_type;
                typedef typename vector_type::iterator iterator;
                typedef typename vector_type::const_iterator const_iterator;

            private:

                vector_type m_vector;

            public:

                VectorBasedSparseMap() :
                    m_vector() {
                }

                explicit VectorBasedSparseMap(int fd) :
                    m_vector(fd) {
                }

                ~VectorBasedSparseMap() final = default;

                void set(const TId id, const TValue value) final {
                    m_vector.push_back(element_type(id, value));
                }

                const TValue get(const TId id) const final {
                    const element_type element {
                        id,
                        osmium::index::empty_value<TValue>()
                    };
                    const auto result = std::lower_bound(m_vector.begin(), m_vector.end(), element, [](const element_type& a, const element_type& b) {
                        return a.first < b.first;
                    });
                    if (result == m_vector.end() || result->first != id) {
                        not_found_error(id);
                    } else {
                        return result->second;
                    }
                }

                size_t size() const final {
                    return m_vector.size();
                }

                size_t byte_size() const {
                    return m_vector.size() * sizeof(element_type);
                }

                size_t used_memory() const final {
                    return sizeof(element_type) * size();
                }

                void clear() final {
                    m_vector.clear();
                    m_vector.shrink_to_fit();
                }

                void sort() final {
                    std::sort(m_vector.begin(), m_vector.end());
                }

                void dump_as_list(const int fd) final {
                    osmium::io::detail::reliable_write(fd, reinterpret_cast<const char*>(m_vector.data()), byte_size());
                }

                iterator begin() {
                    return m_vector.begin();
                }

                iterator end() {
                    return m_vector.end();
                }

                const_iterator cbegin() const {
                    return m_vector.cbegin();
                }

                const_iterator cend() const {
                    return m_vector.cend();
                }

                const_iterator begin() const {
                    return m_vector.cbegin();
                }

                const_iterator end() const {
                    return m_vector.cend();
                }

            }; // class VectorBasedSparseMap

        } // namespace map

    } // namespace index

} // namespace osmium

#endif // OSMIUM_INDEX_DETAIL_VECTOR_MAP_HPP
