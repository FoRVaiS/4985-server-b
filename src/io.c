#include "io.h"
#include <errno.h>
#include <fcntl.h>
#include <p101_c/p101_stdio.h>
#include <p101_c/p101_stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#define TIMEOUT 10000
#define MILLI_SEC 1000

ssize_t read_fully(int fd, char *buf, size_t size, int *err)
{
    ssize_t nread;
    time_t  current;
    time_t  end;

    current = (time_t)(clock() * MILLI_SEC / CLOCKS_PER_SEC);
    end     = current + TIMEOUT;
    do
    {
        errno   = 0;
        current = (time_t)(clock() * MILLI_SEC / CLOCKS_PER_SEC);
        nread   = read(fd, buf, size);
        if(nread == 0)
        {
            break;
        }
        if(nread == -1)
        {
            if(errno == EINTR || errno == EAGAIN)
            {
                errno = 0;
                continue;
            }
            perror("read_fully error");
            *err = errno;
            return -1;
        }
    } while(nread < (ssize_t)size && current <= end);

    return nread;
}

ssize_t write_fully(int fd, void *buf, ssize_t size, int *err)
{
    time_t      current;
    time_t      end;
    ssize_t     bytes_wrote;
    const char *ptr;

    ptr         = (char *)buf;
    bytes_wrote = 0;
    current     = (time_t)(clock() * MILLI_SEC / CLOCKS_PER_SEC);
    end         = current + TIMEOUT;
    do
    {
        ssize_t result;
        current = (time_t)(clock() * MILLI_SEC / CLOCKS_PER_SEC);

        result = write(fd, ptr + bytes_wrote, (size_t)(size - bytes_wrote));
        if(result == 0)
        {
            break;
        }
        if(result == -1)
        {
            *err = errno;
            return -1;
        }
        bytes_wrote += result;
    } while(bytes_wrote < size && current <= end);
    return bytes_wrote;
}

ssize_t copy(int from, int to, int *err)
{
    char    buf[BUFFER_SIZE];
    ssize_t nread;
    ssize_t bytes_wrote;

    memset(&buf, 0, BUFFER_SIZE);
    do
    {
        errno = 0;
        nread = read(from, buf, BUFFER_SIZE);
        if(nread == 0)
        {
            return -1;
        }
        if(nread == 1 && buf[0] == '\0')
        {
            printf("server signal exit\n");
            return -1;
        }
        if(nread < 0)
        {
            if(errno == EAGAIN)
            {
                continue;
            }
            perror("read error\n");
            goto error;
        }

        bytes_wrote = 0;
        do
        {
            ssize_t twrote;
            size_t  remaining;

            remaining = (size_t)(nread - bytes_wrote);
            errno     = 0;
            twrote    = write(to, buf + bytes_wrote, remaining);
            if(twrote < 0)
            {
                if(errno == EAGAIN)
                {
                    errno = 0;
                    continue;
                }
                goto error;
            }

            bytes_wrote += twrote;
        } while(bytes_wrote != nread);
    } while(nread != 0);
    return 0;

error:
    *err = errno;
    return -1;
}
