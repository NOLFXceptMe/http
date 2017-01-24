/* HTTPRequest.h */

#ifndef _HTTP_REQUEST_H_
#define _HTTP_REQUEST_H_

#include<string>
#include<vector>
#include<fstream>

#include"HTTP.h"

using namespace std;

class HTTPRequest{
	public:
		HTTPRequest();
		~HTTPRequest();
	
		void printRequest(void );
		void addData(const char *, const int&);
		void addRequestBody(const string& );
		
		int setMethod(Method );
		Method getMethod(void );

		int setURL(string );
		string getURL(void );

		int setProtocol(Protocol );
		Protocol getProtocol(void );

		int setUserAgent(string userAgent);
		string getUserAgent(void );

		int setHTTPHeader(string headerName, string headerContent);
		string getHTTPHeader(string headerName);

		int setHTTPHeaderVector(vector<pair<string, string> >* const );
		vector<pair<string, string> >* getHTTPHeaderVector(void );

		int setRequestBody(const string* );
		string* getRequestBodyPtr(void );

		int parseRequest(void );
		int prepareRequest(void );
		size_t getRequestSize(void );
		string* getRequestDataPtr(void );

		int copyToFile(ofstream& );
		int copyFromFile(ifstream&, size_t);

	private:
		Method m_httpMethod;
		string m_url;
		Protocol m_protocol;
		string m_hostName;
		string m_userAgent;

		vector<pair<string, string> > m_httpHeaders;
		string m_requestBody;
		
		string m_data;
};

#endif	/* _HTTP_REQUEST_H_ */
