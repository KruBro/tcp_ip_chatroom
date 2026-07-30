#ifndef NETUTILS_H
#define NETUTILS_H

#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <stdio.h>


ssize_t readn(int fd, int n, char *buff);
ssize_t writen(int fd, int n, char *buff);

#endif