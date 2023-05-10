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
 * ./led_app <file_name> <1:2> 0表示关灯，1表示开灯
 * ./led_app /dev/dtsled 0        关灯
 * ./led_app /dev/dtsled 1        开灯
*/
#define LED_ON      1
#define LED_OFF     0


int main(int argc, char const *argv[])
{
    int fd;
    int ret = 0;
    const char *file_name = argv[1];
    const char * cmd = argv[2];
    char data_buf[1] = {0};

    if (argc != 3)
    {
        printf("Error usage!\r\n");
        printf("tips:./led_app /dev/dtsled x. 0:close; 1:open\r\n");
        return -1;
    }

    fd = open(file_name, O_RDWR);
    if (fd < 0)
    {
        printf("open %s file failed!\r\n", file_name);
    }
    
#if 0
    printf("cmd = %s\r\n", cmd); 
    if (strcmp(cmd, "1") == 0)
    {
        data_buf[0] = 1;
        write(fd, data_buf, sizeof(data_buf));
    }
    else
    {
        data_buf[0] = 0;
        write(fd, data_buf, sizeof(data_buf));
    }
#else
    data_buf[0] = atoi(argv[2]);

    ret = write(fd, data_buf, sizeof(data_buf));
    if (ret < 0)
    {
        printf("LED control Failed!\r\n");
        close(fd);
        return -1;
    }
#endif

    close(fd);
    return 0;
}

