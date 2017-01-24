/* HTTPRequest.cpp */

#include<iostream>
#include<string>
#include<vector>
#include<iterator>
#include<cstdlib>
#include<cstring>

#include"HTTPRequest.h"

HTTPRequest::HTTPRequest():m_requestBody(""), m_data("")
{
}

HTTPRequest::~HTTPRequest()
{
}

void HTTPRequest::printRequest(void )
{
	cout<<"---Request Begin---"<<endl<<m_data
		<<"---Request End---"<<endl;
}

void HTTPRequest::addData(const char* buf, const int& len)
{
	m_data.append(buf, len);
}

void HTTPRequest::addRequestBody(const string& str)
{
	m_requestBody += str;
}

int HTTPRequest::setMethod(Method method)
{
	m_httpMethod = method;
	return 0;
}

Method HTTPRequest::getMethod()
{
	return m_httpMethod;
}

int HTTPRequest::setURL(string url)
{
	m_url = url;
	return 0;
}

string HTTPRequest::getURL()
{
	return m_url;
}

int HTTPRequest::setProtocol(Protocol protocol)
{
	m_protocol = protocol;
	return 0;
}

Protocol HTTPRequest::getProtocol()
{
	return m_protocol;
}

int HTTPRequest::setUserAgent(string userAgent)
{
	m_userAgent = userAgent;
	return 0;
}

string HTTPRequest::getUserAgent()
{
	return m_userAgent;
}

int HTTPRequest::setHTTPHeader(string headerName, string headerContent)
{
	m_httpHeaders.push_back(make_pair(headerName, headerContent));
	return 0;
}

string HTTPRequest::getHTTPHeader(string headerName)
{
	for(vector<pair<string, string> >::iterator it = m_httpHeaders.begin(); it!=m_httpHeaders.end(); it++){
		if((*it).first == headerName){
			return (*it).second;
		}
	}
	
	return "";
}

int HTTPRequest::setHTTPHeaderVector(vector<pair<string, string> >* const httpHeaderVector)
{
	for(vector<pair<string, string> >::iterator it = httpHeaderVector->begin(); it!=httpHeaderVector->end(); it++){
		setHTTPHeader((*it).first, (*it).second);
	}

	return 0;
}

vector<pair<string, string> >* HTTPRequest::getHTTPHeaderVector()
{
	return &m_httpHeaders;
}

int HTTPRequest::setRequestBody(const string *requestBody)
{
	m_requestBody = *requestBody;

	return 0;
}

string* HTTPRequest::getRequestBodyPtr()
{
	return &m_requestBody;
}

int HTTPRequest::parseRequest()
{
	/*
	   Request = Request-Line CRLF
	   (Request-Header CRLF)*
	   CRLF
	   Message-Body
	   Request-Line = Method-Name <space> Request-URI <space> HTTP/1.0 CRLF
	   Request-Header = Header-Name ":" <space> Header-Content CRLF
	 */
	
	size_t parseCursorOld = 0, parseCursorNew = 0;
	size_t headerParseCursorOld, headerParseCursorNew;
	string httpMethod, httpProtocol, requestHeader;
	string requestHeaderName, requestHeaderContent;
	
	/* Parse Request-Line */
	/* HTTP Method */
	parseCursorNew = m_data.find_first_of(" ", parseCursorOld);
	httpMethod = m_data.substr(parseCursorOld, parseCursorNew - parseCursorOld);
	parseCursorOld = parseCursorNew+1;
	
	if(httpMethod == "GET"){
		m_httpMethod = GET;
	}else if(httpMethod == "PUT"){
		m_httpMethod = PUT;
	}else{
		m_httpMethod = NOT_IMPLEMENTED;
		return 0;
	}

	/* URL */
	parseCursorNew = m_data.find_first_of(" ", parseCursorOld);
	m_url = m_data.substr(parseCursorOld, parseCursorNew - parseCursorOld);
	parseCursorOld = parseCursorNew+1;

	/* HTTP Protocol */
	parseCursorNew = m_data.find_first_of(CRLF, parseCursorOld);
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

	/* Skip the CRLF */
	parseCursorOld++;

	/* Request Headers start here */
	while(1){
		parseCursorNew = m_data.find_first_of(CRLF, parseCursorOld);
		requestHeader = m_data.substr(parseCursorOld, parseCursorNew - parseCursorOld);
		parseCursorOld = parseCursorNew+1;

		headerParseCursorOld = headerParseCursorNew = 0;
		/* Further parse the request header */
		/* Header Name */
		headerParseCursorNew = requestHeader.find_first_of(":", headerParseCursorOld);
		requestHeaderName = requestHeader.substr(headerParseCursorOld, headerParseCursorNew - headerParseCursorOld);
		headerParseCursorOld = headerParseCursorNew+2;
	
		/* Header Content */
		headerParseCursorNew = requestHeader.find_first_of(CRLF, headerParseCursorOld);
		requestHeaderContent = requestHeader.substr(headerParseCursorOld, headerParseCursorNew - headerParseCursorOld);
		headerParseCursorOld = headerParseCursorNew;

		setHTTPHeader(requestHeaderName, requestHeaderContent);
	
		/* Skip the CRLF */
		parseCursorOld++;
	
		/* Is there another CRLF? */
		if(m_data.substr(parseCursorOld, 2) == CRLF)
			break;
	}

	parseCursorOld+=2;
	m_requestBody = m_data.substr(parseCursorOld);
	
	return 0;
}

int HTTPRequest::prepareRequest()
{
	string httpMethod, protocol;

	switch(m_httpMethod){
		case GET:
			httpMethod = "GET";
			break;
		case PUT:
			httpMethod = "PUT";
			break;
		default:
			return -1;
			break;
	}

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

	m_data += httpMethod + " " + m_url + " " + protocol + CRLF;

	for(vector<pair<string, string> >::iterator  it = m_httpHeaders.begin(); it!=m_httpHeaders.end(); it++){
		m_data += (*it).first + ": " + (*it).second + CRLF;
	}
	m_data += CRLF;

	m_data += m_requestBody;

	return 0;
}

size_t HTTPRequest::getRequestSize()
{
	return m_data.length();
}

string* HTTPRequest::getRequestDataPtr()
{
	return &m_data;
}

int HTTPRequest::copyToFile(ofstream& ofs)
{
	size_t contentLength = atoi(getHTTPHeader("Content-Length").c_str());

	if(ofs.good()){
		ofs.write(m_requestBody.c_str(), contentLength);
	}

	if(ofs.bad())
		return -1;

	return 0;
}

int HTTPRequest::copyFromFile(ifstream& ifs, size_t contentLength)
{
	char* fileBuf = new char[contentLength];
	memset(fileBuf, '\0', contentLength);

	if(ifs.good()){
		ifs.read(fileBuf, contentLength);
	}
	m_requestBody.append(fileBuf, contentLength);

	if(ifs.bad())
		return -1;

	return 0;
}
