//-- Written by : msumon

#include "../includes/ServerConfig.hpp"
#include "../includes/LocationConfig.hpp"

#include <cstddef>
#include <string>
#include <vector>
#include <set>
#include <iostream>
#include <map>
#include <algorithm>
#include <sstream>
#include <map>

ServerConfig::ServerConfig() : LocationConfig()
{
}

bool    ServerConfig::operator==(const ServerConfig& other) const
{
    return listenPort == other.listenPort &&
           serverName == other.serverName &&
           errorPage == other.errorPage &&
           cgiFile == other.cgiFile &&
           tryFiles == other.tryFiles &&
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
            if (key != "allowed_methods" && key != "try_files")
                value.erase(std::remove(value.begin(), value.end(), ' '), value.end());
            if (key.empty() || value.empty())
                throw std::runtime_error(BOLD RED "ERROR : " + line + " [ NOT VALID ]" RESET);
            locationConfig.insertInMap(key, value);
        }
        i++;
    }
    server.locations.push_back(locationConfig);
}

void ServerConfig::handleErrorPages(std::string line, ServerConfig &server)
{
    std::vector<std::string> errorCodes;
    std::string errorPagePath;
    std::string temp;

    size_t pos = line.find(" ");
    if (pos != std::string::npos)
        temp = line.substr(pos + 1);

    std::stringstream ss(temp);
    std::string word;
    while (ss >> word)
        errorCodes.push_back(word);

    errorPagePath = errorCodes.at(errorCodes.size() - 1);
    for (size_t i = 0; i < errorCodes.size() -1; i++)
    {
        server.errorPages.insert(std::pair<std::string, std::string>(errorCodes[i], errorPagePath));
    }

    //std::cout << BOLD YELLOW << errorPagePath << RESET << std::endl;

    // if (server.errorPage.empty())
    //     throw std::runtime_error(BOLD RED "ERROR : " + line + " [ NOT VALID ]" RESET);
}

void ServerConfig::serverBlock(std::string line, size_t &i, std::vector<std::string> configVector, ServerConfig &server, std::string configFile)
{
    while (i < configVector.size())
    {
        line = configVector[i];
        size_t pos = line.find_last_not_of(" ");
        line = line.substr(0, pos + 1);
        //std::cout << "line: " << line << std::endl;
        if (line.find('}') != std::string::npos)
            break;
        // Process server directives
        //-- Nginx does not run if listen and server_name directives are not present
        if (line.find("listen") == 0)
        {
            size_t pos = line.find(" ");
            if (pos != std::string::npos)
            {
                server.listenPort = line.substr(pos + 1);
                if (server.listenPort.empty())
                    throw std::runtime_error(BOLD RED "ERROR : " + line + " [ NOT VALID ]" RESET);
                server.makePortVector();
            }
        }
        else if (line.find("server_name") == 0)
        {
            size_t pos = line.find(" ");
            if (pos != std::string::npos)
            {
                server.serverName = line.substr(pos + 1);
                if (server.serverName.empty())
                    throw std::runtime_error(BOLD RED "ERROR : " + line + " [ NOT VALID ]" RESET);
                server.makeServerNameVector();
            }
        }
        else if (line.find("error_page") == 0)
        {
            handleErrorPages(line, server);
        }
        else if (line.find("cgi-bin") == 0)
        {
            size_t pos = line.find(" ");
            if (pos != std::string::npos)
                server.cgiFile = line.substr(pos + 1);
            server.cgiFile.erase(std::remove(server.cgiFile.begin(), server.cgiFile.end(), ' '), server.cgiFile.end());
            if (server.cgiFile.empty())
                throw std::runtime_error(BOLD RED "ERROR : " + line + " [ NOT VALID ]" RESET);
        }
        else if (line.find("try_files") == 0)
        {
            size_t pos = line.find(" ");
            if (pos != std::string::npos)
                server.tryFiles = line.substr(pos + 1);
            if (server.tryFiles.empty())
                throw std::runtime_error(BOLD RED "ERROR : " + line + " [ NOT VALID ]" RESET);
        }
        else if (line.find("location") == 0)
            locationBlock(line, i, configVector, server, configFile);
        else if (line.find("client_max_body_size") == 0)
        {
            size_t pos = line.find(" ");
            if (pos != std::string::npos)
                server.clientMaxBodySize = line.substr(pos + 1);
            server.clientMaxBodySize.erase(std::remove(server.clientMaxBodySize.begin(), server.clientMaxBodySize.end(), ' '), server.clientMaxBodySize.end());
            if (server.clientMaxBodySize.empty())
                throw std::runtime_error(BOLD RED "ERROR : " + line + " [ NOT VALID ]" RESET);
        }
        else if (line == "{" || line.empty())
        {
            i++;
            continue;
        }
        else  
            throw std::runtime_error(BOLD RED "ERROR : " + line + " [ NOT VALID ]" RESET);
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
            throw std::runtime_error(BOLD + configFile + RED + " [ KO ] " + RESET);
        }
        i++;
    }
    if (checkLocations() == false)
        throw std::runtime_error(BOLD + configFile + RED + " [ KO ] " + RESET);
}

//-- Double checking locationBlock
//--- Check if the location block has valid directives
//--- can not have duplicate directives
bool ServerConfig::checkLocations()
{
    for (size_t i = 0; i < servers.size(); i++)
    {
        ServerConfig server = servers[i];

        std::set<std::string> locationPaths;
        for (size_t j = 0; j < server.locations.size(); j++)
        {
            LocationConfig location = server.locations[j];
            std::string locationPath = location.getPath();
            locationPath.erase(std::remove(locationPath.begin(), locationPath.end(), ' '), locationPath.end());
            locationPath.erase(std::remove(locationPath.begin(), locationPath.end(), '{'), locationPath.end());
            if (locationPaths.find(locationPath) != locationPaths.end())
            {
                std::cerr << BOLD RED << "LINE : " << locationPath << "  [ DUPLICATE ]" << RESET << std::endl;
                return false;
            }

            locationPaths.insert(locationPath);
            
            //-- Check inside location block
            //--- Check if the location block has valid directives
            //--- can not have duplicate directives
            std::multimap<std::string, std::string > locationMap = location.getLocationMap();
            std::multimap<std::string, std::string >::iterator it;
            
            std::set<std::string> locationSet;
            //locationPaths.clear();
            for ( it = locationMap.begin(); it != locationMap.end(); ++it)
            {
                locationPath = it->first;
                if (locationPath == "root" || locationPath == "index" ||
                    locationPath == "autoindex" || locationPath == "cgi-bin" ||
                    locationPath == "allowed_methods" || locationPath == "try_files" ||
                    locationPath == "return" || locationPath == "client_max_body_size")
                {
                    if (locationSet.find(locationPath) != locationSet.end())
                    {
                        std::cerr << BOLD RED << "LINE : " << locationPath << "  [ DUPLICATE ]" << RESET << std::endl;
                        return false;
                    }
                    locationSet.insert(locationPath);
                }
                else
                {
                    std::cerr << BOLD RED << "LINE : " << locationPath << "  [ NOT VALID ]" << RESET << std::endl;
                    return false;
                }
            }
        }
    }
    return true;
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

std::string ServerConfig::getTryFiles()
{
    return (tryFiles);
}

std::map<std::string, std::string> ServerConfig::getErrorPages()
{
    return (errorPages);
}

ServerConfig::~ServerConfig()
{
}
