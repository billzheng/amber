#pragma once

#include "libcore/qstream/qstream_place.hpp"
#include <unordered_map>

#ifdef __AVX__
#include <immintrin.h>
#endif

namespace miye
{
namespace utils
{

template <typename T>
struct blob : public qstream::place
{
    blob(qstream::place const& d) : qstream::place(d)
    {
    }
    blob(void* p, size_t s) : qstream::place(p, s)
    {
    }
    blob(T* t, size_t s) : qstream::place(t, s)
    {
    }
    blob<T>& operator=(qstream::place const& d)
    {
        start = d.start;
        qstream::place::size = d.size;
        return *this;
    }
    T& operator*()
    {
        return *reinterpret_cast<T*>(start);
    }
    T* operator->()
    {
        return reinterpret_cast<T*>(start);
    }
    T* ptr() const
    {
        return reinterpret_cast<T*>(start);
    }
    size_t size() const
    {
        return qstream::place::size;
    }
    void size(size_t s)
    {
        qstream::place::size = s;
    }
    operator bool() const
    {
        return size() == 0;
    }
    template <class... Args>
    T* emplace(Args&&... args)
    {
        return new (start) T(args...);
    }
};

template <typename T>
struct const_blob : public qstream::place
{
    const_blob(qstream::place const& d) : qstream::place(d)
    {
    }
    const_blob(T const* t, size_t s) : qstream::place(t, s)
    {
    }
    const_blob(T const& t, size_t s) : qstream::place(&t, s)
    {
    }
    const_blob<T>& operator=(qstream::place const& d)
    {
        start = d.start;
        qstream::place::size = d.size;
        return *this;
    }
    T const& operator*() const
    {
        return *reinterpret_cast<T const*>(start);
    }
    T const* operator->() const
    {
        return reinterpret_cast<T const*>(start);
    }
    T const* ptr() const
    {
        return reinterpret_cast<T const*>(start);
    }
    size_t size() const
    {
        return qstream::place::size;
    }
    operator bool() const
    {
        return size() == 0;
    }
};

template <typename IO, typename T>
struct scoped_output
{
    template <class... Args>
    scoped_output(IO& io, Args&&... args)
        : io_(io), b_(io.begin_write(sizeof(T)))
    {
        // static_assert(IO::type != qstream::stream_tag::cbmmfile, "Use
        // ui_scope_output for cbmmfile streams");
        b_.emplace(args...);
    }
    T* operator->()
    {
        return b_.ptr();
    }
    T& operator&()
    {
        return *b_;
    }
    ~scoped_output()
    {
        io_.commit_write(b_);
    }

  private:
    IO& io_;
    blob<T> b_;
};

template <typename T>
struct decorated_T
{
    template <class... Args>
    decorated_T(uint64_t tstmp, uint64_t sz, Args&&... args)
        : ts(tstmp), size(sz), t(args...)
    {
    }
    uint64_t ts;
    uint64_t size;
    T t;
};

struct shared_buffer_cache
{
    shared_buffer_cache()
    {
    }
    ~shared_buffer_cache()
    {
        for (auto kv : cache_)
            delete[] kv.second;
    }
    void* get(std::string const& name, size_t size = 1024)
    {
        auto pos = cache_.find(name);
        if (pos == cache_.end())
        {
            cache_[name] = new char[size];
        }
        return cache_[name];
    }

    std::unordered_map<std::string, char*> cache_;
};

#ifdef __AVX__

template <typename T>
constexpr int num_writes()
{
    return (sizeof(decorated_T<T>) + 31) / 32;
}

template <typename IO, typename T>
struct streamed_scoped_output
{
    template <class... Args>
    streamed_scoped_output(IO& qstream, Args&&... args)
        : io_(qstream), tmp(io_.clock().now(), sizeof(T), args...),
          b_(qstream.begin_write(sizeof(T)))
    {
        // static_assert(IO::type == qstream::stream_tag::cbmmfile, "Use
        // scope_output for non cbmmfile streams");
#if !defined(USE_STREAMING_WRITES)
        INVARIANT(false);
#endif
        char* write_address = reinterpret_cast<char*>(b_.start);
        char const* read_address = reinterpret_cast<char const*>(&tmp);

        for (int i = 0; i != num_writes<T>(); ++i)
        {
            __m256 v0 = _mm256_load_ps((const float*)read_address);
            _mm256_stream_ps((float*)write_address, v0);
            read_address += 32;
            write_address += 32;
        }

        if (num_writes<T>() & 0x1)
        {
            // odd number of 32 byte writes
            // write out zeroes so that the total write is a multiple of 64
            // bytes
            //__m256 zero_ = _mm256_xor_ps(zero_, zero_);
            __m256 zero_ = {0, 0, 0, 0};
            _mm256_stream_ps((float*)write_address, zero_);
#ifndef NDEBUG
            write_address += 32;
#endif
        }

        ASSERT((write_address) == ((char*)b_.start) + (num_writes<T>() * 32));
    }
    T* operator->()
    {
        return b_.ptr();
    }
    T& operator&()
    {
        return *b_;
    }
    ~streamed_scoped_output()
    {
        io_.commit_write(b_);
    }

  private:
    IO& io_;
    ALIGN(32) decorated_T<T> tmp;
    blob<T> b_;
};

#endif

#ifdef USE_STREAMING_WRITES
#define ui_scoped_output streamed_scoped_output
#else
#define ui_scoped_output scoped_output
#endif

} // namespace utils
} // namespace miye
