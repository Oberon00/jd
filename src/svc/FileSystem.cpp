// Part of the Jade Engine -- Copyright (c) Christian Neumüller 2012--2013
// This file is subject to the terms of the BSD 2-Clause License.
// See LICENSE.txt or http://opensource.org/licenses/BSD-2-Clause

#include "FileSystem.hpp"

#include "cmdline.hpp"
#include "Logfile.hpp"

#include <boost/filesystem/operations.hpp>
#include <physfs.h>

#include <array>

static std::string lastPhysFsError()
{
    char const* err = PHYSFS_getLastError();
    return err ? err : "OK";
}


vfs::Error::Error(std::string const& msg, bool getLastError):
    std::runtime_error(msg + (
        getLastError ? std::string(": ") + lastPhysFsError() : std::string())) { }



#define CALL_PHYSFS(fn, ...) \
    if (!fn(__VA_ARGS__))           \
        throw ::vfs::Error(#fn " failed"); \
    (void)0

VFile::VFile():
    m_f(nullptr)
{
}

VFile::VFile(std::string const& filename, OpenMode mode):
    m_f(vfs::openRaw(filename, mode))
{
}

VFile::VFile(PHYSFS_File* f):
    m_f(f)
{
    if (!m_f)
        m_err = "Invalid handle.";
}

VFile::VFile(VFile&& rhs)
{
    m_f = rhs.m_f;
    m_err = std::move(rhs.m_err);
    rhs.m_f = nullptr;
    rhs.m_err = "Invalid handle";
}

void VFile::open(std::string const& filename, OpenMode mode)
{
    close();
    m_f = vfs::openRaw(filename, mode);
    m_err.clear();
}

bool VFile::isOpen() const
{
    return m_f != nullptr;
}

bool VFile::eof()
{
    return PHYSFS_eof(m_f) ? true : false;
}

VFile::~VFile()
{
    try {
        close();
    } catch(std::exception const& e) {
        LOG_E("Error closing VFile in destructor:");
        LOG_EX(e);
    }
}

void VFile::close()
{
    if (m_f) {
        CALL_PHYSFS(PHYSFS_close, m_f);
        m_f = nullptr;
        m_err.clear();
    }
}


sf::Int64 VFile::read(void* data, sf::Int64 size)
{
    assert(size >= 0);
    return m_f ?
       check(PHYSFS_read(m_f, data, 1, static_cast<PHYSFS_uint32>(size)), size, "read") : -1;
}

sf::Int64 VFile::write(void const* data, sf::Int64 size)
{
    assert(size >= 0);
    return m_f ?
        check(PHYSFS_write(m_f, data, 1, static_cast<PHYSFS_uint32>(size)), size, "write") : -1;
}


sf::Int64 VFile::seek(sf::Int64 position)
{
    auto ipos = static_cast<PHYSFS_uint64>(position);
    if (!m_f || !check(PHYSFS_seek(m_f, ipos), 1, "seek"))
        return -1;
    return tell();
}

sf::Int64 VFile::tell()
{
    return m_f ? check(PHYSFS_tell(m_f), 0, "tell") : -1;
}

sf::Int64 VFile::getSize()
{
    return m_f ? check(PHYSFS_fileLength(m_f), 0, "getting file length") : -1;
}

sf::Int64 VFile::check(sf::Int64 r, sf::Int64 rq, std::string const& op)
{
    if (r < rq && !eof()) {
        char const* e = PHYSFS_getLastError();
        if (!e) {
            assert(PHYSFS_eof(m_f));
            return r;
        }
        m_err = op + " failed: " + (e ? e : "[no PhysFS error]");
    }
    return r;
}

void VFile::throwError() const
{
    if (!m_err.empty())
        throw vfs::Error(m_err, false);
}


//////////////////////////////////////////////////////////

VFileDevice::VFileDevice(VFile& f):
    m_f(f)
{
    throwErr();
}

std::streamsize VFileDevice::read(char* s, std::streamsize n)
{
    sf::Int64 const result = m_f.read(s, n);
    throwErr();
    return result;
}

std::streamsize VFileDevice::write(char const* s, std::streamsize n)
{
    sf::Int64 const result = m_f.write(s, n);
    throwErr();
    return result;
}

boost::iostreams::stream_offset VFileDevice::seek(
    boost::iostreams::stream_offset off, std::ios_base::seekdir way)
{
    sf::Int64 from;
    if (way == std::ios_base::beg)
        from = 0;
    else if (way == std::ios_base::cur)
        from = m_f.tell();
    else if (way == std::ios_base::end)
        from = m_f.getSize();
    else
        throw std::invalid_argument("invalid seekdir");

    throwErr();
    sf::Int64 const result = m_f.seek(from + off);
    throwErr();
    return result;
}

void VFileDevice::close()
{
    m_f.close();
}

void VFileDevice::throwErr()
{
    if (!m_f.lastError().empty())
        throw std::ios_base::failure(m_f.lastError());
}


//////////////////////////////////////////////////////////


vfs::Init::Init()
{
    // Use original encoding here.
    CALL_PHYSFS(PHYSFS_init, argv()[0]);
}

vfs::Init::~Init()
{
    if (!PHYSFS_isInit()) {
        LOG_W("PhysFS already shut down.");
    } else if (!PHYSFS_deinit()) {
        LOG_E("PHYSFS_deinit failed: " + std::string(PHYSFS_getLastError()));
    }
}


//////////////////////////////////////////////////////////


PHYSFS_File* vfs::openRaw(std::string const& name, VFile::OpenMode mode)
{
    PHYSFS_File* f;
    switch (mode) {
        case VFile::openR: f = PHYSFS_openRead(name.c_str()); break;
        case VFile::openW: f = PHYSFS_openWrite(name.c_str()); break;
        case VFile::openA: f = PHYSFS_openAppend(name.c_str()); break;
        default: throw Error("invalid openmode", false);
    }
    if (!f)
        throw Error("opening file \"" + name + "\" failed");
    return f;
}

bool vfs::mount(
    std::string const& path,
    std::string const& mountPoint,
    int flags)
{
    if (flags & writeDirectory) {
        boost::filesystem::create_directories(path);
        CALL_PHYSFS(PHYSFS_setWriteDir, path.c_str());
    }

    if (!PHYSFS_mount(path.c_str(), mountPoint.c_str(), flags & appendPath)) {
        Error err("", false);
        if (flags & logWarnings || !(flags & mountOptional)) {
            err = Error("Mounting \"" + path + "\" failed");
            if (flags & logWarnings)
                LOG_W(err.what());
        }
        if (flags & mountOptional)
            return false;
        throw err;
    }
    return true;
}


bool vfs::mountFirstWorking(
    std::vector<std::string> const& paths,
    std::string const& mountPoint,
    int flags)
{
    int const mflags = (flags & ~logWarnings) | mountOptional;
    std::string message = "Failed mounting all of the following:";
    for (auto const& path: paths) {
        if (mount(path, mountPoint, mflags))
            return true;
        else
            message += "\n\t" + path + ": " + lastPhysFsError();
    }
    if (flags & logWarnings)
        LOG_W(message);
    if (flags & mountOptional)
        return false;
    throw Error(message, false);
}
