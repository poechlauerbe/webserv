
#include "../includes/PostMethod.hpp"
#include "../includes/Request.hpp"
#include "../includes/Response.hpp"
#include "../includes/Client.hpp"
#include "../includes/LocationFinder.hpp"
#include "../includes/HandleCgi.hpp"

#include <string>
#include <fstream>
#include <algorithm>

PostMethod::PostMethod() : socketFd(-1)
{
    saveDir = "";
    fileName = "";
    fileBody = "";
    root = "";
    locationPath = "";
}

PostMethod::PostMethod(const PostMethod& other) : Method(other), socketFd(other.socketFd) {}

PostMethod& PostMethod::operator=(const PostMethod& other)
{
    if (this == &other)
        return (*this);
    Method::operator=(other);
    return (*this);
}

PostMethod::~PostMethod() {}

//-- Execute the POST method
void PostMethod::executeMethod(int socketFd, Client *client, Request &request)
{
    std::string body = request._requestBody;
    std::string requestPath = request.getMethodPath();

    if (requestPath.find("/cgi-bin") != std::string::npos)
    {
        try
        {
            HandleCgi cgi(requestPath, socketFd, *client, request);
            std::cout << BOLD GREEN << "CGI script executed successfully." << RESET << std::endl;
        }
        catch (std::exception &e)
        {
            request._response->error(socketFd, request, e.what(), client);
        }
        return;
    }

    bool isLocation = false;
    LocationFinder locationFinder;
    isLocation = locationFinder.locationMatch(client, request.getMethodPath(), socketFd);
    //-- SUMON I think this is not needed
    // if (!isLocation)
    // {
    //     request._response->error(socketFd, request, "404", client);
    //     return;
    // }

    //-- Check if the allowed methods include POST
    if (isLocation && locationFinder._allowedMethodFound)
    {
        if (locationFinder._allowed_methods.find("POST") == std::string::npos)
        {
            request._response->error(socketFd, request, "405", client);
            return;
        }
    }

    this->socketFd = socketFd;
    this->root = locationFinder._root;
    this->locationPath = locationFinder._locationPath;
    this->saveDir = root + locationPath + "/";
    this->fileBody = body;
    fileName = request._postFilename;

    handlePostRequest(request, client);
}

void PostMethod::handlePostRequest(Request &request, Client *client)
{
    std::string fileToCreate = saveDir + fileName;
    std::ofstream file;

    file.open(fileToCreate.c_str(), std::ios::binary);
    if (file.is_open())
    {
        file.write(fileBody.c_str(), fileBody.size());
        file.close();
    }
    else
    {
        std::cerr << BOLD RED << "Error: Could not open file for writing" << RESET << std::endl;
        request._response->error(socketFd, request, "500", client);
        return;
    }

    std::string body = "<html><body><h1>File uploaded successfully!</h1></body></html>";
    request._response->headerAndBody(client, socketFd, request, body);

    std::cout << BOLD YELLOW << "size : " << fileBody.size() << " bytes" << RESET << std::endl;
    std::cout << BOLD BLUE "File : " << fileName << RESET << std::endl;
    std::cout << BOLD GREEN "FILE SAVED! 💾" << RESET << std::endl;
}

Method*	PostMethod::clone()
{
	return new PostMethod(*this);
}

void PostMethod::setLocationPath(std::string locationPath)
{
    this->locationPath = locationPath;
}

void PostMethod::setRoot(std::string root)
{
    this->root = root;
}
