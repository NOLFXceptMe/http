/* HTTPProxy.h */

#ifndef _HTTP_PROXY_H_
#define _HTTP_PROXY_H_

#include<string>

#include<arpa/inet.h>
#include<sys/socket.h>

#include"HTTP.h"
#include"HTTPRequest.h"
#include"HTTPResponse.h"

using namespace std;

class HTTPProxy{
	public:
		HTTPProxy();
		HTTPProxy(size_t );
		~HTTPProxy();

		int run(void );

	private:
		int initSocketClient(void );
		int initSocketServer(void );
	
		int handleRequest(void );
	
		int recvRequestClient(void );
		int parseRequestClient(void );
		int processRequestClient(void );

		int prepareRequestServer(void );
		int sendRequestServer(void );
		int recvResponseServer(void );
		int parseResponseServer(void );
		int processResponseServer(void );
		
		int prepareResponseClient(void );
		int sendResponseClient(void );

		int parseServerURL(void );
		string getIp(const string& );

		int prepareErrorResponse(size_t );

		/* For the Client */
		size_t proxyPort = 8080;
		int sockfd_cli, newsockfd_cli;		/* Used for client */
		struct sockaddr_in proxyAddr, cliAddr;
		socklen_t cliLen;

		HTTPRequest* m_httpClientRequest;
		HTTPResponse* m_httpClientResponse;

		/* For the Server */
		int sockfd_serv;			/* Used for server */
		struct sockaddr_in servAddr;

		string svrUrl, svrIp;
		size_t svrPort;

		Method m_httpMethod;
		string m_remoteFileName;

		HTTPRequest* m_httpServerRequest;
		HTTPResponse* m_httpServerResponse;

		bool errorState;
		static const int dnsPort = 9000;
		static const string dnsSvrIp;

		static const int buf_sz = 32;
};

#endif	/* _HTTP_PROXY_H_ */
