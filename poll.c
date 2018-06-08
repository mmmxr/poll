#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <poll.h>

#define MAX 1024
typedef struct pollfd pollfd;
void Add(int fd,pollfd* fd_list,int size)
{
    int i=0;
    for(i=0;i<size;i++)
    {
        if(fd_list[i].fd==-1)
        {
            fd_list[i].fd=fd;
            fd_list[i].events=POLLIN;
            break;
        }
    }
}
void Init(pollfd* fd_list,int size)
{
    int i=0;
    for(i=0;i<size;i++)
    {
        fd_list[i].fd=-1;
        fd_list[i].events=0;
        fd_list[i].revents=0;
    }
}
int startup(int port)
{
    int sock=socket(AF_INET,SOCK_STREAM,0);
    if(sock<0)
    {
        perror("socket");
        exit(1);
    }
    struct sockaddr_in local;
    local.sin_family=AF_INET;
    local.sin_addr.s_addr=htonl(INADDR_ANY);
    local.sin_port=htons(port);

    if(bind(sock,(struct sockaddr*)&local,sizeof(local))<0)
    {
        perror("bind");
        exit(2);
    }
    if(listen(sock,5)<0)
    {
        perror("listen");
        exit(3);
    }
    return sock;
}
int main(int argc,char* argv[])
{
    if(argc!=2)
    {
        printf("Usage:[%s port]\n",argv[0]);
        return 1;
    }

    int new_sock=startup(atoi(argv[1]));
    pollfd fd_list[MAX];
    Init(fd_list,sizeof(fd_list)/sizeof(pollfd));//？??
    Add(new_sock,fd_list,sizeof(fd_list)/sizeof(pollfd));

    for(;;)
    {
        int ret=poll(fd_list,sizeof(fd_list)/sizeof(pollfd),1000);
        if(ret<0)
        {
            perror("poll");
            continue;
        }
        else if(ret==0)
        {
            printf("timeout...\n");
            continue;
        }
        size_t i=0;
        for(i=0;i<sizeof(fd_list)/sizeof(pollfd);i++)
        {
            if(fd_list[i].fd==-1)
            {
                continue;
            }
            if(!(fd_list[i].revents & POLLIN))
            {
                continue;
            }
            if(fd_list[i].fd==new_sock)
            {
                struct sockaddr_in client;
                socklen_t len=sizeof(client);
                int connect_sock=accept(new_sock,(struct sockaddr*)&client,&len);
                if(connect_sock<0)
                {
                    perror("accpet");
                    continue;
                }
                Add(connect_sock,fd_list,sizeof(fd_list)/sizeof(pollfd));
            }
            else
            {
                char buf[MAX];
                ssize_t s=read(fd_list[i].fd,buf,sizeof(buf)-1);
                if(s<0)
                {
                    perror("read");
                    continue;
                }
                else if(s==0)
                {
                    printf("client quit!\n");
                    close(fd_list[i].fd);
                    fd_list[i].fd=-1;
                }
                else
                {
                    printf("client say# %s\n",buf);
                    //write(fd_list[i].fd,buf,strlen(buf));
                }
            }
        }

    }
}
