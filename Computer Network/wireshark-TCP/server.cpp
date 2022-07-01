 #include<stdio.h>
 #include<stdlib.h>
 #include<sys/types.h>
 #include<unistd.h>
 #include<sys/socket.h>
 #include<netinet/in.h>
 #include<arpa/inet.h>
 #include<string.h>
 #include<errno.h>
 #include<sys/un.h>
 #include<stdio.h>
 #define N 64
 
 int main(int argc, const char *argv[]) {
     int sockfd, connectfd;
     char buf[N];
     struct sockaddr_un serveraddr, clientaddr;
     socklen_t len = sizeof(clientaddr);
     sockfd = socket(AF_UNIX, SOCK_STREAM, 0);
     
     if(sockfd < 0) 
       { perror("fail to socket"); return -1; }
 
     serveraddr.sun_family = AF_UNIX;
     strcpy(serveraddr.sun_path, "mysocket");
 
     if(bind(sockfd, (struct sockaddr*)&serveraddr, sizeof(serveraddr)) < 0)
       { perror("fail to bind"); return -1; }
 
     if(listen(sockfd, 5) < 0)
       { perror("fail to listen"); return -1; }
 
     if((connectfd = accept(sockfd, (struct sockaddr*)&clientaddr, &len)) < 0)
       { perror("fail to accept"); return -1; }
 
     while(1) {
         if(recv(connectfd, buf, N, 0) < 0)
           { perror("fail to recv"); return -1; }
         
         if(strncmp(buf, "quit", 4) == 0) break;
         
         buf[strlen(buf) - 1] = '\0';
         printf("buf:%s\n", buf);
         strcat(buf, "+++***---");
         
         if(send(connectfd, buf, N, 0) < 0)
           { perror("fail to send"); return -1; }
     }
     close(sockfd);
     return 0;
 }
