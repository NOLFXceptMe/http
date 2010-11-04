/* HTTPClientMain.cpp */

#include<iostream>
#include<string>

#include<stdlib.h>

#include"HTTPClient.h"

using namespace std;

int main(int argc, char *argv[])
{
	string method, url, fileName, proxy; 

	if(argc==5){
		method = argv[1];
		url = argv[2];
		fileName = argv[3];
		proxy = argv[4];
	}else{
		cerr<<"Invalid number of parameters for starting HTTPClient"<<endl
			<<"Usage: ./HTTPClientMain <method> <url> <localFileName> <direct/proxyaddress>"<<endl;
		exit(-1);
	}

	HTTPClient* httpClient = new HTTPClient(method, url, fileName, proxy);
	if(httpClient->run()){
		cerr<<"Error in starting HTTPClient"<<endl;
		exit(-1);
	}
	
	free(httpClient);

	return 0;
}
