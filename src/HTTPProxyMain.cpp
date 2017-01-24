/* HTTPProxyMain.cpp */

#include<iostream>
#include<string>

#include<stdlib.h>

#include"HTTPProxy.h"

using namespace std;

int main(int argc, char* argv[])
{
	int port;
	HTTPProxy* httpProxy;

	if(argc == 2){
		port = atoi(argv[1]);
		httpProxy = new HTTPProxy(port);
	}else{
		httpProxy = new HTTPProxy();
	}

	if(httpProxy->run()){
		cerr<<"Error starting HTTPProxy"<<endl;
	}
	return 0;
}
