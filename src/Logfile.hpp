// Part of the Jade Engine -- Copyright (c) Christian Neumüller 2012--2013
// This file is subject to the terms of the BSD 2-Clause License.
// See LICENSE.txt or http://opensource.org/licenses/BSD-2-Clause

#pragma once

#include <boost/format/format_fwd.hpp>
#include <boost/iostreams/filtering_stream.hpp>
#include <SFML/System/Clock.hpp>

#include <string>
#include <fstream>


enum class loglevel { debug, info, warning, error, fatal,  max  };
enum class logstyle { html, plain, like_extension,  max };

class Logfile
{
public:
    explicit Logfile(const std::string& filename,
                     loglevel min_level = loglevel::debug,
                     logstyle style = logstyle::like_extension);
    Logfile();
    ~Logfile();
    void write(
        const std::string& msg,
        loglevel level = loglevel::info,
        char const* location = nullptr);
    void write(
        const boost::format& msg,
        loglevel level = loglevel::info,
        char const* location = nullptr);

    void open(const std::string& filename, logstyle style = logstyle::like_extension);
    loglevel minLevel() const;
    void setMinLevel(loglevel level);
    BOOST_ATTRIBUTE_NORETURN void logThrow(
        const std::exception& ex,
        loglevel level = loglevel::error,
        char const* location = nullptr);
    void logEx(
        const std::exception& ex,
        loglevel level = loglevel::error,
        char const* location = nullptr);

    class Error;

private:
    class LineNotifier;

    void init();
    const std::string printable_time() const;

    loglevel m_min_level;
    logstyle m_style;
    std::ofstream m_file;
    boost::iostreams::filtering_ostream m_sferr;
    sf::Clock m_timer;
    std::streambuf* m_originalSfBuf;
};

class Logfile::Error :public std::runtime_error
{
public:
    Error(const std::string& msg): std::runtime_error(msg) { }
};

inline Logfile& log() {
    static Logfile instance;
    return instance;
}

#ifndef LOGFILE_NO_SHORTCUTS
#   define LOGFILE_LOCATION __FILE__ ":" BOOST_STRINGIZE(__LINE__)
#   define LOGFILE_LOG(s, l) log().write((s), loglevel::l, LOGFILE_LOCATION)

#   define LOG_D(s) LOGFILE_LOG((s), debug)
#   define LOG_I(s) LOGFILE_LOG((s), info)
#   define LOG_W(s) LOGFILE_LOG((s), warning)
#   define LOG_E(s) LOGFILE_LOG((s), error)
#   define LOG_F(s) LOGFILE_LOG((s), fatal)

#   define LOG_EX(e) log().logEx(e, loglevel::error, LOGFILE_LOCATION)
#   define LOG_THROW(e) log().logThrow((e), loglevel::error, LOGFILE_LOCATION)
#endif
