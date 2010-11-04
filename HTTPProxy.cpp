/* HTTPProxy.cpp */

/* An HTTP Proxy written in C++ */
/* Sweta Yamini         06CS3008
   Naveen Kumar Molleti 06CS3009        */

/*
*/

#include<iostream>
#include<fstream>
#include<string>
#include<cstring>
#include<stdexcept>
#include<sstream>

#include<stdlib.h>
#include<errno.h>
#include<fcntl.h>
#include<arpa/inet.h>
#include<sys/socket.h>

#include"HTTPProxy.h"

using namespace std;

const string HTTPProxy::dnsSvrIp = "127.0.0.1";

HTTPProxy::HTTPProxy()
{
}

HTTPProxy::HTTPProxy(size_t port):proxyPort(port)
{
}

HTTPProxy::~HTTPProxy()
{
	close(sockfd_cli);
	close(newsockfd_cli);
	close(sockfd_serv);
}

int HTTPProxy::run()
{
	string funcName = "run: ";

	if(initSocketClient()){
		cerr<<funcName<<"Failed to initialize socket for client"<<endl;
		return -1;
	}

	while(1){
		cliLen = sizeof(cliAddr);

		if((newsockfd_cli = accept(sockfd_cli, (struct sockaddr *)&cliAddr, &cliLen))<0){
			cerr<<funcName<<"Accept call failed"<<endl;
			return -1;
		}

		if(fork() == 0){
			if(handleRequest()){
				cerr<<funcName<<"Failed handling request"<<endl;
				exit(-1);
			}

			exit(0);
		}
		close(newsockfd_cli);
	}

	return 0;
}

int HTTPProxy::initSocketClient()
{
	string funcName = "initSocketClient: ";

	if((sockfd_cli = socket(AF_INET, SOCK_STREAM, 0))<0){
		cerr<<funcName<<"Failed to create socket"<<endl;
		return -1;
	}

	proxyAddr.sin_family = AF_INET;
	proxyAddr.sin_port = htons(proxyPort);
	proxyAddr.sin_addr.s_addr = INADDR_ANY;

	/* Bind */
	if((bind(sockfd_cli, (struct sockaddr *)&proxyAddr, sizeof(proxyAddr)))<0){
		cerr<<funcName<<"Failed to bind to port "<<proxyPort<<endl;
		return -1;
	}

	/* Set to listen on control socket */
	if(listen(sockfd_cli, 5)){
		cerr<<funcName<<"Listen on port "<<proxyPort<<" failed"<<endl;
		return -1;
	}

	return 0;
}

int HTTPProxy::initSocketServer()
{
	string funcName = "initSocketServer: ";
	if(errorState)
		return 0;
	
	if((sockfd_serv = socket(AF_INET, SOCK_STREAM, 0))<0){
		cerr<<funcName<<"Failed to create socket"<<endl;
		return -1;
	}

	cout<<"Connecting to '"<<svrIp<<":"<<svrPort<<"'"<<endl<<endl;
	servAddr.sin_family = AF_INET;
	servAddr.sin_port = htons(svrPort);
	servAddr.sin_addr.s_addr = inet_addr(svrIp.c_str());

	if((connect(sockfd_serv, (struct sockaddr *)&servAddr, sizeof(servAddr)))<0){
		cerr<<funcName<<"Failed to connect"<<endl;
		return -1;
	}

	return 0;
}

