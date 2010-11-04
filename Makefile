CC=gcc
CPP=g++
CPPFLAGS=-Wall -g
LD=g++
LDFLAGS=

COMMON_OBJS=HTTPRequest.o HTTPResponse.o
SVR_OBJS=HTTPServer.o
CLI_OBJS=HTTPClient.o
PROXY_OBJS=HTTPProxy.o
DNS_OBJS=DNSServer.o
OBJS=$(COMMON_OBJS) $(SVR_OBJS) $(CLI_OBJS) $(PROXY_OBJS)

all: HTTPServerMain HTTPClientMain HTTPProxyMain DNSServer

objs: $(OBJS)

HTTPServerMain: $(COMMON_OBJS) $(SVR_OBJS)

HTTPClientMain: $(COMMON_OBJS) $(CLI_OBJS)

HTTPProxyMain: $(COMMON_OBJS) $(PROXY_OBJS)

clean:
	rm -f *.o HTTPServerMain HTTPClientMain HTTPProxyMain DNSServer
