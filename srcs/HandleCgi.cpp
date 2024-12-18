//-- Written by : msumon

#include "../includes/HandleCgi.hpp"
#include "../includes/Client.hpp"
#include "../includes/main.hpp"
#include "../includes/Request.hpp"
#include "../includes/Server.hpp"
#include "../includes/Helper.hpp"
#include "../includes/Response.hpp"
#include "../includes/LocationFinder.hpp"

#include <cstddef>
#include <cstring>
#include <fcntl.h>
#include <iostream>
#include <netdb.h>
#include <stdexcept>
#include <string>
#include <algorithm>
#include <sys/epoll.h>
#include <sys/types.h>
#include <unistd.h>

HandleCgi::HandleCgi()
{
    _pipeIn[0] = -1;
    _pipeIn[1] = -1;
    _pipeOut[0] = -1;
    _pipeOut[1] = -1;
	_locationPath = "";
    _method = "";
    _postBody = "";
    _fileName = "";
	_pid = -1;
	_childReaped = false;
	_byteTracker = 0;
	_totalBytesSent = 0;
	_mimeCheckDone = false;
	_cgiDone = false;
}

//-- Function to initialize the environment variables
//-- We can add more environment variables here
void HandleCgi::initEnv(Request &request)
{
    _env["SERVER_PROTOCOL"] = request.getMethodProtocol();
    _env["GATEWAY_INTERFACE"] = "CGI/1.1";
    _env["SERVER_SOFTWARE"] = "Webserv/1.0";
    _env["METHOD"] = request.getMethodName();
    _env["PATH_INFO"] = _locationPath;
    _env["METHOD PATH"] = request.getMethodPath();
    _env["CONTENT_LENGTH"] = xto_string(request._requestBody.size());
    _env["CONTENT_TYPE"] = request.getMethodMimeType();
}

//-- Constructor to handle the CGI request
//-- I will parse the request to get the path of the CGI file
//-- then I will call the proccessCGI function to execute the CGI script
//-- and send the output to the client
HandleCgi::HandleCgi(std::string requestBuffer, int nSocket, Client &client, Request &request)
{
    //-- Initialize the environment variables
    LocationFinder locationFinder;
    locationFinder.locationMatch(&client, requestBuffer, nSocket);
    std::string rootFolder = locationFinder._root;
    std::string location = locationFinder._locationPath;
    std::string index = locationFinder._index;
    _locationPath = locationFinder._pathToServe;

    //-- Handle POST method for CGI
    _method = request.getMethodName();
    _postBody = request._requestBody;
    _fileName = request._postFilename;
	_pid = -1;
	_pipeIn[0] = -1;
    _pipeIn[1] = -1;
    _pipeOut[0] = -1;
    _pipeOut[1] = -1;
	_childReaped = false;
	_byteTracker = 0;
	_totalBytesSent = 0;
	_mimeCheckDone = false;
	_cgiDone = false;
    if (locationFinder.isDirectory(_locationPath)) {
        throw std::runtime_error("404");
	}
    if (_method != "GET" && _method != "POST")
        throw std::runtime_error("405");
    if (locationFinder._allowedMethodFound && locationFinder._cgiFound)
    {
        if (locationFinder._allowed_methods.find(_method) == std::string::npos)
            throw std::runtime_error("405");
    }
	if (access(_locationPath.c_str(), W_OK) == -1)
    {
        std::cerr << BOLD RED << "Permission denied: " << _locationPath << RESET << std::endl;
        throw std::runtime_error("403");
    }
    client._isCgi = true;
    proccessCGI(&client);
}

//--- Main function to process CGI
void HandleCgi::proccessCGI(Client* client)
{
	if (pipe(_pipeOut) == -1 || pipe(_pipeIn) == -1)
		throw std::runtime_error("500");
	_pid = fork();
	if (_pid < 0)
		throw std::runtime_error("500");
	else if (_pid == 0)
		handleChildProcess(_locationPath, *client->_request.begin());
	else
		handleParentProcess(client);
}

