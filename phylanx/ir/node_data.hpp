 // Copyright (c) 2017-2018 Hartmut Kaiser
// Copyright (c) 2017 Parsa Amini
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#if !defined(PHYLANX_IR_NODE_DATA_AUG_26_2017_0924AM)
#define PHYLANX_IR_NODE_DATA_AUG_26_2017_0924AM

#include <phylanx/config.hpp>
#include <phylanx/util/variant.hpp>

#include <hpx/include/serialization.hpp>
#include <hpx/include/util.hpp>
#include <hpx/throw_exception.hpp>

#include <array>
#include <cstddef>
#include <cstdint>
#include <iosfwd>
#include <type_traits>
#include <utility>
#include <vector>

#include <blaze/Math.h>

namespace phylanx { namespace ir
{
    ///////////////////////////////////////////////////////////////////////////
    template <typename T>
    class PHYLANX_EXPORT node_data;

    namespace detail
    {
        /// \cond NOINTERNAL

        ///////////////////////////////////////////////////////////////////////
        template <typename T>
        class node_data_iterator
          : public hpx::util::iterator_facade<node_data_iterator<T>,
                T,
                std::random_access_iterator_tag,
                T const&,
                std::ptrdiff_t,
                T const*>
        {
        private:
            using base_type =
                hpx::util::iterator_facade<node_data_iterator<T>,
                    T,
                    std::random_access_iterator_tag,
                    T const&,
                    std::ptrdiff_t,
                    T const*>;

        public:
            node_data_iterator(node_data<T> const& nd, std::size_t index = 0)
              : nd_(nd), index_(index)
            {
            }

        private:
            friend class hpx::util::iterator_core_access;

            typename base_type::reference dereference() const
            {
                return nd_[index_];
            }

            bool equal(node_data_iterator const& x) const
            {
                return &nd_ == &x.nd_ && index_ == x.index_;
            }

            void advance(typename base_type::difference_type n)
            {
                index_ += n;
            }

            void increment()
            {
                ++index_;
            }

            void decrement()
            {
                --index_;
            }

            typename base_type::difference_type distance_to(
                node_data_iterator const& y) const
            {
                return y.index_ - index_;
            }

            node_data<T> const& nd_;
            std::size_t index_;
        };
    }

    template <typename T>
    class node_data
    {
    private:
        static void increment_copy_construction_count();
        static void increment_move_construction_count();
        static void increment_copy_assignment_count();
        static void increment_move_assignment_count();

    public:
        static std::int64_t copy_construction_count(bool reset);
        static std::int64_t move_construction_count(bool reset);
        static std::int64_t copy_assignment_count(bool reset);
        static std::int64_t move_assignment_count(bool reset);

        static bool enable_counts(bool enable);

    public:
        constexpr static std::size_t const max_dimensions = 2;

        using dimensions_type = std::array<std::size_t, max_dimensions>;

        using storage0d_type = T;
        using storage1d_type = blaze::DynamicVector<T>;
        using storage2d_type = blaze::DynamicMatrix<T>;

        using custom_storage1d_type = blaze::CustomVector<T, true, true>;
        using custom_storage2d_type = blaze::CustomMatrix<T, true, true>;

        using storage_type =
            util::variant<storage0d_type, storage1d_type, storage2d_type,
                custom_storage1d_type, custom_storage2d_type>;

        node_data() = default;

        explicit node_data(dimensions_type const& dims);
        node_data(dimensions_type const& dims, T default_value);

        /// Create node data for a 0-dimensional value
        explicit node_data(storage0d_type const& value);
        explicit node_data(storage0d_type && value);

        /// Create node data for a 1-dimensional value
        explicit node_data(storage1d_type const& values);
        explicit node_data(storage1d_type && values);

        explicit node_data(custom_storage1d_type const& values);
        explicit node_data(custom_storage1d_type && values);

        /// Create node data for a 2-dimensional value
        explicit node_data(storage2d_type const& values);
        explicit node_data(storage2d_type && values);

        explicit node_data(custom_storage2d_type const& values);
        explicit node_data(custom_storage2d_type && values);

        // conversion helpers for Python bindings
        explicit node_data(std::vector<T> const& values);
        explicit node_data(std::vector<std::vector<T>> const& values);

        template <typename U = T, typename U1 =
            typename std::enable_if<!std::is_same<U, bool>::value>::type>
        explicit node_data(std::vector<std::vector<T>> const& values)
          : data_(storage2d_type{values.size(), values[0].size()})
        {
            std::size_t const nx = values.size();
            for (std::size_t i = 0; i != nx; ++i)
            {
                std::vector<T> const& row = values[i];
                std::size_t const ny = row.size();
                for (std::size_t j = 0; j != ny; ++j)
                {
                    util::get<2>(data_)(i, j) = row[j];
                }
            }
        }

    private:
        static storage_type init_data_from(node_data const& d);

        template <typename U>
        static storage_type init_data_from_type(node_data<U> const& d)
        {
            std::size_t dims = d.num_dimensions();

            switch (dims)
            {
            case 0:
                return storage_type(d.scalar());

            case 1:
                increment_copy_construction_count();
                return storage_type(d.vector());

            case 2:
                increment_copy_construction_count();
                return storage_type(d.matrix());

            default:
                HPX_THROW_EXCEPTION(hpx::invalid_status,
                    "phylanx::ir::node_data<T>::node_data<U>",
                    "node_data object holds unsupported data type");
            }
        }

