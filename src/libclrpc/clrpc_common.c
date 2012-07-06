
#ifdef WIN32
#include <winsock2.h>
#include <windows.h>
#include <ws2tcpip.h>
#endif

#include "event2/event-config.h"

#include <sys/types.h>
#include <sys/stat.h>
#ifdef _EVENT_HAVE_SYS_TIME_H
#include <sys/time.h>
#endif
#include <sys/queue.h>
#ifndef WIN32
#include <sys/socket.h>
#include <signal.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#endif
#ifdef _EVENT_HAVE_NETINET_IN6_H
#include <netinet/in6.h>
#endif
#ifdef HAVE_NETDB_H
#include <netdb.h>
#endif
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <assert.h>

#include "event2/event.h"
#include "event2/event_compat.h"
#include "event2/util.h"
#include "event2/listener.h"
#include "event2/bufferevent.h"
//#include "log-internal.h"
#include "clrpc_common.h"

//#include "util-internal.h"


/* Helper: return the port that a socket is bound on, in host order. */
int
regress_get_socket_port(evutil_socket_t fd)
{
	struct sockaddr_storage ss;
	ev_socklen_t socklen = sizeof(ss);
	if (getsockname(fd, (struct sockaddr*)&ss, &socklen) != 0)
		return -1;
	if (ss.ss_family == AF_INET)
		return ntohs( ((struct sockaddr_in*)&ss)->sin_port);
	else if (ss.ss_family == AF_INET6)
		return ntohs( ((struct sockaddr_in6*)&ss)->sin6_port);
	else
		return -1;
}

int
regress_get_listener_addr(struct evconnlistener *lev,
    struct sockaddr *sa, ev_socklen_t *socklen)
{
	evutil_socket_t s = evconnlistener_get_fd(lev);
	if (s <= 0)
		return -1;
	return getsockname(s, sa, socklen);
}

struct evrpc_pool*
rpc_pool_with_connection(ev_uint16_t port)
{
   struct evhttp_connection *evcon;
   struct evrpc_pool *pool;

   pool = evrpc_pool_new(NULL);
   assert(pool != NULL);

   evcon = evhttp_connection_new("127.0.0.1", port);
   assert(evcon != NULL);

   evrpc_pool_add_connection(pool, evcon);

   return (pool);
}

