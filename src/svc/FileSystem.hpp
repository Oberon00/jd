// Part of the Jade Engine -- Copyright (c) Christian Neumüller 2012--2013
// This file is subject to the terms of the BSD 2-Clause License.
// See LICENSE.txt or http://opensource.org/licenses/BSD-2-Clause

#ifndef FILESYSTEM_HPP_INCLUDED
#define FILESYSTEM_HPP_INCLUDED FILESYSTEM_HPP_INCLUDED

#include "compsys/Component.hpp"

#include <boost/iostreams/categories.hpp>
#include <boost/iostreams/positioning.hpp>
#include <SFML/System/InputStream.hpp>
#include <boost/range/any_range.hpp>

#include <iosfwd>
#include <string>
#include <vector>

struct PHYSFS_File;

class VFile: public sf::InputStream, private boost::noncopyable
{
public:
    enum OpenMode { openR, openW, openA };

    VFile();
    VFile(std::string const& filename, OpenMode mode = openR);
    VFile(PHYSFS_File* f);
    VFile(VFile&& rhs);

    virtual ~VFile();
    virtual void close();

    virtual sf::Int64 read(void* data, sf::Int64 size);
    sf::Int64 write(void const* data, sf::Int64 size);

    virtual sf::Int64 seek(sf::Int64 position);
    virtual sf::Int64 tell();
    virtual sf::Int64 getSize();

    void open(std::string const& filename, OpenMode mode = openR);
    bool isOpen() const;
    bool eof();
    std::string const& lastError() const { return m_err; }
    void clearLastError() { m_err.clear(); }
    void throwError() const;

    PHYSFS_File* get() { return m_f; }

private:
    sf::Int64 check(sf::Int64 r, sf::Int64 rq, std::string const& op = "I/O operation");

    std::string m_err;
    PHYSFS_File* m_f;
};

class VFileDevice
{
public:
    typedef char char_type;
    struct category:
        public boost::iostreams::device_tag,
        public boost::iostreams::closable_tag,
        public boost::iostreams::seekable
    { };

    VFileDevice(VFile& f);

    std::streamsize read(char* s, std::streamsize n);
    std::streamsize write(char const* s, std::streamsize n);
    boost::iostreams::stream_offset seek(
        boost::iostreams::stream_offset off, std::ios_base::seekdir way);
    void close();

private:
    VFileDevice& operator=(VFileDevice const&);
    void throwErr();

    VFile& m_f;
};

namespace vfs { // Virtual File System
    struct Init { Init(); ~Init(); };
    
    class Error: public std::runtime_error {
    public:
        Error(std::string const& msg, bool getLastError = true);
    };

    PHYSFS_File* openRaw(std::string const& name, VFile::OpenMode mode);

    enum MountFlags {
        prependPath = 0, appendPath = 1,
        writeDirectory = 2,
        mountOptional = 4,
        logWarnings = 8};

    bool mount(
        std::string const& path,
        std::string const& mountPoint = std::string(),
        int flags = prependPath);

    bool mountFirstWorking(
        std::vector<std::string> const& paths,
        std::string const& mountPoint = std::string(),
        int flags = prependPath);
} // namespace vfs

#endif
