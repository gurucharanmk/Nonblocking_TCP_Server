//Author : Gurucharan MK

#define _GNU_SOURCE

//System header
#include <strings.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <event.h>
#include <event2/listener.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <unistd.h>
#include <sched.h>
#include <errno.h>
#include <pthread.h>

//C header
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

 
#define SVR_IP                         "127.0.0.1"
#define SVR_PORT                       8787

//Read callback
static void onRead(struct bufferevent *bev, void *ctx) {
    char buff[1024*8];
    int len;
 
    bzero((char *) &buff, sizeof(buff));
    char  ret[128];
    len = bufferevent_read(bev, buff, sizeof(buff));
    bzero((char *) &ret, sizeof(ret));
    strcpy(ret, "BadRequest");
    
    if (memcmp (buff, "GETMESSAGE", len - 2) == 0){
        //printf("Closing !!!! \n");
        bzero((char *) &ret, sizeof(ret));
        strcpy(ret, "The message is ABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWX");
    }
    ret[strlen(ret)] = '\n';
    ret[strlen(ret)+1] = '\0';
    /* Write data */
    bufferevent_write(bev, ret, strlen(ret));
    
    if (memcmp (buff, "BYE", strlen(buff) - 2) == 0){
        //printf("Closing !!!! \n");
        //ret = "CLOSING CONNECTION";
      bufferevent_free(bev);
    }

}
 
//Write callback
static void onWrite(struct bufferevent *bev, void *ctx) {
    //printf("write finished\n");
}
 
//Event callback on handiling client socket
static void onEvent(struct bufferevent *bev, short events, void *ctx) {
    if (events & BEV_EVENT_EOF) {
        bufferevent_free(bev);
    }
}
 
//Accept callback
static void onAccept(struct evconnlistener *lev, evutil_socket_t fd,
    struct sockaddr *sa, int socklen, void *ctx) {
    struct event_base *evbase = ctx;
    struct bufferevent *bev;
    
 
     bev = bufferevent_socket_new(evbase, fd, BEV_OPT_CLOSE_ON_FREE);
    if (bev == NULL) {
        printf("bufferevent_socket_new() failed\n");
        evutil_closesocket(fd);
        return;
    }
  
    bufferevent_setcb(bev, onRead, onWrite, onEvent, NULL);
    bufferevent_enable(bev, EV_READ);
}
 
int main(int argc, char *argv[]){
    struct event_base *evbase;
    struct sockaddr_in sin;
    struct evconnlistener *lev;

    int port = SVR_PORT;

    //Icreasing the priority of the current task
    int rc = setpriority(PRIO_PROCESS, 0, -20);

    if (rc < 0) {
        perror("setpriority:");
        rc = setpriority(PRIO_PROCESS, 0, 0);
    }
    

    //Setting the CPU affinity of the current task
    
    cpu_set_t cpu_set;
    int affinity = 0;
    int ncores = 1;

    sched_getaffinity(0, sizeof(cpu_set_t), &cpu_set);
    if (errno)
        perror("couldn't get affinity:");

    if ((ncores = sysconf(_SC_NPROCESSORS_CONF)) <= 0)
        perror("sysconf: couldn't get _SC_NPROCESSORS_CONF");

    CPU_ZERO(&cpu_set);
    CPU_SET(affinity, &cpu_set);

    if (sched_setaffinity(0, sizeof(cpu_set_t), &cpu_set) != 0)
        perror("couldn't change CPU affinity");
    
    //Configuring the port
    if (argc < 2) {
         printf("Using default port 8787\n"); 
    }
    else{
        port = atoi(argv[1]);
    }
 
    //Event base
    if ((evbase = event_base_new()) == NULL) {
        perror("event_base_new() failed\n");
        return -1;
    }
 
    
    memset(&sin, 0, sizeof(sin));
    sin.sin_family = AF_INET;
    sin.sin_addr.s_addr = inet_addr(SVR_IP);
    sin.sin_port = htons(port);
 
    //Create and bind the socket
    lev = evconnlistener_new_bind(evbase, onAccept, evbase,
            LEV_OPT_REUSEABLE | LEV_OPT_CLOSE_ON_FREE, -1,
            (struct sockaddr *) &sin, sizeof(sin));
 
    if (lev == NULL) {
        perror("bind() failed\n");
        return -1;
    } else {
        printf("Server is listening on the port %d", port);
    }
    
    //Event loop starts here 
    event_base_dispatch(evbase);
    
    return 0;
  }
