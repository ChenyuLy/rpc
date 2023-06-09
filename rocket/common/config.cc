#include "rocket/common/config.h"
#include <tinyxml/tinyxml.h>
#include "config.h"


#define READ_XML_NODE(name,parent) \
TiXmlElement* name##_node = parent->FirstChildElement(#name);\
if(!(name##_node)){\
    printf("Start rocket server error,failed to read node [%s]\n", #name);\
    exit(0);\
}\


#define READ_STR_FROM_XML_NODE(name,parent) \
TiXmlElement* name##_node = parent->FirstChildElement(#name);\
if(!name##_node || !name##_node->GetText())\
{printf("Start socket server error, failed to read config file %s\n",#name);exit(0);} \
std::string name##_str = std::string(name##_node->GetText());\


namespace rocket
{
    static Config* g_config = NULL;

    Config* Config::GetGlobalConfig(){
        return g_config;
    }

    void Config::SetGlobelConfig(const char *xmlfile)
    {
        if(g_config == NULL){
            g_config = new Config(xmlfile);
        }
    }

    Config::Config(const char *xmlfile)
    {
        TiXmlDocument* xml_document = new TiXmlDocument();

        bool rt = xml_document->LoadFile(xmlfile);
        if(!rt){
            printf("Start socket server error, failed to read config file %s , error info[%s] \n",xmlfile,xml_document->ErrorDesc());
            xml_document->ErrorDesc();
            exit(0);
        }

        
        // TiXmlElement* root_node = xml_document->FirstChildElement("root");
        // if(!root_node){
        //     printf("Start rocket server error,failed to read node [%s]\n", "root");
        //     exit(0);
        // }
        READ_XML_NODE(root,xml_document);
        READ_XML_NODE(log,root_node);

        // TiXmlElement* log_level_node = log_node->FirstChildElement("log_level");
        // if(!log_level_node || !log_level_node->GetText());
        // {
        //     printf("Start socket server error, failed to read config file %s\n","log_level");
        //     exit(0);
        // }
        // std::string log_level = std::string(log_level_node->GetText());
        READ_STR_FROM_XML_NODE(log_level,log_node);
        READ_STR_FROM_XML_NODE(log_file_name,log_node);
        READ_STR_FROM_XML_NODE(log_file_path,log_node);
        READ_STR_FROM_XML_NODE(log_max_file_size,log_node);
        READ_STR_FROM_XML_NODE(log_sync_inteval,log_node);




        m_log_level = log_level_str;
        m_log_file_name = log_file_name_str ; 
        m_log_file_path= log_file_path_str;
        m_log_max_file_size = std::atoi(log_max_file_size_str.c_str());
        m_log_sync_inteval = std::atoi(log_sync_inteval_str.c_str());

        printf("LOG -- CONFIG LEVEL[%s],[FILE %s],MAX_FILE_SIZE{%d B},SYNC_INTEVAL{%d ms}\n",
          m_log_level.c_str(),(m_log_file_path +" "+ m_log_file_name).c_str(),m_log_max_file_size,m_log_sync_inteval);
    }

} // namespace rocket
