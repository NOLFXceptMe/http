/* HTTPServer.h */

#ifndef _HTTP_SERVER_H_
#define _HTTP_SERVER_H_

#include<arpa/inet.h>
#include<sys/socket.h>

#include"HTTPRequest.h"
#include"HTTPResponse.h"

#define SVR_ROOT "www"

using namespace std;

class HTTPServer{
	public:
		HTTPServer();
		HTTPServer(int );
		~HTTPServer();

		int run(void );
		int setPort(size_t );
		int initSocket(void );

		int handleRequest(void );
		int recvRequest(void );
		int parseRequest(void );
		int prepareResponse(void );
		int sendResponse(void );
		int processRequest(void );
	
	private:
		string getMimeType(string );

		size_t svrPort;
		int sockfd, newsockfd;
		socklen_t cliLen;
		struct sockaddr_in servAddr, cliAddr;

		string m_url;
		string m_mimeType;
		HTTPRequest* m_httpRequest;
		HTTPResponse* m_httpResponse;
		static const int buf_sz = 32;
};

#endif	/* _HTTP_SERVER_H_ */
