#include <stdio.h>
#include <string.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

// ./first_chrdev_app /dev/first_chrdev read
// ./first_chrdev_app /dev/first_chrdev write
int main(int argc, char const *argv[])
{
    int fd = 0, ret = 0;
    char argv_buf[10] = {0};
    char write_buf[128] = {0}, read_buf[128] = {0};
    const char* dev_pathname = NULL;

    if (argc != 3)
    {
        printf("Input error!\r\n");
        printf("tips:./first_chrdev_app /dev/first_chrdev x. x:read or write.\r\n");
        return -1;
    }

    dev_pathname = argv[1];            //接收第二个参数作为设备路径
    memcpy(argv_buf, argv[2], strlen(argv[2]));

    if ((0 == strcmp("write", argv[2])) ||  (0 == strcmp("read", argv[2])))
    {
        fd = open(dev_pathname, O_RDWR);
        if (fd < 0)
        {
            printf("open %s file failed!\r\n", dev_pathname);
            return -1;
        }
        
        if (strcmp("write", argv_buf) == 0)
        {
            /* user write data to kernel */
            strcpy(write_buf, "user write data");
            ret = write(fd, write_buf, sizeof(write_buf));
            if (ret < 0)
            {
                printf("write %s file failed!\r\n", dev_pathname);
                return -1;
            }
        } 
        else if (strcmp("read", argv_buf) == 0)
        {
            /* user read data from kernel */
            read(fd, read_buf, sizeof(read_buf));
            printf("usr read data:%s\r\n", read_buf);
        }

        close(fd);
    }
    else
    {
        printf("Input error!\r\n");
        printf("tips:./first_chrdev_app /dev/first_chrdev x. x:read or write.\r\n");
        return -1;
    }

    return 0;
}
