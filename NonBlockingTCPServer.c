//Author : Gurucharan MK

//System header
#include <strings.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <event.h>
#include <event2/listener.h>


//C header
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

 
#define SVR_IP                         "127.0.0.1"
#define SVR_PORT                       8787

 



static void onRead(struct bufferevent *bev, void *ctx) {
    char buff[1024*8];
    int len;
 
    bzero((char *) &buff, sizeof(buff));
    char  ret[128];
    len = bufferevent_read(bev, buff, sizeof(buff));
    bzero((char *) &ret, sizeof(ret));
    strcpy(ret, "BadRequest");
    
    if (strncmp (buff, "GETMESSAGE", len - 1) == 0){
        //printf("Closing !!!! \n");
        bzero((char *) &ret, sizeof(ret));
        strcpy(ret, "The message is ABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWX");
    }
    ret[strlen(ret)] = '\n';
    ret[strlen(ret)+1] = '\0';
    /* Write data */
    bufferevent_write(bev, ret, strlen(ret));
    
    if (strncmp (buff, "BYE", strlen(buff) - 1) == 0){
        //printf("Closing !!!! \n");
        //ret = "CLOSING CONNECTION";
      bufferevent_free(bev);
    }

}
 

static void onWrite(struct bufferevent *bev, void *ctx) {
    //printf("write finished\n");
}
 
static void onEvent(struct bufferevent *bev, short events, void *ctx) {
    if (events & BEV_EVENT_EOF) {
        bufferevent_free(bev);
    }
}
 
static void onAccept(struct evconnlistener *lev, evutil_socket_t fd,
    struct sockaddr *sa, int socklen, void *ctx) {
    struct event_base *evbase = ctx;
    struct bufferevent *bev;
    struct timeval tv;
 
     bev = bufferevent_socket_new(evbase, fd, BEV_OPT_CLOSE_ON_FREE);
    if (bev == NULL) {
        printf("bufferevent_socket_new() failed\n");
        evutil_closesocket(fd);
        return;
    }
  
    bufferevent_setcb(bev, onRead, onWrite, onEvent, NULL);
    bufferevent_enable(bev, EV_READ|EV_WRITE);
}
 
int main(int argc, char *argv[]){
    struct event_base *evbase;
    struct sockaddr_in sin;
    struct evconnlistener *lev;

    int port = SVR_PORT;

    if (argc < 2) {
         printf("Using default port 8787\n"); 

    }
    else{
        port = atoi(argv[1]);
    }
 
    
    if ((evbase = event_base_new()) == NULL) {
        perror("event_base_new() failed\n");
        return -1;
    }
 
    
    memset(&sin, 0, sizeof(sin));
    sin.sin_family = AF_INET;
    sin.sin_addr.s_addr = inet_addr(SVR_IP);
    sin.sin_port = htons(SVR_PORT);
 
    
    lev = evconnlistener_new_bind(evbase, onAccept, evbase,
            LEV_OPT_REUSEABLE | LEV_OPT_CLOSE_ON_FREE, -1,
            (struct sockaddr *) &sin, sizeof(sin));
 
    if (lev == NULL) {
        perror("bind() failed\n");
        return -1;
    } else {
        printf("Server is listening on the port %d", port);
    }
 
    
    event_base_dispatch(evbase);
    
 
    return 0;
  }
