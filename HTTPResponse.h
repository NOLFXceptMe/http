/* HTTPResponse.h */

#ifndef _HTTP_RESPONSE_H_
#define _HTTP_RESPONSE_H_

#include<string>
#include<vector>
#include<fstream>

#include"HTTP.h"

using namespace std;

class HTTPResponse{
	public:
		HTTPResponse();
		~HTTPResponse();

		void printResponse(void );
		void addData(const char *, const int&);
	
		int setProtocol(Protocol );
		Protocol getProtocol(void );

		int setStatusCode(size_t );
		size_t getStatusCode(void );

		int setReasonPhrase(void );
		string getReasonPhrase(void );
		
		int setHTTPHeader(string headerName, string headerContent);
		string getHTTPHeader(string headerName);

		int setHTTPHeaderVector(vector<pair<string, string> >* const );
		vector<pair<string, string> >* getHTTPHeaderVector(void );

		int setResponseBody(const string* );
		string* getResponseBodyPtr(void );

		int prepareResponse(void );
		int parseResponse(void );
	
		size_t getResponseSize(void );
		string* getResponseDataPtr(void );

		int copyToFile(ofstream& );
		int copyFromFile(ifstream&, int);

	private:
		Protocol m_protocol;
		size_t m_statusCode;
		string m_reasonPhrase;

		vector<pair<string, string> > m_httpHeaders;
		string m_responseBody;

		string m_data;
};

#endif	/* _HTTP_RESPONSE_H_ */
