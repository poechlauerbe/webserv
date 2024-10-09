#pragma once

#include <ctime>
#include "Request.hpp"

class Server;

class Client
{
private:
	int				_fd;
	time_t			_lastActive;
	unsigned		_numRequests;
public:
	Server*			_server;
	Request			_request; // BP: can there be one or multiple requests at the same time
	// Coplien's form
	Client();
	Client(int, Server*);
	~Client();
	Client(const Client&);
	Client&	operator=(const Client&);
	bool	operator==(const Client& other) const;

	// setters
	void		setFD(int);
	void		setLastActive(time_t);
	void		numRequestAdd1();

	// getters
	int			getFd() const;
	time_t		getLastActive() const;
	unsigned	getNumRequest() const;
};