//-- Function to handle the child process
void HandleCgi::handleChildProcess(const std::string &_locationPath, Request &request)
{
	//-- Check if the file has write permissions

    dup2(_pipeIn[0], STDIN_FILENO); //-- Redirect stdin to read end of the pipe
    dup2(_pipeOut[1], STDOUT_FILENO); //-- Redirect stdout to write end of the pipe

    close(_pipeIn[0]); //-- Close write end of the pipe
    close(_pipeOut[1]); //-- Close read end of the pipe


    initEnv(request);
    std::string executable = getExecutable(_locationPath);

    //--- Prepare arguments for execve
    char *const argv[] = { (char *)executable.c_str(), (char *)_locationPath.c_str(), NULL };

    //--- Prepare environment variables for execve
    std::vector<char*> envp;
    for (std::map<std::string, std::string>::const_iterator it = _env.begin(); it != _env.end(); it++)
    {
        std::string envVar = it->first + "=" + it->second + '\n';
        char* envStr = new char[envVar.size() + 1];
        std::copy(envVar.begin(), envVar.end(), envStr);
        envStr[envVar.size()] = '\0';
        envp.push_back(envStr);
    }
    envp.push_back(NULL);

    execve(executable.c_str(), argv, envp.data());
    std::cout << BOLD RED << "EXECVE FAILED" << RESET << std::endl;
    throw std::runtime_error("500");
}

//-- Function to determine the executable based on the file extension
std::string HandleCgi::getExecutable(const std::string &_locationPath)
{
    size_t pos = _locationPath.rfind(".");
    std::string extension;
    if (pos != std::string::npos)
        extension = _locationPath.substr(pos);
    else
        throw std::runtime_error("403"); //-- NOT SURE IF 403 IS THE RIGHT ERROR CODE

    std::string executable;
    executable = Helper::executableMap.find(extension)->second;
    if (executable.empty())
        throw std::runtime_error("403"); //-- NOT SURE IF 403 IS THE RIGHT ERROR CODE

    return executable;
}

//----- Function to handle the parent process
//--- NOTE: This should go through epoll
//--- read there from pipe_fd[0] and write to the client socket
void HandleCgi::handleParentProcess(Client* client)
{
	close(_pipeIn[0]); //-- Close read end of the pipe
	close(_pipeOut[1]); //-- Close write end of the pipe
	Helper::addFdToEpoll(client, _pipeIn[1], EPOLLOUT);
	Helper::setFdFlags(_pipeIn[1], O_NONBLOCK);
	Helper::setFdFlags(_pipeOut[0], O_NONBLOCK);
	client->_io.setFd(_pipeIn[1]);
	client->_request.begin()->_isWrite = true;
}

void	HandleCgi::checkWaitPid(Client* client)
{
	if (_childReaped || client->_io.getTimeout() || checkCgiTimeout(client))
		return;
	int status = 0;
	pid_t result = waitpid(_pid, &status, WNOHANG);	
	if (result == -1)
		throw std::runtime_error("500");
	else if (result > 0)
	{
		// Child process has terminated
		if (WIFEXITED(status))
		{
			if (WEXITSTATUS(status) > 0) {
				throw std::runtime_error("500");
			}
	        std::cout << BOLD GREEN << "CGI script executed successfully." << RESET << std::endl;
		}
		else if (WIFSIGNALED(status))
			throw std::runtime_error("500");
		_childReaped = true;
	}
}

bool	HandleCgi::checkCgiTimeout(Client *client)
{
	if (Helper::getElapsedTime(client) < CGI_TIMEOUT)
		return (false);
	client->_io.setTimeout(true);
	kill (_pid, SIGKILL);
	_cgiDone = true;
	throw std::runtime_error("504");
}

