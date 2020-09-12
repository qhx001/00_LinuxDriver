#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

int main(int argc, char **argv)
{
    int fd = -1;
    int val = 0;

    if (argc != 3)
    {
        printf ("Usage:%s /dev/xxx 0/1\n", argv[0]);
        return -1;
    }

    fd = open(argv[1], O_RDWR);
    if (fd < 0)
    {
        printf ("%s open failed(%d)!\n", argv[1], errno);
        return -1;
    }

    if (!strcmp(argv[2], "0"))
    {
        val = 0;
        write(fd, &val, 1);
    }
    else if (!strcmp(argv[2], "1"))
    {
        val = 1;
        write(fd, &val, 1);
    }
    else
    {
        printf ("Usage:%s /dev/xxx 0/1\n", argv[0]);
    }

    close(fd);

    return 0;
}


