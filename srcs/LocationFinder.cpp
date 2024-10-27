#include "../includes/LocationFinder.hpp"
#include "../includes/Server.hpp"

#include <algorithm>
#include <cstddef>
#include <stdexcept>
#include <string>
#include <vector>
#include <sys/stat.h>
#include <dirent.h>

LocationFinder::LocationFinder()
{
    _root = "";
    _index = "";
    _autoIndex = "";
    _redirect = "";
    _allowed_methods = "";
    _pathToServe = "";
    _locationPath = "";
    _defaultRoot = "./www";

    _cgiFound = false;
    _autoIndexFound = false;
    _allowedMethodFound = false;
    _redirectFound = false;
    
    locationsVector.clear();
}

LocationFinder&	LocationFinder::operator=(const LocationFinder& other)
{
	if (this == &other)
		return *this;
	return *this;
}

LocationFinder::~LocationFinder() {}

//-- Check if the path is a directory.
bool LocationFinder::isDirectory(const std::string &path)
{
    struct stat pathStat;
    if (stat(path.c_str(), &pathStat) == 0 && S_ISDIR(pathStat.st_mode))
        return true;
    return false;
}

void LocationFinder::searchIndexHtml(const std::string &directory, std::string &foundPaths)
{
    DIR* dir = opendir(directory.c_str());
    if (dir == NULL)
    {
        std::cerr << "Failed to open directory: " << directory << std::endl;
        return;
    }

    struct dirent* ent;
    while ((ent = readdir(dir)) != NULL)
    {
        if (std::string(ent->d_name) == "index.html")
        {
            std::string path = directory + "/" + ent->d_name;
            struct stat st;
            if (stat(path.c_str(), &st) == 0 && S_ISREG(st.st_mode))
                foundPaths = path;
            else if (stat(path.c_str(), &st) == 0 && S_ISDIR(st.st_mode))
                searchIndexHtml(path, foundPaths);
        }
    }
    closedir(dir);
}

//-- RequestPath should be the location of the request.
//-- If the request path matches with the location path,
//-- then get the root and index and other values.
//-- It will return false if location not found.
bool LocationFinder::locationMatch(Client *client, std::string path, int _socketFd)
{
    std::string requestPath;
    socketFd = _socketFd;
 
    //-- Remove the last slash from the path to avoid mismatch.
    if (path != "/" && path[path.size() - 1] == '/')
    {
        size_t pos = path.find_last_not_of("/");
        path = path.substr(0, pos + 1);
    }
    requestPath = path;

    //std::cout << BOLD BLUE << "PATH " << path << RESET << std::endl;
    //std::cout << BOLD BLUE << "REQUEST PATH " << requestPath << RESET << std::endl;
	std::vector<LocationConfig>* tmp = client->_server->getLocationConfig(client->_request.getHost(), client->getPort());
	std::cout << "host" << client->_request.getHost() << std::endl;
	if (tmp)
    	locationsVector = *tmp;
	else
	 	throw std::runtime_error("");

    for (size_t i = 0; i < locationsVector.size(); i++)
    {
        std::string tempPath = locationsVector[i].getPath();
        tempPath.erase(std::remove(tempPath.begin(), tempPath.end(), ' '), tempPath.end());
        tempPath.erase(std::remove(tempPath.begin(), tempPath.end(), '{'), tempPath.end());

        if (requestPath == tempPath)
        {
            std::multimap<std::string, std::string> locationMap = locationsVector[i].getLocationMap();
            std::multimap<std::string, std::string>::iterator it;
            
            //-- IF location has no Index or Root, It will search for index,html
            //-- In the current directory, if inde.html is a dir
            //-- It will open it and look for index.html file.
            if (locationMap.find("root") == locationMap.end())
            {
                std::string path = _defaultRoot + tempPath;
                searchIndexHtml(path, _pathToServe);
                //std::cout << BOLD BLUE << "PATH TO SERVE " << _pathToServe << RESET << std::endl;
                return true;
            }

            if (locationMap.find("index") == locationMap.end())
            {
                if (locationMap.find("root") != locationMap.end())
                    _defaultRoot = locationsVector[0].getLocationMap().find("root")->second;
                std::string path = _defaultRoot + tempPath;
                searchIndexHtml(path, _pathToServe);
                //std::cout << BOLD BLUE << "PATH TO SERVE " << _pathToServe << RESET << std::endl;
                return true;
            }            

            if (requestPath.find("cgi-bin") != std::string::npos) {
                _cgiFound = true;
                _root = locationMap.find("root")->second;
                _index = locationMap.find("index")->second;
                _allowed_methods = locationMap.find("allowed_methods")->second;
                return true;
            }

            for (it = locationMap.begin(); it != locationMap.end(); it++)
            {
                if (it->first == "root")
                    _root = it->second;
                if (it->first == "index")
                    _index = it->second;
                if (it->first == "return") {
                    _redirect = it->second;
                    _redirectFound = true;
                }
                if (it->first == "autoindex") {
                    _autoIndex = it->second;
                    _autoIndexFound = true;
                }
                if (it->first == "allowed_methods") {
                    _allowed_methods = it->second;
                    _allowedMethodFound = true;
                }
            }
            _pathToServe = _root + tempPath + "/" + _index;
            _locationPath = tempPath;

            //std::cout << BOLD BLUE << "PATH TO SERVE " << _pathToServe << RESET << std::endl;
            
            return true;
        }
    }
    _root = locationsVector[0].getLocationMap().find("root")->second;
    _pathToServe = _root + _locationPath + path;
    //std::cout << BOLD RED << "PATH TO SERVE " << _pathToServe << RESET << std::endl;
    return false;
}
