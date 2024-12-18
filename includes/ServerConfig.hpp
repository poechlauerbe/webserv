//-- Written by : msumon

#pragma once

#include <ctime>
#include <map>
#include <string>
#include <vector>

#include "LocationConfig.hpp"

#define BOLD "\033[1m"
#define RED "\033[31m"
#define GREEN "\033[32m"
#define YELLOW "\033[33m"
#define BLUE "\033[34m"
#define RESET "\033[0m"

// --> If you want to access the config values, initialize the ServerConfig class
// --> The ServerConfig class will have the server directives
// --> use the get functions to access the values

class ServerConfig : public LocationConfig
{
    private:
        std::string listenPort;
        std::string serverName;
        std::string errorPage;
        std::string cgiFile;
        std::string tryFiles;
        std::vector<LocationConfig> locations;
        std::vector<ServerConfig> servers;
        std::vector<int> listenPorts;
        std::vector<std::string> serverNames;
        std::string _configFile;
        std::string clientMaxBodySize;
        time_t      timeout;
        std::map<std::string, std::string> errorPages;

    public:
        ServerConfig();
        ServerConfig(std::string configFile);
        ~ServerConfig();
        bool operator==(const ServerConfig& other) const;

        void displayConfig();
        std::string getListenPort();
        std::string getServerName();
        std::string getErrorPage();
        std::string getCgiFile();
        std::string getTryFiles();
        std::string getClientMaxBodySize();
        time_t      getTimeout();
        std::vector<LocationConfig> getLocations();
        std::vector<ServerConfig> getServers();
        std::map<std::string, std::string> getErrorPages();

        void makePortVector();
        void makeServerNameVector();
        std::vector<int> getListenPorts();
        std::vector<std::string> getServerNames();
        bool checkLocations();
        void checkAllowedMethods(std::string value, std::string line);
        void checkRedirect(std::string value);
        bool checkAutoIndex(std::string line);
        bool checkClientMaxBodySize(std::string value);
        bool timeoutCheck(std::string line, ServerConfig &server);

        void serverBlock(std::string line, size_t &i, std::vector<std::string> configVector, ServerConfig &server, std::string configFile);
        void handleErrorPages(std::string line, ServerConfig &server);
        void locationBlock(std::string line, size_t &i, std::vector<std::string> configVector, ServerConfig &server, std::string configFile);
};