void	HandleCgi::closeCgi(Client* client)
{
	if (client->_isCgi)
	{
		client->_epoll->removeCgiClientFromEpoll(_pipeIn[1]);
		client->_epoll->removeCgiClientFromEpoll(_pipeOut[0]);
		_pipeIn[1] = -1;
		_pipeOut[0] = -1;
	}
}

void	HandleCgi::setCgiDone(bool value)
{
	_cgiDone = value;
}

bool    HandleCgi::getCgiDone() const
{
    return (_cgiDone);
}

bool	HandleCgi::getChildReaped() const
{
	return (_childReaped);
}

int		HandleCgi::getPipeIn(unsigned i) const
{
	if (i > 1)
		throw std::runtime_error("500");
	return (_pipeIn[i]);
}

void	HandleCgi::setPipeIn(unsigned i, int val)
{
	if (i > 1)
		throw std::runtime_error("500");
	_pipeIn[i] = val;
}

int		HandleCgi::getPipeOut(unsigned i) const
{
	if (i > 1)
		throw std::runtime_error("500");
	return (_pipeOut[i]);
}

void	HandleCgi::setPipeOut(unsigned i, int val)
{
	if (i > 1)
		throw std::runtime_error("500");
	_pipeIn[i] = val;
}

pid_t	HandleCgi::getPid() const
{
	return (_pid);
}

std::string	HandleCgi::getLocationPath() const
{
	return (_locationPath);
}

HandleCgi::~HandleCgi()
{}

//--- Copy constructor
HandleCgi::HandleCgi(const HandleCgi &src)
	:	_locationPath(src._locationPath),
		_method(src._method),
		_postBody(src._postBody),
		_fileName(src._fileName),
		_pid(src._pid),
		_childReaped(src._childReaped),
		_byteTracker(src._byteTracker),
		_totalBytesSent(src._totalBytesSent),
		_response(src._response),
		_responseStr(src._responseStr),
		_mimeCheckDone(src._mimeCheckDone),
		_cgiDone(src._cgiDone),
		_env(src._env)
{
	// Deep copy the pipe file descriptors
	_pipeIn[0] = src._pipeIn[0];
	_pipeIn[1] = src._pipeIn[1];
	_pipeOut[0] = src._pipeOut[0];
	_pipeOut[1] = src._pipeOut[1];
}

//--- Assignment operator
HandleCgi &HandleCgi::operator=(const HandleCgi &src)
{
    if (this != &src)
	{
		_locationPath = src._locationPath;
		_method = src._method;
		_postBody = src._postBody;
		_fileName = src._fileName;
		_pipeIn[0] = src._pipeIn[0];
		_pipeIn[1] = src._pipeIn[1];
		_pipeOut[0] = src._pipeOut[0];
		_pipeOut[1] = src._pipeOut[1];
		_pid = src._pid;
		_childReaped = src._childReaped;
		_byteTracker = src._byteTracker;
		_totalBytesSent = src._totalBytesSent;
		_response = src._response;
		_responseStr = src._responseStr;
		_mimeCheckDone = src._mimeCheckDone;
		_cgiDone = src._cgiDone;
		_env = src._env;
	}
	return (*this);
}

//--- == operator overloading
bool HandleCgi::operator==(const HandleCgi &src) const
{
    return (_locationPath == src._locationPath &&
           _method == src._method &&
           _postBody == src._postBody &&
           _fileName == src._fileName &&
           _pipeIn[0] == src._pipeIn[0] &&
           _pipeIn[1] == src._pipeIn[1] &&
           _pipeOut[0] == src._pipeOut[0] &&
           _pipeOut[1] == src._pipeOut[1] &&
		   _pid == src._pid &&
		   _childReaped == src._childReaped &&
           _byteTracker == src._byteTracker &&
           _totalBytesSent == src._totalBytesSent &&
           _response == src._response &&
           _responseStr == src._responseStr &&
           _mimeCheckDone == src._mimeCheckDone &&
           _cgiDone == src._cgiDone &&
           _env == src._env);
}