int HTTPProxy::handleRequest()
{
	string funcName = "handleRequest: ";

	m_httpClientRequest = new HTTPRequest();
	m_httpClientResponse= new HTTPResponse();
	m_httpServerRequest = new HTTPRequest();
	m_httpServerResponse = new HTTPResponse();
	errorState = false;

	if(recvRequestClient()){
		cerr<<funcName<<"Receiving request failed"<<endl;
		return -1;
	}

	m_httpClientRequest->printRequest();

	if(parseRequestClient()){
		cerr<<funcName<<"Parsing request failed"<<endl;

		prepareErrorResponse(400);
		errorState = true;
	}
	
	if(processRequestClient()){
		cerr<<funcName<<"Processing request failed"<<endl;

		prepareErrorResponse(400);
		errorState = true;
	}

	if(prepareRequestServer()){
		cerr<<funcName<<"Preparing request failed"<<endl;
	}

	if(!errorState)
		m_httpServerRequest->printRequest();

	if(initSocketServer()){
		cerr<<funcName<<"Initializing server side socket failed"<<endl;
		prepareErrorResponse(502);
		errorState = true;
	}

	if(sendRequestServer()){
		cerr<<funcName<<"Sending request failed"<<endl;

		prepareErrorResponse(502);
		errorState = true;
	}

	if(recvResponseServer()){
		cerr<<funcName<<"Receiving response failed"<<endl;

		prepareErrorResponse(502);
		errorState = true;
	}

	if(!errorState)
		m_httpServerResponse->printResponse();

	if(parseResponseServer()){
		cerr<<funcName<<"Parsing response failed"<<endl;

		prepareErrorResponse(502);
		errorState = true;
	}

	if(processResponseServer()){
		cerr<<funcName<<"Processing response failed"<<endl;
		return -1;
	}

	if(prepareResponseClient()){
		cerr<<funcName<<"Preparing response failed"<<endl;
		return -1;
	}

	m_httpClientResponse->printResponse();

	if(sendResponseClient()){
		cerr<<funcName<<"Sending response failed"<<endl;
		return -1;
	}

	delete m_httpClientRequest;
	delete m_httpClientResponse;
	delete m_httpServerRequest;
	delete m_httpServerResponse;

	errorState = false;
	return 0;
}

int HTTPProxy::recvRequestClient()
{
	string funcName = "recvRequestClient: ";
	int recvLength;
	char* buf = new char[buf_sz];
	memset(buf, '\0', buf_sz);

	if(!(recvLength = recv(newsockfd_cli, buf, buf_sz, 0))){
		cerr<<funcName<<"Failed to receive request (blocking)"<<endl;
		return -1;
	}
	m_httpClientRequest->addData(buf, recvLength);

	while(1){
		memset(buf, '\0', buf_sz);
		recvLength = recv(newsockfd_cli, buf, buf_sz, MSG_DONTWAIT);

		if(recvLength < 0){

			if(errno == EWOULDBLOCK || errno == EAGAIN){
				break;
			} else {
				cerr<<funcName<<"Failed receiving request (nonblocking)"<<endl;
				return -1;
			}
		}
	
		m_httpClientRequest->addData(buf, recvLength);

		if(recvLength<buf_sz)
			break;
	};

	return 0;
}

int HTTPProxy::parseRequestClient()
{
	string funcName = "parseRequestClient: ";

	if(m_httpClientRequest->parseRequest()){
		cerr<<funcName<<"Failed parsing request"<<endl;
		return -1;
	}

	return 0;
}

int HTTPProxy::processRequestClient()
{
	string funcName = "processRequestClient: ";
	if(errorState)
		return 0;

	svrUrl = m_httpClientRequest->getURL();
	if(parseServerURL())
		return -1;
	
	return 0;
}

int HTTPProxy::prepareRequestServer()
{
	string funcName = "prepareRequestServer: ";
	if(errorState)
		return 0;

	m_httpServerRequest->setMethod(m_httpClientRequest->getMethod());
	m_httpServerRequest->setProtocol(m_httpClientRequest->getProtocol());
	m_httpServerRequest->setURL(m_remoteFileName);
	m_httpServerRequest->setHTTPHeaderVector(m_httpClientRequest->getHTTPHeaderVector());
	m_httpServerRequest->setRequestBody(m_httpClientRequest->getRequestBodyPtr());

	if(m_httpServerRequest->prepareRequest()){
		cerr<<funcName<<"Failed to prepare Server Request"<<endl;
		return -1;
	}

	return 0;
}

int HTTPProxy::sendRequestServer()
{
	string funcName = "sendRequestServer: ";
	if(errorState)
		return 0;

	size_t requestSize = m_httpServerRequest->getRequestSize();
	string* requestDataPtr = m_httpServerRequest->getRequestDataPtr();

	char *buf = new char[requestSize];
	memset(buf, '\0', requestSize);
	memcpy(buf, requestDataPtr->data(), requestSize);

	if((send(sockfd_serv, buf, requestSize, 0))<0){
		cerr<<funcName<<"Sending request failed"<<endl;
	}

	delete buf;

	return 0;
}

