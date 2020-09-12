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

    if (argc != 2)
    {
        printf ("Usage:%s /dev/xxx\n", argv[0]);
        return -1;
    }

    fd = open(argv[1], O_RDWR);
    if (fd < 0)
    {
        printf ("%s open failed(%d)!\n", argv[1], errno);
        return -1;
    }

    while(1)
    {
        if (read(fd, &val, 4) > 0)
        {
            printf ("Key1[%d],Key2[%d]\n", (val & 0x2) ? 1 : 0, (val & 0x1) ? 1 : 0);
        }
    }

    close(fd);

    return 0;
}


