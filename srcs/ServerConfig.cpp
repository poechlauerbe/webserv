//-- Written by : msumon

#include "ServerConfig.hpp"
#include <cstddef>
#include <vector>

ServerConfig::ServerConfig() : LocationConfig()
{
}

bool    ServerConfig::operator==(const ServerConfig& other) const
{
    return listenPort == other.listenPort &&
           serverName == other.serverName &&
           errorPage == other.errorPage &&
           cgiFile == other.cgiFile &&
           locations == other.locations &&
           servers == other.servers;
}

void ServerConfig::locationBlock(std::string line, size_t &i, std::vector<std::string> configVector, ServerConfig &server, std::string configFile)
{
    LocationConfig locationConfig(configFile);
    size_t pos = line.find(" ");
    if (pos != std::string::npos)
        locationConfig.setPath(line.substr(pos + 1));
    i++;
    while (i < configVector.size())
    {
        line = configVector[i];
        if (line.find('}') != std::string::npos)
            break;
        pos = line.find(' ');
        if (pos != std::string::npos)
        {
            std::string key = line.substr(0, pos);
            std::string value = line.substr(pos + 1);
            locationConfig.insertInMap(key, value);
        }
        i++;
    }
    server.locations.push_back(locationConfig);
}

void ServerConfig::serverBlock(std::string line, size_t &i, std::vector<std::string> configVector, ServerConfig &server, std::string configFile)
{
    while (i < configVector.size())
    {
        line = configVector[i];
        //std::cout << "line: " << line << std::endl;
        if (line.find('}') != std::string::npos)
            break;
        // Process server directives
        if (line.find("listen") == 0)
        {
            size_t pos = line.find(" ");
            if (pos != std::string::npos)
            {
                server.listenPort = line.substr(pos + 1);
                server.makePortVector();
            }
        }
        else if (line.find("server_name") == 0)
        {
            size_t pos = line.find(" ");
            if (pos != std::string::npos)
            {
                server.serverName = line.substr(pos + 1);
                server.makeServerNameVector();
            }
        }
        else if (line.find("error_page") == 0)
        {
            size_t pos = line.find(" ");
            if (pos != std::string::npos)
                server.errorPage = line.substr(pos + 1);
        }
        else if (line.find("cgi-bin") == 0)
        {
            size_t pos = line.find(" ");
            if (pos != std::string::npos)
                server.cgiFile = line.substr(pos + 1);
        }
        else if (line.find("location") == 0)
            locationBlock(line, i, configVector, server, configFile);
        else if (line.find("client_max_body_size") == 0)
        {
            size_t pos = line.find(" ");
            if (pos != std::string::npos)
                server.clientMaxBodySize = line.substr(pos + 1);
        }
        i++;
    }
}

//-- Constructor with parameter
ServerConfig::ServerConfig(std::string configFile) : LocationConfig(configFile)
{
    this->_configFile = configFile;
    std::vector<std::string> configVector = Config::getConfig();
    size_t i = 0;

    while (i < configVector.size())
    {
        std::string line = configVector[i];
        if (line.find("server") != std::string::npos)
        {
            ServerConfig server;
            i++;
            serverBlock(line, i, configVector, server, configFile);
            servers.push_back(server);
        }
        else  
        {
            std::cerr << RED << "Line: " << line << "  [ NOT VALID ]" << RESET << std::endl;
            throw std::runtime_error("Invalid server config !!");
        }
        i++;
    }
}

//--- > To Print the config after parsing
void ServerConfig::displayConfig()
{
    for (size_t i = 0; i < servers.size(); i++)
    {
        std::cout << "-----------------------------------" << std::endl;
        //--- Display server directives
        ServerConfig server = servers[i];
        std::cout << "Server: " << i + 1 << std::endl;
        std::cout << "Listen Port: " << server.listenPort << std::endl;
        std::cout << "Server Name: " << server.serverName << std::endl;
        std::cout << "Error Page: " << server.errorPage << std::endl;
        std::cout << "CGI File: " << server.cgiFile << std::endl;

        //--- Display multiple ports
        for (size_t k = 0; k < server.getListenPorts().size(); k++)
            std::cout << "Port Vector : " << server.getListenPorts()[k] << std::endl;

        //--- Display multiple server names
        for (size_t l = 0; l < server.getServerNames().size(); l++)
            std::cout << "ServerName Vector : " << server.getServerNames()[l] << std::endl;

        //-- Display locations
        std::cout << "Locations: " << std::endl;
        for (size_t j = 0; j < server.locations.size(); j++)
        {
            LocationConfig location = server.locations[j];
            std::multimap<std::string, std::string > locationMap = location.getLocationMap();
            std::cout << "Path: " << location.getPath() << std::endl;
            std::multimap<std::string, std::string >::iterator it;
            for ( it = locationMap.begin(); it != locationMap.end(); ++it)
            {
                std::cout << it->first << " : " << it->second << std::endl;
            }
        }
        std::cout << std::endl;
    }
}

//--- > Make a vector of ports
void ServerConfig::makePortVector()
{
    std::vector<int> tempPorts;
    std::string ports = getListenPort();

    while (!ports.empty())
    {
        size_t pos = ports.find(" ");
        std::string temp = ports.substr(0, pos);
        std::stringstream ss(temp);
        int port;
        ss >> port;
        if (port < 0 || port > 65535)
            throw std::runtime_error("Invalid port number !!");
        tempPorts.push_back(port);
        if (pos == std::string::npos)
            break;
        ports.erase(0, pos + 1);
    }
    listenPorts = tempPorts;
}

//--- > Make a vector of server names
void ServerConfig::makeServerNameVector()
{
    std::vector<std::string> tempServerNames;
    std::string serverNamesStr = getServerName();

    while (!serverNamesStr.empty())
    {
        size_t pos = serverNamesStr.find(" ");
        std::string temp = serverNamesStr.substr(0, pos);
        tempServerNames.push_back(temp);
        if (pos == std::string::npos)
            break;
        serverNamesStr.erase(0, pos + 1);
    }
    serverNames = tempServerNames;
}

//--- > Get functions
std::string ServerConfig::getListenPort()
{
    return (listenPort);
}

std::string ServerConfig::getServerName()
{
    return (serverName);
}

std::string ServerConfig::getErrorPage()
{
    return (errorPage);
}

std::string ServerConfig::getCgiFile()
{
    return (cgiFile);
}

std::vector<LocationConfig> ServerConfig::getLocations()
{
    return (locations);
}

std::vector<ServerConfig> ServerConfig::getServers()
{
    return (servers);
}

std::vector<int> ServerConfig::getListenPorts()
{
    return (listenPorts);
}

std::string ServerConfig::getClientMaxBodySize()
{
    return (clientMaxBodySize);
}

std::vector<std::string> ServerConfig::getServerNames()
{
    return (serverNames);
}

ServerConfig::~ServerConfig()
{
}
