#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>


/*
 * argc:参数个数
 * argv[]:每个参数具体的内容，以字符串形式
 * ./chrdevbaseApp <file_name> <1:2> 1表示读，2表示写
 * ./chrdevbaseAPP /dev/chrdevbase 1        表示从驱动里面读数据
 * ./chrdevbaseAPP /dev/chrdevbase 2        表示向驱动里面写数据
*/

int main(int argc, char const *argv[])
{
    int fd = 0, ret = 0;
    const char *file_name = argv[1];
    char read_buf[128] = {0} , write_buf[128] = {0};
    static char usr_data[] = {"user data!"};


    if (argc != 3)
    {
        printf("Error usage!\r\n");
        printf("tips:./chrdevbaseAPP /dev/chrdevbase x. 1:read data from drv;2:write data to drv\r\n");
        return -1;
    }
    
    fd = open(file_name, O_RDWR);
    if (fd < 0)
    {
        printf("can't open %s file\r\n", file_name);
        return -1;
    }
    
    if (atoi(argv[2]) == 1)
    {
        /* read */
        ret = read(fd, read_buf, sizeof(read_buf));
        if (ret < 0)
        {
            printf("read %s file failed!\r\n", file_name);
            return -1;
        }
        else
        {
            printf("APP read data from KERNEL:%s\r\n", read_buf);
        }
        
    }
    
// char *strcpy(char *dest, const char *src);
    if (atoi(argv[2]) == 2)
    {
        /* write */
        strcpy(write_buf, usr_data);
        ret = write(fd, write_buf, sizeof(write_buf));
        if (ret < 0) 
        {
            printf("write file %s failed!\r\n", file_name);
        }
        else
        {
            /* code */
        }
        
    }

    
    close(fd);

    return 0;
}
