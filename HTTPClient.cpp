/* HTTPClient.cpp */

/* An HTTP Server written in C++ */
/* Sweta Yamini		06CS3008	
   Naveen Kumar Molleti 06CS3009	*/

/* 
Connect to server
Prepare request
Send request
Receive response
Close connection
*/

#include<iostream>
#include<string>
#include<cstring>
#include<cstdlib>
#include<fstream>
#include<sstream>

#include<errno.h>
#include<fcntl.h>
#include<unistd.h>
#include<arpa/inet.h>
#include<sys/socket.h>

#include"HTTPClient.h"

extern int errno;

using namespace std;

const string HTTPClient::dnsSvrIp = "127.0.0.1";

HTTPClient::HTTPClient(string method, string url, string fileName, string proxy):svrUrl(url), svrPort(80), m_fileName(fileName)
{
	if(method == "GET")
		m_httpMethod = GET;
	else if(method == "PUT")
		m_httpMethod = PUT;
	else
		cerr<<"Unsupported HTTP Method"<<endl;
	
	if(proxy == "direct"){
		isProxy = false;
		m_httpProxy = "";
	}else{
		isProxy = true;
		m_httpProxy = proxy;
	}
}

HTTPClient::~HTTPClient()
{
	close(sockfd);
}

int HTTPClient::run()
{
	string funcName = "run: ";

	m_httpRequest = new HTTPRequest();
	m_httpResponse = new HTTPResponse();

	if(prepareRequest()){
		cerr<<funcName<<"Failed to prepare request"<<endl;
		return -1;
	}

	m_httpRequest->printRequest();

	if(initSocket()){
		cerr<<funcName<<"Failed to initialize socket"<<endl;
		return -1;
	}
	
	if(sendRequest()){
		cerr<<funcName<<"Failed to send request"<<endl;
	}

	if(handleResponse()){
		cerr<<funcName<<"Failed to receive response"<<endl;
	}

	delete m_httpRequest;
	delete m_httpResponse;

	return 0;
}

int HTTPClient::parseProxyURL()
{
	if(parseServerURL())
		return -1;

	size_t posColon = m_httpProxy.find_first_of(":");
	svrIp = m_httpProxy.substr(0, posColon);
	svrPort = atoi(m_httpProxy.substr(posColon+1).c_str());
	m_remoteFileName = svrUrl;

	return 0;
}

string HTTPClient::getIp(const string& svrName)
{
	string funcName = "getIp: ";
	int sockfd_dns;
	socklen_t dnsServLen;

	struct sockaddr_in dnsServAddr;
	struct timeval recvTimeout;
	size_t nameLength = svrName.length();

	char *buf = new char[nameLength];
	memset(buf, '\0', nameLength);
	memcpy(buf, svrName.data(), nameLength);

	recvTimeout.tv_sec = 2;
	recvTimeout.tv_usec = 0;

	if((sockfd_dns = socket(AF_INET, SOCK_DGRAM, 0))<0){
		cerr<<funcName<<"Unable to open dns socket"<<endl;
		return "";
	}
	setsockopt(sockfd_dns, SOL_SOCKET, SO_RCVTIMEO, &recvTimeout, sizeof(struct timeval));

	dnsServAddr.sin_family = AF_INET;
	dnsServAddr.sin_port = htons(dnsPort);
	dnsServAddr.sin_addr.s_addr = inet_addr(dnsSvrIp.c_str());

	if((sendto(sockfd_dns, buf,nameLength , 0,
		(struct sockaddr *)&dnsServAddr, sizeof(dnsServAddr)))<0){
		cerr<<funcName<<"Sending request failed"<<endl;
	}
	
	memset(buf, '\0', nameLength);		/* Clear the same buffer and use for getting no. of addresses that will be received */

	int data_bytes;
	dnsServLen = sizeof(dnsServAddr);

	if((data_bytes = recvfrom(sockfd_dns, buf, nameLength, 0,  (struct sockaddr *)&dnsServAddr, &dnsServLen))<0){
		if(errno == ETIMEDOUT || errno == EAGAIN){
			cerr<<"DNS Server did not respond"<<endl;
		}

		return "";
	}
	int n_addr = atoi(buf);
	if(!n_addr){
		printf("Couldn't find IP address for given DNS name\n");
		return "";
	}

	delete buf;
	buf = new char[512];			/* 512 bytes */
	if((data_bytes = recvfrom(sockfd_dns, buf, 512, 0,  (struct sockaddr *)&dnsServAddr, &dnsServLen))<0){
		cerr<<"Data receive failed"<<endl;
		return "";
	}
	struct in_addr *addr_list = (struct in_addr *)buf;

	delete buf;
	return inet_ntoa(addr_list[0]);
}

