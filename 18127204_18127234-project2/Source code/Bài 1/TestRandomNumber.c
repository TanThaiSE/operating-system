#include<stdio.h>
#include<stdlib.h>
#include<errno.h>
#include<fcntl.h>
#include<string.h>
#include<unistd.h>

int main(){
    int ret, fd;
    char numReceive[5];
    fd = open("/dev/RandomMachine", O_RDWR); //Open the device to read
    if (fd < 0){
        perror("Failed to open the device..\n");
        return errno;
    }
    printf("Read device\n"); //Read number from generator
    ret = read(fd, numReceive, 1);
    if (ret < 0){
        perror("Failed to read from the device...\n");
        return errno;
    }
    printf("A received number is %u \n", numReceive[0]);
    return 0;
}
