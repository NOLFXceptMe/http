cmake_minimum_required(VERSION 3.5)
project(http)

set(CMAKE_CXX_STANDARD 11)

set(DNS_SRC
        DNSServer.c)

set(HTTP_LIB_SRC
        HTTP.h
        HTTPRequest.h
        HTTPRequest.cpp
        HTTPResponse.h
        HTTPResponse.cpp)

set(HTTP_CLIENT_SRC
        HTTPClient.h
        HTTPClient.cpp
        HTTPClientMain.cpp)

set(HTTP_SERVER_SRC
        HTTPServer.h
        HTTPServer.cpp
        HTTPServerMain.cpp)

set(HTTP_PROXY_SRC
        HTTPProxy.cpp
        HTTPProxy.h
        HTTPProxyMain.cpp)

add_executable(DNSServer ${DNS_SRC})
add_executable(HTTPClientMain ${HTTP_LIB_SRC} ${HTTP_CLIENT_SRC})
add_executable(HTTPServerMain ${HTTP_LIB_SRC} ${HTTP_SERVER_SRC})
add_executable(HTTPProxyMain ${HTTP_LIB_SRC} ${HTTP_PROXY_SRC})
