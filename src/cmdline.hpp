// Part of the Jade Engine -- Copyright (c) Christian Neumüller 2012--2013
// This file is subject to the terms of the BSD 2-Clause License.
// See LICENSE.txt or http://opensource.org/licenses/BSD-2-Clause

#ifndef CMDLINE_HPP_INCLUDED
#define CMDLINE_HPP_INCLUDED CMDLINE_HPP_INCLUDED

#include <string>
#include <vector>


int argc();
const char* const* argv();
std::vector<std::string> const& commandLine();


#endif
