//////////////////////////////////////////////////////////////////////////
// ConfigParser.cpp
//
//////////////////////////////////////////////////////////////////////////
#include "stdafx.h"
#include "ConfigParser.h"
#include <fstream>

//////////////////////////////////////////////////////////////////////////
// ctor
//////////////////////////////////////////////////////////////////////////
ConfigParser::ConfigParser()
{
}
//////////////////////////////////////////////////////////////////////////
// dtor
//////////////////////////////////////////////////////////////////////////
ConfigParser::~ConfigParser()
{
    // delete vector
    for(auto it : mConf)
        delete(it);
}
//////////////////////////////////////////////////////////////////////////
// parse file
//////////////////////////////////////////////////////////////////////////
bool ConfigParser::init(std::string filename)
{
    Section      *sec;
    std::ifstream f;
    // open file
    f.open(filename);
    // check
    if(f.fail())
        return(false);
    // read file
    while(!f.eof())
    {
        char buf[255];
        // get next line
        f.getline(buf,_countof(buf));
        std::string line(Tools::trimStr(buf));
        // skip blank lines
        if(line.empty())
            continue;
        // skip comments
        if(line[0]==';' || line[0]=='#')
            continue;
        // found open section
        if(line[0]=='[')
        {
            std::string::size_type end=line.rfind(']');
            if(end==std::string::npos)
                return(false);
            // add empty section
            mConf.push_back(new Section());
            sec         =mConf[mConf.size()-1];
            sec->section=Tools::trimStr(line.substr(1,end-1));
            continue;
        }
        // found property
        std::string::size_type eqpos=line.find('=');
        Property prop;
        prop.key  =Tools::trimStr(line.substr(0,eqpos));
        prop.value=Tools::trimStr(line.substr(eqpos+1,line.length()));
        sec->properties.push_back(prop);
    }
    // close file
    f.close();
    // success
    return(true);
}
