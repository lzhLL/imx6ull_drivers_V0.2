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
 * ./ap3216cAPP <file_name>
 * ./ap3216cAPP /dev/ap3216c
 * ./ap3216cAPP /dev/ap3216c
*/



int main(int argc, char const *argv[])
{
    int fd = 0, err = 0;
    const char* filename = NULL;
    int data = 0;

    if(argc != 2) {
        printf("Error Usage!\r\n");
        return -1;
    }

    filename = argv[1];

    fd = open(filename, O_RDWR);
    if(fd < 0) {
        printf("file %s open failed!\r\n", filename);
        return -1;
    }

    err = read(fd, &data, sizeof(data));

    close(fd);
    return 0;
}

