#ifndef DINOBOT_SYS_UTILS_H
#define DINOBOT_SYS_UTILS_H

#include <exception>
#include <assert.h>

namespace dinobot { namespace lib { namespace base {

    /**
     *
     * Atomically creates a new file for exclusive writing, or opens
     * an existing file.
     *
     * If truncate is set to true, make use of clever locking to ensure the file
     * is swapped over without disrupting existing readers of the file.
     *
     * If block is set to true, flock calls will block until the file is unlocked.
     *
     * Throws on error, otherwise returns the open file descriptor
     *
     * */
    inline static int atomic_open_for_write(const std::string& filename, bool truncate,
            bool append, bool block = false)
    {

        int fd = 0;

        int flags = append ? (O_RDWR|O_CREAT|O_APPEND) : (O_RDWR|O_CREAT);

        if (truncate)
        {

            // complex file dance to avoid race conditions with rewriting files
            std::string replace_filename = filename + ".new.tmp";
            int replace_fd = ::open(replace_filename.c_str(), flags, 0666);
            if (replace_fd == -1)
            {
				throw std::runtime_error("open failed");
            }

            int lock_result = ::flock(replace_fd, block ? LOCK_EX : (LOCK_EX | LOCK_NB));
            if (lock_result && errno != ENOSYS)
            {
                ::close(replace_fd);
				throw std::runtime_error("flock failed");
            }

            int existing_fd = ::open(filename.c_str(), flags, 0666);
            if (existing_fd == -1)
            {
                ::unlink(replace_filename.c_str()); // delete first while we hold the lock
                ::close(replace_fd);
				throw std::runtime_error("open failed");
            }

            lock_result = ::flock(existing_fd, block ? LOCK_EX : (LOCK_EX | LOCK_NB));
            if (lock_result)
            {
                if (errno != ENOSYS)
                {
                    ::unlink(replace_filename.c_str()); // delete first while we hold the lock
                    ::close(replace_fd);
                    ::close(existing_fd);
					throw std::runtime_error("flock failed");
                }
            }

            if (::ftruncate(replace_fd, 0))
            {
                ::unlink(replace_filename.c_str()); // delete first while we hold the lock
                ::close(replace_fd);
                ::close(existing_fd);
				throw std::runtime_error("ftruncate failed");
            }

            if (::rename(replace_filename.c_str(), filename.c_str()))
            {
                ::unlink(replace_filename.c_str()); // delete first while we hold the lock
                ::close(replace_fd);
                ::close(existing_fd);
				throw std::runtime_error("rename failed");
            }

            // now we're safe
            close(existing_fd);
            fd = replace_fd;

        } else
        {
            int result = ::open(filename.c_str(), flags, 0666);
            if (result == -1)
            {
				throw std::runtime_error("open failed");
            }
            if (result == 0)
                abort();
            assert(result != 0);

            int lock_result = ::flock(result, block ? LOCK_EX : (LOCK_EX | LOCK_NB));
            if (lock_result)
            {
                if (errno != ENOSYS)
                {
                    close(result);
					throw std::runtime_error("flock failed");
                }
            }
            fd = result;
        }

        return fd;
    }

} } }

#endif /* SYS_UTILS_H */