int HTTPClient::parseServerURL()
{
	size_t posParserOld = 0, posParserNew = 0;
	string resType = "http://";
	string alphabet = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";
	string svrName;

	/* Pass over http:// if any */
	if((posParserOld = svrUrl.find_first_of(resType)) != string::npos){
		posParserOld += resType.length();
	}

	/* Extract string till the next / */
	string svrNamePort = "";
	if((posParserNew = svrUrl.find_first_of("/", posParserOld)) != string::npos){
		svrNamePort = svrUrl.substr(posParserOld, posParserNew-posParserOld);

		/* Parse into Server DNS Name/IP and Port(if any) */
		if((posParserOld = svrNamePort.find_first_of(":")) != string::npos){
			svrName = svrNamePort.substr(0, posParserOld);
			svrPort = atoi(svrNamePort.substr(posParserOld+1).c_str());
		}else{
			svrName = svrNamePort;
		}
		m_host = svrName;
		
		if(!isProxy){
			bool isIp = (strpbrk(svrName.c_str(), alphabet.c_str()) == NULL);

			if(isIp){
				svrIp = svrName;
			}else{
				svrIp = getIp(svrName);
			}
			if(svrIp == "")
				return -1;
		}
	}else{
		cerr<<"Parse Error!"<<endl;
		return -1;
	}

	posParserOld = posParserNew;
	m_remoteFileName = svrUrl.substr(posParserOld);

	return 0;
}

int HTTPClient::prepareRequest()
{
	string funcName = "prepareRequest: ";

	ifstream ifs;
	ostringstream os;
	size_t contentLength;

	if(isProxy){
		if(parseProxyURL())
			return -1;
	}else{
		if(parseServerURL())
			return -1;
	}
	
	m_httpRequest->setMethod(m_httpMethod);
	m_httpRequest->setProtocol(HTTP1_0);
	m_httpRequest->setURL(m_remoteFileName);

	m_httpRequest->setHTTPHeader("Host", m_host);
	m_httpRequest->setHTTPHeader("User-Agent", "Awesome HTTP Client");
	m_httpRequest->setHTTPHeader("Content-Type", getMimeType(m_fileName));
	m_httpRequest->setHTTPHeader("Connection", "close");

	switch(m_httpMethod){
		case PUT:
			/* Open file and write to m_httpRequest->m_requestBody */
			ifs.open(m_fileName.c_str(), ifstream::in);
		
			if(ifs.is_open()){
				ifs.seekg(0, ifstream::end);
				contentLength = ifs.tellg();
				ifs.seekg(0, ifstream::beg);
				os<<contentLength;

				if(m_httpRequest->copyFromFile(ifs, contentLength)){
					cerr<<funcName<<"Failed to copy file to Request Body"<<endl;
					return -1;
				}

				m_httpRequest->setHTTPHeader("Content-Length", os.str());
			}else{
				cerr<<"Unable to read from file"<<endl;
				return -1;
			}

			break;
		default:
			break;
	}

	m_httpRequest->prepareRequest();
	return 0;
	
}

int HTTPClient::initSocket()
{
	string funcName = "initSocket: ";
	
	if((sockfd = socket(AF_INET, SOCK_STREAM, 0))<0){
		cerr<<funcName<<"Failed to create socket"<<endl;
		return -1;
	}

	servAddr.sin_family = AF_INET;
	servAddr.sin_port = htons(svrPort);
	servAddr.sin_addr.s_addr = inet_addr(svrIp.c_str());

	if((connect(sockfd, (struct sockaddr *)&servAddr, sizeof(servAddr)))<0){
		cerr<<funcName<<"Failed to connect"<<endl;
		return -1;
	}

	return 0;
}

int HTTPClient::sendRequest()
{
	string funcName = "sendRequest: ";

	size_t requestSize = m_httpRequest->getRequestSize();
	string* requestDataPtr = m_httpRequest->getRequestDataPtr();

	char *buf = new char[requestSize];
	memset(buf, '\0', requestSize);
	memcpy(buf, requestDataPtr->data(), requestSize);

	if((send(sockfd, buf, requestSize, 0))<0){
		cerr<<funcName<<"Sending request failed"<<endl;
	}

	delete buf;

	return 0;
}

