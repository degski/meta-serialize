
// MIT License
//
// Copyright (c) 2019 degski
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#include <cassert>
#include <cstddef>
#include <cstdint>
#include <cstdlib>

#include <array>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <iterator>
#include <list>
#include <map>
#include <random>
#include <sax/iostream.hpp>
#include <sstream>
#include <string>
#include <tuple>
#include <type_traits>
#include <vector>

namespace details {
template<class>
struct is_ref_wrapper : std::false_type {};
template<class T>
struct is_ref_wrapper<std::reference_wrapper<T>> : std::true_type {};

template<class T>
using not_ref_wrapper = std::negation<is_ref_wrapper<std::decay_t<T>>>;

template<class D, class...>
struct return_type_helper {
    using type = D;
};
template<class... Types>
struct return_type_helper<void, Types...> : std::common_type<Types...> {
    static_assert ( std::conjunction_v<not_ref_wrapper<Types>...>, "Types cannot contain reference_wrappers when D is void" );
};

template<class D, class... Types>
using return_type = std::array<typename return_type_helper<D, Types...>::type, sizeof...( Types )>;
} // namespace details

template<class D = void, class... Types>
constexpr details::return_type<D, Types...> make_array ( Types &&... t ) {
    return { std::forward<Types> ( t )... };
}

#include <boost/preprocessor.hpp>

#define EXPOSE_MEMBERS_Q( M_ignore0, M_ignore1, M_member ) BOOST_PP_STRINGIZE ( M_member )

#define EXPOSE_MEMBERS( ... )                                                                                                      \
    auto members ( ) { return std::forward_as_tuple ( __VA_ARGS__ ); }                                                             \
    auto members ( ) const { return std::forward_as_tuple ( __VA_ARGS__ ); }                                                       \
    static constexpr auto names ( ) {                                                                                              \
        return make_array (                                                                                                        \
            BOOST_PP_LIST_ENUM ( BOOST_PP_LIST_TRANSFORM ( EXPOSE_MEMBERS_Q, @, BOOST_PP_VARIADIC_TO_LIST ( __VA_ARGS__ ) ) ) );   \
    }

template<typename T>
std::ostream & operator<< ( std::ostream & os, T const & obj ) {
    using std::operator<< ;

    std::apply (
        [&os] ( auto const & names, auto const & fst, auto const &... rest ) {
            unsigned int i = 0;
            os << names[ i ] << '=' << fst;
            ( ( os << ", " << names[ ++i ] << '=' << rest ), ... );
        },
        std::tuple_cat ( std::make_tuple ( obj.names ( ) ), obj.members ( ) ) );
    return os;
}

template<typename T>
std::istream & operator>> ( std::istream & is, T & obj ) {
    using std::operator>> ;

    std::apply ( [&is] ( auto &... members ) { ( is >> ... >> members ); }, obj.members ( ) );
    return is;
}

struct employee {
    std::string name;
    int salary;

    EXPOSE_MEMBERS ( name, salary );
};

int main ( ) {
    employee e{ "Steve Jobs", 1 };

    std::cout << e << nl;

    std::string input = "Bill-Gates 100";
    std::istringstream ss{ input };
    ss >> e;

    std::cout << e << nl;

    return EXIT_SUCCESS;
}
