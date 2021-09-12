#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <assert.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/time.h>

#define MAXFD 10

void fds_add(int fds[], int fd)
{
    for (int i = 0; i < MAXFD; i++)
    {
        if (fds[i] == -1)
        {
            fds[i] = fd;
            break;
        }
    }
}

int main(int argc, char const *argv[])
{
    int sockfd = socket(PF_INET, SOCK_STREAM, 0);
    assert(sockfd != -1);
    printf("sockfd = %d\n", sockfd);

    struct sockaddr_in saddr;
    memset(&saddr, 0, sizeof(saddr));
    saddr.sin_family = AF_INET;
    saddr.sin_port = htons(6000);
    saddr.sin_addr.s_addr = inet_addr("127.0.0.1");

    int res = bind(sockfd, (struct sockaddr *)&saddr, sizeof(saddr));
    assert(res != -1);

    int lsn = listen(sockfd, 5);
    assert(lsn != -1);

    fd_set readfds; //sever-end read file descriptors
    int fds[MAXFD] = {-1}; //init fd
    fds_add(fds, sockfd);

    while (1)
    {
        FD_ZERO(&readfds);
        int maxfd = -1;
        for (int i = 0; i < MAXFD; i++)
        {
            if (fds[i] == -1) continue;
            FD_SET(fds[i], &readfds);

            if (fds[i] > maxfd) maxfd = fds[i];
        }

        struct timeval tv = {5, 0}; //set timeout of 5 seconds.

        int n = select(maxfd + 1, &readfds, NULL, NULL, &tv);
        if (n == -1) {
            perror("select error");
        } else if (n == 0) {
            printf("timeout.\n");
        } else {
            for (int i = 0; i < MAXFD; i++)
            {
                if (fds[i] == -1) continue;
                if (FD_ISSET(fds[i], &readfds))
                {
                    printf("fds = %d\n", fds[i]);
                    if (fds[i] == sockfd)
                    {
                        struct sockaddr_in caddr;
                        socklen_t len = sizeof(caddr);

                        int c = accept(sockfd, (struct sockaddr *) &caddr, &len); //Accept new client connections.
                        if (c < 0)
                        {
                            perror("accpet error.");
                            continue;
                        }

                        printf("accept c = %d\n", c);
                        fds_add(fds, c);
                    } else { //Receive data when an existing client sends data
                        char buff[128] = {0};
                        int res = recv(fds[i], buff, 127, 0);
                        if (res <= 0)
                        {
                            close(fds[i]);
                            fds[i] = -1;
                            printf("one client over\n");
                        } else {
                            printf("recv(%d) = %s\n", fds[i], buff); //Output client send information.
                            send(fds[i], "OK", 2, 0); //Reply message to client.
                        }
                        
                    }
                    
                }
                
            }
            
        }
        
    }
    
    return 0;
}