    public:
        /// Create node data from a node data
        node_data(node_data const& d);
        node_data(node_data && d);

        template <typename U, typename U1 =
            typename std::enable_if<!std::is_same<T, U>::value>::type>
        explicit node_data(node_data<U> const& d)
          : data_(init_data_from_type(d))
        {
        }

        node_data& operator=(storage0d_type val);

        node_data& operator=(storage1d_type const& val);
        node_data& operator=(storage1d_type && val);

        node_data& operator=(custom_storage1d_type const& val);
        node_data& operator=(custom_storage1d_type && val);

        node_data& operator=(storage2d_type const& val);
        node_data& operator=(storage2d_type && val);

        node_data& operator=(custom_storage2d_type const& val);
        node_data& operator=(custom_storage2d_type && val);

        // conversion helpers for Python bindings
        node_data& operator=(std::vector<T> const& val);
        node_data& operator=(std::vector<std::vector<T>> const& values);

    private:
        static storage_type copy_data_from(node_data const& d);

    public:
        node_data& operator=(node_data const& d);
        node_data& operator=(node_data && d);

        template <typename U, typename U1 =
            typename std::enable_if<!std::is_same<T, U>::value>::type>
        node_data& operator=(node_data<U> const& d)
        {
            data_ = init_data_from_type(d);
            return *this;
        }

        /// Access a specific element of the underlying N-dimensional array
        T& operator[](std::size_t index);
        T const& operator[](std::size_t index) const;

        T& operator[](dimensions_type const& indicies);
        T const& operator[](dimensions_type const& indicies) const;

        T& at(std::size_t index1, std::size_t index2);
        T const& at(std::size_t index1, std::size_t index2) const;

        std::size_t size() const;

        storage2d_type& matrix_non_ref();
        storage2d_type const& matrix_non_ref() const;
        storage2d_type matrix_copy() const;

        custom_storage2d_type matrix() &;
        custom_storage2d_type matrix() const&;
        custom_storage2d_type matrix() &&;
        custom_storage2d_type matrix() const&&;

        storage1d_type& vector_non_ref();
        storage1d_type const& vector_non_ref() const;
        storage1d_type vector_copy() const;

        custom_storage1d_type vector() &;
        custom_storage1d_type vector() const&;
        custom_storage1d_type vector() &&;
        custom_storage1d_type vector() const&&;

        storage0d_type& scalar();
        storage0d_type const& scalar() const;

        /// Extract the dimensionality of the underlying data array.
        std::size_t num_dimensions() const;

        /// Extract the dimensional extends of the underlying data array.
        dimensions_type dimensions() const;
        std::size_t dimension(int dim) const;

        /// Return a new instance of node_data referring to this instance.
        node_data<T> ref() &;
        node_data<T> ref() const&;
        node_data<T> ref() &&;
        node_data<T> ref() const&&;

        /// Return a new instance of node_data holding a copy of this instance.
        node_data<T> copy() const;

        /// Return whether the internal representation is referring to another
        /// instance of node_data
        bool is_ref() const;

        explicit operator bool() const;

        bool operator!() const
        {
            return !bool(*this);
        }

        using const_iterator = detail::node_data_iterator<T>;

        const_iterator begin() const;
        const_iterator end() const;

        const_iterator cbegin() const;
        const_iterator cend() const;

        // conversion helpers for Python bindings
        std::vector<T> as_vector() const;
        std::vector<std::vector<T>> as_matrix() const;
        std::size_t index() const { return data_.index(); }

    private:
        /// \cond NOINTERNAL
        friend class hpx::serialization::access;

        void serialize(hpx::serialization::input_archive& ar, unsigned);
        void serialize(hpx::serialization::output_archive& ar, unsigned);

        storage_type data_;
        /// \endcond
    };

    ///////////////////////////////////////////////////////////////////////////
    struct reset_enable_counts_on_exit
    {
        reset_enable_counts_on_exit(bool enabled = false)
          : enabled_(node_data<double>::enable_counts(enabled))
        {}

        ~reset_enable_counts_on_exit()
        {
            node_data<double>::enable_counts(enabled_);
        }

        bool enabled_;
    };

    ///////////////////////////////////////////////////////////////////////////
    PHYLANX_EXPORT bool operator==(
        node_data<double> const& lhs, node_data<double> const& rhs);
    PHYLANX_EXPORT bool operator==(
        node_data<std::uint8_t> const& lhs, node_data<std::uint8_t> const& rhs);
    PHYLANX_EXPORT bool operator==(
        node_data<std::int64_t> const& lhs, node_data<std::int64_t> const& rhs);

    template <typename T>
    bool operator!=(node_data<T> const& lhs, node_data<T> const& rhs)
    {
        return !(lhs == rhs);
    }

    PHYLANX_EXPORT std::ostream& operator<<(
        std::ostream& out, node_data<double> const& nd);
    PHYLANX_EXPORT std::ostream& operator<<(
        std::ostream& out, node_data<std::uint8_t> const& nd);
    PHYLANX_EXPORT std::ostream& operator<<(
        std::ostream& out, node_data<std::int64_t> const& nd);
}}

#endif
