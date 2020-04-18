#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>

int main(int argc, char **argv)
{
    int fd_old = -1;
    int fd_new = -1;
    char buf[1024] = {0, };
    int len = 0;
    
    /* 1.判断参数有效性 */
    if (argc != 3)
    {
        printf ("Usage:%s <old-file> <new-file>\n", argv[0]);
        return -1;
    }
    
    /* 2.打开老文件
     * argc[1] :文件路径
     * O_EDONLY:以只读的方式打开该文件
     */
    fd_old = open(argv[1], O_RDONLY);
    if (fd_old < 0)
    {
        printf ("open file:%s failed!\n", argv[1]);
        return -1;
    }
    
    /* 3.创建新文件 
     * argv[2] :新文件的名字
     * O_WRONLY:以只写的方式打开
     * O_CREAT :当文件不存在时，自动创建该文件
     * O_TRUNC :当文件存在时，将长度截为0，清除该文件的内容
     */
    fd_new = open(argv[2], O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH);
    if (fd_new < 0)
    {
        printf ("create new file:%s failed\n", argv[2]);
        return -1;
    }
    
    /* 4.读老文件中的内容，写入写文件中
     * read函数返回读取成功的字节数，返回0表示读到了文件结尾
     * write函数返回写入成功的字节数
     */
    while((len = read(fd_old, buf, 1024)) > 0)
    {
        if (len != write(fd_new, buf, len))
        {
            printf ("new file %s write failed!\n", argv[2]);
            return -1;
        }
    }

    close(fd_old);
    close(fd_new);

    return 0;
}
