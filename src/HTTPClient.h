/* HTTPClient.h */

#ifndef _HTTP_CLIENT_H_
#define _HTTP_CLIENT_H_

#include<string>

#include<arpa/inet.h>
#include<sys/socket.h>

#include"HTTPRequest.h"
#include"HTTPResponse.h"

using namespace std;

class HTTPClient{
	public:
		HTTPClient(string , string , string , string );
		~HTTPClient();

		int setPort(size_t );
		int initSocket(void );
		int run(void );

		int prepareRequest(void );
		int sendRequest(void );
		int handleResponse(void );
		int recvResponse(void );
		int parseResponse(void );
		int processResponse(void );

	private:
		int parseProxyURL(void );
		int parseServerURL(void );

		string getIp(const string& );
		string getMimeType(string );

		int sockfd;
		struct sockaddr_in servAddr;

		string svrUrl;
		string svrIp;
		size_t svrPort;

		Method m_httpMethod;
		string m_fileName;
		string m_remoteFileName;

		string m_httpProxy;
		string m_host;
		bool isProxy;
		
		HTTPRequest* m_httpRequest;
		HTTPResponse* m_httpResponse;

		static const int dnsPort = 9000;
		static const string dnsSvrIp;

		static const int buf_sz = 32;
};

#endif	/* _HTTP_CLIENT_H_ */
