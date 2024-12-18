#pragma once

#include <cstddef>
#include <map>
#include <string>
#include <vector>
#include <sys/types.h>
#include <sys/socket.h>

#include "Method.hpp"
#include "ServerConfig.hpp"

class Server;
class Response;
class Client;

class Request {
	private:
		bool	_firstLineChecked;
		bool	_headerChecked;
		bool	_readingFinished;
		bool	_isChunked;
		Method*	_method;
		std::map<std::string, std::string> _headerMap;
		std::size_t	_contentRead;
		ssize_t 	_totalBytesRead;
		std::string _fileName;
		std::string	_methodAndPath;


		std::string	_host;
		void	storeOneHeaderInMap(const std::string& oneLine);
		void	storeHeadersInMap(const std::string& oneLine, std::size_t& endPos);

	public:
		std::size_t		_contentLength;
		std::string		_requestBody;
		std::string		_postFilename;
		ServerConfig*	_servConf;
		Response*		_response;
		bool			_isRead;
		bool			_isWrite;

		Request();
		Request(const Request& other);
		Request&	operator=(const Request& other);
		bool		operator==(const Request& other) const;
		~Request();

		bool		hasMethod() const;
		std::string getMethodName() const;
		std::string getMethodPath() const;
		std::string getMethodProtocol() const;
		std::string getMethodMimeType() const;
		bool		getFirstLineChecked() const;
		bool		getReadingFinished() const;
		std::map<std::string, std::string> getHeaderMap() const;
		std::string getHeaderFromHeaderMap(std::string headerName) const;
		std::string getHost() const;
		std::string getSessionId() const; //-- BONUS : cookies
		std::string	getUri(); //-- Extract Referer

		void	setMethodMimeType(std::string path);
		void	storeRequestBody(const std::string& strLine, std::size_t endPos);
		void	extractHttpMethod(std::string&);
		void	createHttpMethod(const std::string&);
		void	checkFirstLine(std::string& strLine, std::size_t& endPos);

		void	checkLine(std::vector<char>& line);
		void	checkHost(Client*);

		void	executeMethod(int socketFd, Client *client);

		int		clientRequest(Client* client);
		void	validRequest(Server* serv, std::vector<char> buffer, ssize_t count, Request& request);
		void	validRequest(std::vector<char>, ssize_t, Request&);
		int		invalidRequest(Client*);
		int		emptyRequest(Client*);

		void	requestReset();

		void 	storeRequestBody(std::string& strLine, std::size_t endPos);
};
