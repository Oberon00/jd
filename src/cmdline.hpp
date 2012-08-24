#ifndef CMDLINE_HPP_INCLUDED
#define CMDLINE_HPP_INCLUDED CMDLINE_HPP_INCLUDED

#include <vector>
#include <string>

int argc();
const char* const* argv();
std::vector<std::string> const& commandLine();


#endif