int HTTPClient::handleResponse()
{
	string funcName = "handleResponse: ";

	if(recvResponse()){
		cerr<<funcName<<"Failed to receive response"<<endl;
		return -1;
	}

	m_httpResponse->printResponse();

	if(parseResponse()){
		cerr<<funcName<<"Parsing HTTP Response failed"<<endl;
		return -1;
	}

	if(processResponse()){
		cerr<<funcName<<"Parsing HTTP Responsefailed"<<endl;
		return -1;
	}

	return 0;
}

int HTTPClient::recvResponse()
{
	string funcName = "recvResponse: ";
	int recvLength;
	char* buf = new char[buf_sz];
	memset(buf, '\0', buf_sz);

	if(!(recvLength = recv(sockfd, buf, buf_sz, 0))){
		cerr<<funcName<<"Failed to receive response(blocking)"<<endl;
		return -1;
	}
	m_httpResponse->addData(buf, recvLength);

	while(1){
		memset(buf, '\0', buf_sz);
		recvLength = recv(sockfd, buf, buf_sz, MSG_DONTWAIT);

		if(recvLength < 0){
			if(errno == EWOULDBLOCK || errno == EAGAIN){
				cerr<<funcName<<"End of response"<<endl;
			}

			cerr<<funcName<<"Failed receiving response (nonblocking)"<<endl;
			return -1;	
		}
	
		m_httpResponse->addData(buf, recvLength);

		if(recvLength<buf_sz)
			break;
	}

	delete buf;

	return 0;
}

int HTTPClient::parseResponse()
{
	string funcName = "parseResponse: ";
	
	if(m_httpResponse->parseResponse()){
		cerr<<funcName<<"Failed parsing response"<<endl;
		return -1;
	}

	return 0;
}

int HTTPClient::processResponse()
{
	string funcName = "processResponse: ";
	ofstream ofs;

	switch(m_httpMethod){
		case GET:
			ofs.open(m_fileName.c_str(), ofstream::out|ofstream::trunc);

			if(ofs.is_open()){
				if(m_httpResponse->copyToFile(ofs))
					cerr<<funcName<<"Failed to write to file"<<endl;
			}else{
				cerr<<funcName<<"Failed to write to file"<<endl;
				return -1;
			}

			ofs.close();
			break;
		default:
			break;
	}

	return 0;
}

string HTTPClient::getMimeType(string fileName)
{
	size_t extPos = fileName.find_last_of(".");
	string extension;
	string mimeType = "text/plain, charset=us-ascii";

	if(extPos == string::npos){
		extension = "";
	}else{
		extension = fileName.substr(extPos+1);
	}

	/* Compare and return mimetype */
	switch(extension[0]){
		case 'b':
			if(extension == "bmp")
				mimeType = "image/bmp";
			if(extension == "bin")
				mimeType = "application/octet-stream";

			break;
		case 'c':
			if(extension == "csh")
				mimeType = "application/csh";
			if(extension == "css")
				mimeType = "text/css";

			break;
		case 'd':
			if(extension == "doc")
				mimeType = "application/msword";
			if(extension == "dtd")
				mimeType = "application/xml-dtd";
			break;
		case 'e':
			if(extension == "exe")
				mimeType = "application/octet-stream";
			break;
		case 'h':
			if(extension == "html" || extension == "htm")
				mimeType = "text/html";
			break;
		case 'i':
			if(extension == "ico")
				mimeType = "image/x-icon";
			break;
		case 'g':
			if(extension == "gif")
				mimeType = "image/gif";
			break;
		case 'j':
			if(extension == "jpeg" || extension == "jpg")
				mimeType = "image/jpeg";
			break;
		case 'l':
			if(extension == "latex")
				mimeType = "application/x-latex";
			break;
		case 'p':
			if(extension == "png")
				mimeType = "image/png";
			if(extension == "pgm")
				mimeType = "image/x-portable-graymap";
			break;	
		case 'r':
			if(extension == "rtf")
				mimeType  = "text/rtf";
			break;
		case 's':
			if(extension == "svg")
				mimeType = "image/svg+xml";
			if(extension == "sh")
				mimeType = "application/x-sh";
			break;
		case 't':
			if(extension == "tar")
				mimeType = "application/x-tar";
			if(extension == "tex")
				mimeType = "application/x-tex";
			if(extension == "tif" || extension == "tiff")
				mimeType = "image/tiff";
			if(extension == "txt")
				mimeType = "text/plain";
			break;
		case 'x':
			if(extension == "xml")
				mimeType = "application/xml";
			break;
		default:
			break;
	}

	return mimeType;
}