int HTTPProxy::recvResponseServer()
{
	string funcName = "recvResponseServer: ";
	if(errorState)
		return 0;

	int recvLength;
	char* buf = new char[buf_sz];
	memset(buf, '\0', buf_sz);

	if(!(recvLength = recv(sockfd_serv, buf, buf_sz, 0))){
		cerr<<funcName<<"Failed to receive request (blocking)"<<endl;
		return -1;
	}
	m_httpServerResponse->addData(buf, recvLength);

	while(1){
		memset(buf, '\0', buf_sz);
		recvLength = recv(sockfd_serv, buf, buf_sz, MSG_DONTWAIT);

		if(recvLength < 0){
			if(errno == EWOULDBLOCK || errno == EAGAIN){
				break;
			}

			cerr<<funcName<<"Failed receiving reponse (nonblocking)"<<endl;
			return -1;	
		}
	
		m_httpServerResponse->addData(buf, recvLength);

		if(recvLength<buf_sz)
			break;
	};

	delete buf;

	return 0;
}

int HTTPProxy::parseResponseServer()
{
	string funcName = "parseResponseServer: ";
	if(errorState)
		return 0;
	
	if(m_httpServerResponse->parseResponse()){
		cerr<<funcName<<"Failed parsing response"<<endl;
		return -1;
	}

	return 0;
}

int HTTPProxy::processResponseServer()
{
	return 0;
}


int HTTPProxy::prepareResponseClient()
{
	string funcName = "prepareResponseClient: ";
	if(errorState)
		return 0;

	m_httpClientResponse->setProtocol(m_httpServerResponse->getProtocol());
	m_httpClientResponse->setStatusCode(m_httpServerResponse->getStatusCode());
	m_httpClientResponse->setReasonPhrase();

	m_httpClientResponse->setHTTPHeader("Via", "Awesome Proxy Server");
	m_httpClientResponse->setHTTPHeaderVector(m_httpServerResponse->getHTTPHeaderVector());

	m_httpClientResponse->setResponseBody(m_httpServerResponse->getResponseBodyPtr());

	if(m_httpClientResponse->prepareResponse()){
		cerr<<funcName<<"Failed to prepare response"<<endl;
		return -1;
	}

	return 0;
}

int HTTPProxy::sendResponseClient()
{
	string funcName = "sendResponseClient: ";

	size_t responseSize = m_httpClientResponse->getResponseSize();
	string* responseDataPtr = m_httpClientResponse->getResponseDataPtr();

	char *buf = new char[responseSize];
	memset(buf, '\0', responseSize);
	memcpy(buf, responseDataPtr->data(), responseSize);

	if((send(newsockfd_cli, buf, responseSize, 0))<0){
		cerr<<funcName<<"Sending response failed"<<endl;
	}

	delete buf;

	return 0;
}

int HTTPProxy::parseServerURL()
{
	/* Set the svrIp, and svrPort */
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
			svrPort = 80;
		}
		
		bool isIp = (strpbrk(svrName.c_str(), alphabet.c_str()) == NULL);

		if(isIp){
			svrIp = svrName;
		}else{
			svrIp = getIp(svrName);
		}

		if(svrIp == "")
			return -1;
	}else{
		cerr<<"Parse Error!"<<endl;
		return -1;
	}

	posParserOld = posParserNew;
	m_remoteFileName = svrUrl.substr(posParserOld);

	return 0;
}

string HTTPProxy::getIp(const string& svrName)
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
		if(errno == ETIMEDOUT || errno == EAGAIN)
			cerr<<"DNS Server did not respond"<<endl;

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

int HTTPProxy::prepareErrorResponse(size_t statusCode)
{
	string funcName = "prepareErrorResponse: ";
	ostringstream os;

	m_httpClientResponse->setProtocol(m_httpServerResponse->getProtocol());
	m_httpClientResponse->setStatusCode(statusCode);
	m_httpClientResponse->setReasonPhrase();

	m_httpClientResponse->setHTTPHeader("Via", "Awesome Proxy Server");
	m_httpClientResponse->setHTTPHeader("Connection", "close");

	string errResponse = "Error: " + m_httpClientResponse->getReasonPhrase();
	os<<errResponse.length();
	m_httpClientResponse->setHTTPHeader("Content-Length", os.str());
	m_httpClientResponse->setHTTPHeader("Content-Type", "text/plain, charset=us-ascii");
	m_httpClientResponse->setResponseBody(&errResponse);

	if(m_httpClientResponse->prepareResponse()){
		cerr<<funcName<<"Failed to prepare response"<<endl;
		return -1;
	}
	return 0;
}
