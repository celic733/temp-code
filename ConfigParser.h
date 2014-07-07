//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
#pragma once

//////////////////////////////////////////////////////////////////////////
// configuration file parser
//////////////////////////////////////////////////////////////////////////
class ConfigParser
{
public:
    // property pair
    struct Property
    {
        std::string key;
        std::string value;
    };
    // section
    struct Section
    {
        std::string section;
        std::vector<Property> properties;
    };

private:
    // list of sections with children
    std::vector<Section *> mConf;

public:
    // ctor/dtor
    ConfigParser();
    ~ConfigParser();
    // init and parse
    bool    init(std::string filename);
    // access config
    size_t  sectionCount()              { return(mConf.size());  }
    const Section *sectionGet(int pos)  { return(mConf.at(pos)); }
};
