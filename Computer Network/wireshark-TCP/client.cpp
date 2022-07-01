 #include<stdio.h>
 #include<stdlib.h>
 #include<sys/types.h>
 #include<unistd.h>
 #include<sys/socket.h>
 #include<arpa/inet.h>
 #include<netinet/in.h>
 #include<string.h>
 #include<sys/un.h>

 #define N 64
 
 int main(int argc, const char *argv[]) {
     int sockfd;
     struct sockaddr_un serveraddr;
     char buf[N];
 
     sockfd = socket(AF_UNIX, SOCK_STREAM, 0);
     
     if(sockfd < 0)
       { perror("fail to sockfd"); return -1; }
 
     serveraddr.sun_family = AF_UNIX;
     strcpy(serveraddr.sun_path, "mysocket");
 
     if(connect(sockfd, (struct sockaddr*)&serveraddr, sizeof(serveraddr)) < 0)
       { perror("fail to connect"); return -1; }
 
     while(1) {
         printf("<client>");
         fgets(buf, N, stdin);
         
         if(send(sockfd, buf, N, 0) < 0)
           { perror("fail to send"); return -1; }
           
         if(strncmp(buf, "quit", 4) == 0) break;
         
         if(recv(sockfd, buf, N, 0) < 0)
           { perror("fail to recv"); return -1; }
           
         printf("buf:%s\n", buf);
     }
     close(sockfd);
     return 0;
 }
