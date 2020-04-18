#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>

int main(int argc, char **argv)
{
    int fd = -1;
    int val = 0;

    /* 1.判断参数 */
    if (argc < 1)
    {
        printf ("Usage:%s 0/1\n", argv[0]);
        return -1;
    }

    /* 2.打开文件 */
    fd = open("/dev/alpha_led0", O_RDWR);
    if (fd < 0)
    {
        printf ("/dev/alpha_led0 open failed!\n");
        return -1;
    }

    /* 3.对驱动进行写操作 */
    if (!strcmp(argv[1], "0"))
    {
        val = 0;
        write(fd, &val, 1);
    }
    else if (!strcmp(argv[1], "1"))
    {
        val = 1;
        write(fd, &val, 1);
    }
    else
    {
        printf("Usage:%s 0/1\n", argv[0]);
    }

    close(fd);

    return 0;
}

