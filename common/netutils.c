#include "netutils.h"

ssize_t readn(int fd, int n, char *buff)
{
    ssize_t total_read = 0;
    ssize_t bytes_left = n;

    while(bytes_left != 0)
    {
        ssize_t bytes_read = read(fd, buff + total_read, bytes_left);
        if(bytes_read == 0)
        {
            break;
        }
        else if(bytes_read < 0)
        {
            if(errno == EINTR)
            {
                continue;
            }
            else
            {
                perror("read");
                break;
            }
        }

        total_read += bytes_read;
        bytes_left -= bytes_read;
    }

    return total_read;
}

ssize_t writen(int fd, int n, char *buff)
{
    ssize_t total_write = 0;
    ssize_t bytes_left = n;

    while(bytes_left != 0)
    {
        ssize_t bytes_write = write(fd, buff + total_write, bytes_left);
        if(bytes_write == 0)
        {
            break;
        }
        else if(bytes_write < 0)
        {
            if(errno == EINTR)
            {
                continue;
            }
            else
            {
                perror("write");
                break;
            }
        }

        total_write += bytes_write;
        bytes_left -= bytes_write;
    }

    return total_write;
}