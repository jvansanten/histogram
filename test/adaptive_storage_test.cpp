// Copyright 2015-2016 Hans Dembinski
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt
// or copy at http://www.boost.org/LICENSE_1_0.txt)

#define BOOST_TEST_MODULE adaptive_storage_test
#include <boost/test/unit_test.hpp>
#include <boost/test/test_tools.hpp>
#include <boost/histogram/storage/adaptive_storage.hpp>
#include <boost/histogram/storage/container_storage.hpp>
#include <boost/histogram/detail/utility.hpp>
#include <limits>
#include <tuple>
using namespace boost::histogram;

template <typename Storage1, typename Storage2>
bool operator==(const Storage1& a, const Storage2& b)
{
    if (a.size() != b.size())
        return false;
    return detail::storage_content_equal(a, b);
}

namespace boost {
namespace histogram {

struct storage_access {
    template <typename T>
    static std::tuple<adaptive_storage, double>
    max_minus_one() {
        const auto tmax = std::numeric_limits<T>::max();
        adaptive_storage s(1);
        s.increase(0);
        while (s.value(0) < 0.1 * tmax)
            s += s;
        static_cast<T*>(s.buffer_.ptr_)[0] = tmax - 1;
        const double v = tmax - 1;
        BOOST_CHECK_EQUAL(s.value(0), v);
        return std::tie(s, v);
    }
};

}
}

BOOST_AUTO_TEST_CASE(equal_operator)
{
    adaptive_storage a(1), b(1), c(1), d(2);
    a.increase(0);
    b.increase(0);
    c.increase(0);
    c.increase(0);
    d.increase(0);
    BOOST_CHECK(a == a);
    BOOST_CHECK(a == b);
    BOOST_CHECK(!(a == c));
    BOOST_CHECK(!(a == d));
}

template <typename T>
void increase_and_grow_impl()
{
    adaptive_storage s; double v;
    std::tie(s, v) = storage_access::max_minus_one<T>();

    auto n = s;
    auto n2 = s;

    n.increase(0);
    n.increase(0);

    adaptive_storage x(1);
    x.increase(0);
    n2 += x;
    n2 += x;

    v += 2;
    BOOST_CHECK_EQUAL(n.value(0), v);
    BOOST_CHECK_EQUAL(n2.value(0), v);
}

BOOST_AUTO_TEST_CASE(increase_and_grow)
{
    increase_and_grow_impl<uint8_t>();
    increase_and_grow_impl<uint16_t>();
    increase_and_grow_impl<uint32_t>();
    increase_and_grow_impl<uint64_t>();
}

BOOST_AUTO_TEST_CASE(add_and_grow)
{
    adaptive_storage a(1);
    a.increase(0);
    double x = 1.0;
    adaptive_storage y(1);
    BOOST_CHECK_EQUAL(y.value(0), 0.0);
    a += y;
    BOOST_CHECK_EQUAL(a.value(0), x);
    for (unsigned i = 0; i < 80; ++i) {
        a += a;
        x += x;
        adaptive_storage b(1);
        b += a;
        BOOST_CHECK_EQUAL(a.value(0), x);
        BOOST_CHECK_EQUAL(a.variance(0), x);
        BOOST_CHECK_EQUAL(b.value(0), x);
        BOOST_CHECK_EQUAL(b.variance(0), x);
        b.increase(0, 0.0);
        BOOST_CHECK_EQUAL(b.value(0), x);
        BOOST_CHECK_EQUAL(b.variance(0), x);
        adaptive_storage c(1);
        c.increase(0, 0.0);
        c += a;
        BOOST_CHECK_EQUAL(c.value(0), x);
        BOOST_CHECK_EQUAL(c.variance(0), x);
    }
}

BOOST_AUTO_TEST_CASE(equality)
{
    adaptive_storage a(2), b(2), c(2);
    BOOST_CHECK(a == b);
    a.increase(0);
    BOOST_CHECK(!(a == b));
    c.increase(0, 1.0);
    BOOST_CHECK(a == c);
}

template <typename T>
void convert_container_storage_impl() {
    adaptive_storage a(2), b(2), c(2);
    container_storage<std::vector<T>> s(2);
    a.increase(0);
    c.increase(0, 1.0);
    s.increase(0);
    for (unsigned i = 0; i < (8 * sizeof(T) - 1); ++i)
        s += s;
    a = s;
    b = s;
    c = s;
    BOOST_CHECK(a == s);
    BOOST_CHECK(b == s);
    BOOST_CHECK(c == s);
    a = adaptive_storage(2);
    b = adaptive_storage(2);
    BOOST_CHECK(a == b);
    a += s;
    BOOST_CHECK(a == s);
    adaptive_storage d(s);
    BOOST_CHECK(d == s);
    adaptive_storage e;
    e = s;
    BOOST_CHECK(e == s);
}

BOOST_AUTO_TEST_CASE(convert_container_storage)
{
    convert_container_storage_impl<uint8_t>();
    convert_container_storage_impl<uint16_t>();
    convert_container_storage_impl<uint32_t>();
    convert_container_storage_impl<uint64_t>();
}
