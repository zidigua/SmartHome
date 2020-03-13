#ifndef __TCP_CORE_H__
#define __TCP_CORE_H__


int init_tcp(const char *ip,const char *port);
int tcp_send(char *buf);
int tcp_recv(char *buf);
void tcp_close();

#endif
