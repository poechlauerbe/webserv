#include "GetMethod.hpp"
#include "Method.hpp"
// #include "main.hpp"
#include "Response.hpp"
#include "Request.hpp"
// #include <iostream>
#include <fstream>
#include <sstream>

GetMethod::GetMethod() : Method() {}

GetMethod::GetMethod(const GetMethod& other) : Method(other) {}

GetMethod&	GetMethod::operator=(const GetMethod& other) {
	if (this == &other)
		return *this;
	Method::operator=(other);
	return *this;
}

GetMethod::~GetMethod() {}

void	GetMethod::executeMethod(int socketFd, Request& request) {

	std::string body;
	std::string path = "./conf/webpageBP"; //BP: should be from config file

	if (request.getMethodPath() == "/")
		path += "/index.html";
	else
		path += request.getMethodPath();

	this->setMimeType(path);
	std::cout << path << std::endl;
	std::ifstream file(path.c_str());
	if (!file.is_open()) {
		Response::FallbackError(socketFd, request, "404");
		return;
	}
	std::ostringstream buffer;
	buffer << file.rdbuf();
	body = buffer.str();
	file.close();
	Response::headerAndBody(socketFd, request, body);
	// 	Response::header(socketFd, request, body);

}

Method*	GetMethod::clone() {
	return new GetMethod(*this);
}