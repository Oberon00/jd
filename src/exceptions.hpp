// Part of the Jade Engine -- Copyright (c) Christian Neumüller 2012--2013
// This file is subject to the terms of the BSD 2-Clause License.
// See LICENSE.txt or http://opensource.org/licenses/BSD-2-Clause

#ifndef EXCEPTIONS_HPP_INCLUDED
#define EXCEPTIONS_HPP_INCLUDED EXCEPTIONS_HPP_INCLUDED

/// \file Exceptions.hpp \brief Defines common exception-classes for jd.

#include <stdexcept>


namespace jd {

/// \brief Base-class for various jd-exceptions
class Exception :public std::runtime_error {
    public: explicit Exception(std::string const& msg): std::runtime_error(msg) { }
};

/// \brief Indicates an resource-related error.
/// \see ResourceManager
class ResourceError :public Exception {
    public: explicit ResourceError(std::string const& msg): Exception(msg) { }
};

/// \brief Thrown if an resource can not be loaded.
class ResourceLoadError :public ResourceError {
    public: explicit ResourceLoadError(std::string const& msg): ResourceError(msg) { }
};

}

#endif
