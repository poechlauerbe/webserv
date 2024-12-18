//-- Written by : msumon

#pragma once

#include "Config.hpp"
#include <map>
#include <string>

//--> TODO- remove mltiple space after "server" and "location" and before "{"

class LocationConfig : public Config
{
    private:
        std::string path;
        std::multimap<std::string, std::string > locationMap;

    public:
        LocationConfig();
        LocationConfig(std::string configFile);
        ~LocationConfig();
        bool    operator==(const LocationConfig& other) const;

        void insertInMap(std::string key, std::string value);
        void setPath(std::string path);
        std::string getPath();
        std::multimap<std::string, std::string > getLocationMap();
};
