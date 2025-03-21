#include "./inc/webserv.hpp"
#include "./inc/WebServer.hpp"

int main(int argc, char* argv[]) {
  int sv[2];
  struct epoll_event ev = {EPOLLIN | EPOLLOUT | EPOLLET};
  int epoll = epoll_create1(0);
  pthread_t t;

  socketpair(AF_LOCAL, SOCK_STREAM, 0, sv);
  pthread_create(&t, NULL, iothread, sv);
  epoll_ctl(epoll, EPOLL_CTL_ADD, sv[0], &ev);
  while(1){
    struct epoll_event events[64];
    int n = epoll_wait(epoll, events, 64, -1);

    printf("Event count: %d\n", n);

    if(events[0].events == EPOLLIN)
      printf("EPOLLIN only\n\n");
    else
    if(events[0].events == (EPOLLIN|EPOLLOUT))
      printf("EPOLLIN and EPOLLOUT\n\n");
    else
      printf("EPOLLOUT only\n\n");

    char buffer[256];
    ssize_t r;
again:
    r = recv(sv[0], buffer, 256, MSG_DONTWAIT);
    if (r > 0) goto again;
    if (r < 0 && errno == EAGAIN) {
        sleep(1);
        continue;
    }
    break;
  }
  return 0;
}