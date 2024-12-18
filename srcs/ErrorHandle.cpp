//-- Written by : msumon

#include "../includes/ErrorHandle.hpp"
#include "../includes/Helper.hpp"
#include "../includes/ServerConfig.hpp"

#include <cstddef>
#include <iostream>
#include <fstream>
#include <stdexcept>

ErrorHandle::ErrorHandle()
{
    errorFile = "";
    errorStatusCode = "";
    errorMessage = "";
    pageTitle = "Error Page";
}

std::string ErrorHandle::modifyErrorPage()
{
    size_t pos = errorFile.rfind("/");
    std::string fileDirectory = errorFile.substr(0, pos + 1);

    //-- open the default error file
    std::ifstream defaultErrorFile(errorFile.c_str());

    //-- create a new error file, error code will be the file name
    std::string tempFileName = fileDirectory + errorStatusCode + ".html";
    newErrorFile = tempFileName;
    std::ofstream newErrorFile(tempFileName.c_str());

    if (defaultErrorFile.is_open() && newErrorFile.is_open())
    {
        std::string line;
        while (std::getline(defaultErrorFile, line))
        {
            if (line.find("0xvcqtr") != std::string::npos)
                line.replace(line.find("0xvcqtr"), 7, errorStatusCode);
            if (line.find("1xlssld") != std::string::npos)
                line.replace(line.find("1xlssld"), 7, errorMessage);
            if (line.find("2xpoqwq") != std::string::npos)
                line.replace(line.find("2xpoqwq"), 7, pageTitle);
            newErrorFile << line << std::endl;
        }
    }

    newErrorFile.close();
    defaultErrorFile.close();

    // Re-open the new error file to read its contents
    std::ifstream readNewErrorFile(tempFileName.c_str());
    std::ostringstream buffer;
    buffer << readNewErrorFile.rdbuf();
    std::string body = buffer.str();
    readNewErrorFile.close();

    errorBody = body;
    return body;
}

void ErrorHandle::prepareErrorPage(Client *client, std::string statusCode)
{
    std::string errorPage;

    std::map<std::string, std::string> errorPages;
	if (client->_request.begin()->_servConf)
		errorPages = client->_request.begin()->_servConf->getErrorPages();
	else
		errorPages = client->_server->getServerConfig().getErrorPages();

    if (errorPages.empty())
        errorPage = client->_server->getServerConfig().getErrorPage();
    else
    {
        std::map<std::string, std::string>::iterator itError;
        for (itError = errorPages.begin(); itError != errorPages.end(); itError++)
        {
            if (statusCode == itError->first)
            {
                errorPage = itError->second;
                break;
            }
            else
            {
                errorPage = client->_request.begin()->_servConf->getErrorPage();
                if (errorPage.empty())
                    throw std::runtime_error(statusCode);
            }
        }
    }

    std::string statusMessage = "";
    Helper::checkStatus(statusCode, statusMessage);

    errorFile = errorPage;
    std::ifstream file(errorFile.c_str());
    if (!file.is_open())
        throw std::runtime_error("404");

    errorStatusCode = statusCode;
    errorMessage = statusMessage;
    pageTitle = statusCode + " " + statusMessage;
}

std::vector<ErrorHandle> ErrorHandle::getErrorVector()
{
    return errorVector;
}

void ErrorHandle::displayError()
{
    for (size_t i = 0; i < errorVector.size(); i++)
    {
        ErrorHandle& error = errorVector[i];
        std::cout << error.getErrorFile() << std::endl;
        std::cout << error.getErrorStatusCode() << std::endl;
        std::cout << error.getErrorMessage() << std::endl;
    }
}

ErrorHandle::~ErrorHandle()
{
    // -- Remove the error file
    std::remove(newErrorFile.c_str());
}

void ErrorHandle::setErrorFile(std::string errorFile)
{
    this->errorFile = errorFile;
}

void ErrorHandle::setErrorStatusCode(std::string errorStatusCode)
{
    this->errorStatusCode = errorStatusCode;
}

void ErrorHandle::setErrorMessage(std::string errorMessage)
{
    this->errorMessage = errorMessage;
}

std::string ErrorHandle::getErrorFile()
{
    size_t pos = errorFile.find(";");
    errorFile = errorFile.substr(0, pos);
    return errorFile;
}

std::string ErrorHandle::getErrorStatusCode()
{
    return errorStatusCode;
}

std::string ErrorHandle::getErrorMessage()
{
    return errorMessage;
}

std::string ErrorHandle::getNewErrorFile()
{
    return newErrorFile;
}
