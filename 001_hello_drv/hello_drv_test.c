#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>

int main(int argc, char **argv)
{
    int fd = -1;
    int len = 0;
    char buf[1024] = {0, };

    /* 1.�жϲ��� */
    if (argc < 2)
    {
        printf ("Usage:%s -w <string>\n", argv[0]);
        printf ("      %s -r\n", argv[0]);
        return -1;
    }

    /* 2.���ļ� */
    fd = open("/dev/hello", O_RDWR);
    if (fd < 0)
    {
        printf ("/dev/hello open failed!\n");
        return -1;
    }

    /* 3.���������ж�д���� */
    if ((!strcmp(argv[1], "-w")) && (argc == 3))
    {
        len = strlen(argv[2]) + 1;
        len = len < 1024 ? len : 1024;
        write(fd, argv[2], len);
    }
    else if ((!strcmp(argv[1], "-r")) && (argc == 2))
    {
        len = read(fd, buf, 1024);
        printf("APP read length:%d, text:%s\n", len, buf);
    }
    else
    {
        ;
    }

    close(fd);

    return 0;
}

