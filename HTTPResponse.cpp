/* HTTPResponse.cpp */

#include<iostream>
#include<string>
#include<vector>
#include<sstream>
#include<cstdlib>
#include<cstring>

#include"HTTPResponse.h"

HTTPResponse::HTTPResponse(): m_responseBody(""), m_data("")
{
}

HTTPResponse::~HTTPResponse()
{
}

void HTTPResponse::printResponse()
{
	cout<<"---Response Begin---"<<endl<<m_data
		<<"---Response End---"<<endl;
}

void HTTPResponse::addData(const char *buf, const int&len)
{
	m_data.append(buf, len);
}

int HTTPResponse::setProtocol(Protocol protocol)
{
	m_protocol = protocol;
	return 0;
}

Protocol HTTPResponse::getProtocol()
{
	return m_protocol;
}

int HTTPResponse::setStatusCode(size_t statusCode)
{
	m_statusCode = statusCode;
	return 0;
}

size_t HTTPResponse::getStatusCode()
{
	return m_statusCode;
}

int HTTPResponse::setReasonPhrase()
{
	switch(m_statusCode){
		case 200:
			m_reasonPhrase = "OK";
			break;
		case 201:
			m_reasonPhrase = "Created";
			break;
		case 400:
			m_reasonPhrase = "Bad Request";
			break;
		case 403:
			m_reasonPhrase = "Forbidden";
			break;
		case 404:
			m_reasonPhrase = "Not Found";
			break;
		case 411:
			m_reasonPhrase = "Length Required";
			break;
		case 500:
			m_reasonPhrase = "Internal Server Error";
			break;
		case 501:
			m_reasonPhrase = "Not Implemented";
			break;
		case 502:
			m_reasonPhrase = "Bad Gateway";
			break;
		case 505:
			m_reasonPhrase = "HTTP Version Not Supported";
			break;
		default:
			return -1;
			break;
	}

	return 0;
}

string HTTPResponse::getReasonPhrase()
{
	return m_reasonPhrase;
}

int HTTPResponse::setHTTPHeader(string headerName, string headerContent)
{
	m_httpHeaders.push_back(make_pair(headerName, headerContent));
	return 0;
}

string HTTPResponse::getHTTPHeader(string headerName)
{
	for(vector<pair<string, string> >::iterator it = m_httpHeaders.begin(); it!=m_httpHeaders.end(); it++){
		if((*it).first == headerName){
			return (*it).second;
		}
	}
	
	return "";
}

int HTTPResponse::setHTTPHeaderVector(vector<pair<string, string> >* const httpHeaderVector)
{
	for(vector<pair<string, string> >::iterator it = httpHeaderVector->begin(); it!=httpHeaderVector->end(); it++){
		setHTTPHeader((*it).first, (*it).second);
	}

	return 0;
}

vector<pair<string, string> >* HTTPResponse::getHTTPHeaderVector()
{
	return &m_httpHeaders;
}

int HTTPResponse::setResponseBody(const string *responseBody)
{
	m_responseBody = *responseBody;

	return 0;
}

string* HTTPResponse::getResponseBodyPtr()
{
	return &m_responseBody;
}

int HTTPResponse::prepareResponse(void )
{
	string protocol;
	ostringstream dataStream;

	switch(m_protocol){
		case HTTP1_0:
			protocol = "HTTP/1.0";
			break;
		case HTTP1_1:
			protocol = "HTTP/1.1";
			break;
		default:
			return -1;
			break;
	}

	dataStream<<protocol<<" "<<m_statusCode<<" "<<m_reasonPhrase<<CRLF;

	for(vector<pair<string, string> >::iterator  it = m_httpHeaders.begin(); it!=m_httpHeaders.end(); it++){
		dataStream<<(*it).first<<": "<<(*it).second<<CRLF;
	}
	dataStream<<CRLF;

	dataStream<<m_responseBody;

	m_data = dataStream.str();
	return 0;
}

