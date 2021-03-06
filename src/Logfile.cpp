// Part of the Jade Engine -- Copyright (c) Christian Neumüller 2012--2013
// This file is subject to the terms of the BSD 2-Clause License.
// See LICENSE.txt or http://opensource.org/licenses/BSD-2-Clause

#include "Logfile.hpp"

#include "encoding.hpp"

#include <boost/exception/diagnostic_information.hpp>
#include <boost/format.hpp>
#include <boost/iostreams/device/file.hpp>
#include <boost/iostreams/filter/line.hpp>
#include <SFML/System/Err.hpp>

#include <array>
#include <iomanip>
#include <sstream>


class Logfile::LineNotifier :public boost::iostreams::line_filter
{
public:
    LineNotifier(Logfile* parent): m_parent(parent) { }

private:
     std::string do_filter(const std::string& line)
     {
         if (loglevel::error >= m_parent->minLevel())
             m_parent->write(line, loglevel::error, "<unknown/SFML>");
         return "";
     }
     Logfile* m_parent;
};

static const std::string full_time()
{
    std::time_t tms = std::time(nullptr);
#ifdef BOOST_MSVC
#    pragma warning (push)
#    pragma warning (disable:4996)
#endif
    return ctime(&tms);
#ifdef BOOST_MSVC
#    pragma warning (pop)
#endif
}

const std::string Logfile::printable_time() const
{
    std::stringstream str;
    str << std::setprecision(6) << std::fixed << m_timer.getElapsedTime().asSeconds();
    return str.str();
}

static const std::string filterHtml(std::string const& s)
{
    std::string result;
    result.reserve(s.size() * 3 / 2);
    for (char const c : s) switch (c)
    {
        case '<':
            result += "&lt;";
            break;
        case '>':
            result += "&gt;";
            break;
        case '&':
            result += "&amp;";
            break;
        case '"':
            result += "&quot;";
            break;
        default:
            result += c;
            break;
    }
    return result;
}

Logfile::Logfile(const std::string& filename, loglevel min_level_, logstyle style_):
    m_min_level(min_level_),
    m_style(style_)
{
    assert(m_style < logstyle::max);
    assert(m_min_level < loglevel::max);
    init();
    open(filename, style_);
}

Logfile::Logfile():
    m_min_level(loglevel::max)
{
    init();
}

Logfile::~Logfile()
{
    LOG_I("End logging at " + full_time());

    if (m_style == logstyle::html)
        m_file << "\n</tbody></table> </body> </html>\n";
    sf::err().rdbuf(m_originalSfBuf);
}

void Logfile::init()
{
    m_sferr.push(LineNotifier(this), 0);
    m_sferr.push(m_file, 0);
    m_originalSfBuf = sf::err().rdbuf();
    sf::err().rdbuf(m_sferr.rdbuf());
}

void Logfile::open(const std::string& filename, logstyle style)
{
    m_file.clear();
    m_file.open(enc::utf8ToFstreamArg(filename));
    m_file << "";
    if (!m_file)
        throw Error("Cannot open or write to file \"" + filename + "\"!");

    if (style == logstyle::like_extension)
    {
        // Determine style (plain if file has any extension other than html or htm).
        const std::string::size_type pos_dot = filename.rfind('.');
        const std::string extension = pos_dot == std::string::npos ? "" : filename.substr(pos_dot);
        if (extension == ".html" || extension == ".htm")
            m_style = logstyle::html;
        else
            m_style = logstyle::plain;
    }

    if (m_style == logstyle::html)
    {
        m_file << "<html> <head> <title>Logfile</title>"
                  "<link rel=\"stylesheet\" href=\"logstyle.css\" type=\"text/css\"/> </head>\n"
                  "<body ><h1>Logfile</h1>\n<table> <thead><tr> "
                  "<td>Level</td> <td>Time(s)</td> <td>Location</td> <td>Message</td> </tr></thead><tbody>\n";
    } else {
        m_file << "===< LOGFILE >===\n\n"
                  "Level      Time(s)    Location                              Message\n"
                  "---------- ---------- ------------------------------------- ----------" \
                  "----------------------------------------\n";
    }
    LOG_I("Start logging at " + full_time());
}

void Logfile::write(const boost::format& msg, loglevel level, char const* location)
{
    write(msg.str(), level, location);
}

void Logfile::write(std::string const& msg, loglevel level_, char const* location)
{
    if (level_ < m_min_level)
        return;

    auto level = static_cast<unsigned>(level_);

    static auto const maxloglevel = static_cast<int>(loglevel::max);
    static_assert(maxloglevel == 5, "Please correct levelnames below!");
    static const std::array<const char* const, maxloglevel> levelnames =
        { "debug", "info", "warning", "error", "fatal" };
    static const std::array<const char* const, maxloglevel> levelpreambles =
        { "    ", "   _", "  + ", " *  ", "!>>>" };

    std::string location_;
    std::string fmt;
    std::string real_msg;
    real_msg.reserve(msg.size());

    if (m_style == logstyle::html)
    {
        if (location) {
            location_ = filterHtml(location);
            location = location_.c_str();
        }
        real_msg = filterHtml(msg);
        fmt = str(boost::format("<tr class=\"%1%\"><td>%1%</td>") % levelnames[level]) +
            "<td>%1%</td><td>%2%</td><td><pre>%3%</pre></td></tr>";
    }
    else // if (m_style == logstyle::plain)
    {
        fmt = levelpreambles[level] + str(boost::format("%|-8|") % levelnames[level]) + "%|9| %|-37| %||";
        std::string msg2 = msg;
        if (msg.back() == '\n')
            msg2.pop_back();

        // Add indentation to linebreaks
        for (const char c : msg2) switch (c)
        {
            case '\n':
                real_msg += "\n" + std::string(60, ' ');
                break;
            default:
                real_msg += c;
                break;
        }
    }

    std::string filename(location ? location : "<unknown>");
    if (location) {
        std::string::size_type posSrc;
#ifdef _WIN32
        posSrc = filename.rfind("src\\");
        if (posSrc == std::string::npos)
#endif
            posSrc = filename.rfind("src/");
        if (posSrc != std::string::npos)
            filename.erase(0, posSrc + 4);
    }
    m_file << boost::format(fmt)
        % printable_time()
        % (m_style == logstyle::html ? filename : filterHtml(filename))
        % real_msg << '\n';
    m_file.flush();
}


loglevel Logfile::minLevel() const
{
    return m_min_level;
}

void Logfile::setMinLevel(loglevel level)
{
    m_min_level = level;
}

void Logfile::logThrow(const std::exception& ex, loglevel level, char const* location)
{
    logEx(ex, level, location);
    throw ex;
}

void Logfile::logEx(const std::exception& ex, loglevel level, char const* location)
{
    write(boost::diagnostic_information(ex), level, location);
}
