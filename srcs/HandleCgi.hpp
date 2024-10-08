//-- Written by : msumon

#pragma once

#include <iostream>
#include <string>
#include <vector>
#include <unistd.h>
#include <stdexcept>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <sstream>
#include "ServerConfig.hpp"
#include "Server.hpp"

class HandleCgi : public ServerConfig
{
    private:
        std::string cgiPath;
        std::string cgiConf;

    public:
        HandleCgi();
        HandleCgi(std::string requestBuffer, int nSocket, Server&);
        ~HandleCgi();

        void proccessCGI(int nSocket);
        void getCgiConfPath(std::string configFile);

        // Function template to convert various types to string
        template <typename T>
        std::string to_string(const T& value)
        {
            std::stringstream ss;
            ss << value;
            return ss.str();
        }
};