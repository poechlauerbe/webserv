#pragma once

#include <string>

#include "Header.hpp"
#include "Socket.hpp"

class Response {
	private:
		Response();
		Response(const Response& other);
		Response& operator=(const Response& other);
		~Response();

	public:
		static std::string getActualTimeString();

		static void header(int socketFd, Header& header, std::string& body);
		static void	headerAndBody(int socketFd, Header& header, std::string& body);
};