int HTTPResponse::parseResponse()
{
	/*
	   Response = Status-Line CRLF
	   (Response-Header CRLF)*
	   CRLF
	   [message-body]

	   Status-Line = HTTP/1.0 <space> Status-Code <space> Reason-Phrase CRLF
	 */
	
	size_t parseCursorOld = 0, parseCursorNew = 0;
	size_t headerParseCursorOld, headerParseCursorNew;
	string httpProtocol, statusCode, reasonPhrase, responseHeader;
	string responseHeaderName, responseHeaderContent;
	
	/* Parse Status-Line */
	/* HTTP Protocol */
	parseCursorNew = m_data.find_first_of(" ", parseCursorOld);
	httpProtocol = m_data.substr(parseCursorOld, parseCursorNew - parseCursorOld);
	parseCursorOld = parseCursorNew+1;
	
	if(httpProtocol == "HTTP/1.0"){
		m_protocol = HTTP1_0;
	}else if(httpProtocol == "HTTP/1.1"){
		m_protocol = HTTP1_1;
	}else{
		m_protocol = HTTP_UNSUPPORTED;
		return 0;
	}

	/* Status Code */
	parseCursorNew = m_data.find_first_of(" ", parseCursorOld);
	statusCode = m_data.substr(parseCursorOld, parseCursorNew - parseCursorOld);
	m_statusCode = atoi(statusCode.c_str());
	parseCursorOld = parseCursorNew+1;

	/* Reason Phrase */
	parseCursorNew = m_data.find_first_of(CRLF, parseCursorOld);
	m_reasonPhrase = m_data.substr(parseCursorOld, parseCursorNew - parseCursorOld);
	parseCursorOld = parseCursorNew+1;

	/* Skip the CRLF */
	parseCursorOld++;

	/* Response Headers start here */
	while(1){
		parseCursorNew = m_data.find_first_of(CRLF, parseCursorOld);
		responseHeader = m_data.substr(parseCursorOld, parseCursorNew - parseCursorOld);
		parseCursorOld = parseCursorNew+1;

		headerParseCursorOld = headerParseCursorNew = 0;
		/* Further parse the response header */
		/* Header Name */
		headerParseCursorNew = responseHeader.find_first_of(":", headerParseCursorOld);
		responseHeaderName = responseHeader.substr(headerParseCursorOld, headerParseCursorNew - headerParseCursorOld);
		headerParseCursorOld = headerParseCursorNew+2;
	
		/* Header Content */
		headerParseCursorNew = responseHeader.find_first_of(CRLF, headerParseCursorOld);
		responseHeaderContent = responseHeader.substr(headerParseCursorOld, headerParseCursorNew - headerParseCursorOld);
		headerParseCursorOld = headerParseCursorNew;

		setHTTPHeader(responseHeaderName, responseHeaderContent);
	
		/* Skip the CRLF */
		parseCursorOld++;
	
		/* Is there another CRLF? */
		if(m_data.substr(parseCursorOld, 2) == CRLF)
			break;
	}

	parseCursorOld+=2;
	m_responseBody = m_data.substr(parseCursorOld);
	
	return 0;
}

size_t HTTPResponse::getResponseSize(void )
{
	return m_data.length();
}

string* HTTPResponse::getResponseDataPtr()
{
	return &m_data;
}

int HTTPResponse::copyFromFile(ifstream& ifs, int contentLength)
{
	char* fileBuf = new char[contentLength];
	memset(fileBuf, '\0', contentLength);

	if(ifs.good()){
		ifs.read(fileBuf, contentLength);
	}
	m_responseBody.append(fileBuf, contentLength);

	if(ifs.bad())
		return -1;

	return 0;
}

int HTTPResponse::copyToFile(ofstream& ofs)
{
	int contentLength = atoi(getHTTPHeader("Content-Length").c_str());
	if(contentLength == -1)
		return -1;

	if(ofs.good()){
		if(contentLength)
			ofs.write(m_responseBody.c_str(), contentLength);
		else{
			cerr<<"WARNING: Content-Length Header not found. Written file might not be accurate."<<endl;
			ofs.write(m_responseBody.c_str(), m_responseBody.length());
		}
	}

	if(ofs.bad())
		return -1;

	return 0;
}
