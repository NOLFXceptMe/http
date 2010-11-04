/* HTTPServerMain.cpp */

#include<iostream>
#include<string>

#include<stdlib.h>

#include"HTTPServer.h"

using namespace std;

int main(int argc, char* argv[])
{
	int port;
	HTTPServer* httpServer;

	if(argc == 2){
		port = atoi(argv[1]);
		httpServer = new HTTPServer(port);
	}else{
		httpServer = new HTTPServer();
	}

	if(httpServer->run()){
		cerr<<"Error starting HTTPServer"<<endl;
	}

	free(httpServer);

	return 0;
}
