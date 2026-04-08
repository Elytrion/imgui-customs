#include "XMLMain.hpp"
#include <mutex>
#include <xercesc/util/PlatformUtils.hpp>
#include <xercesc/util/XMLString.hpp>
#include <iostream>
using namespace xercesc;

namespace
{
	std::mutex g_XercesInitMutex;
	int g_XercesInitRefCount = 0;
}
namespace XMLLib
{
void InitXMLLib()
{
    std::lock_guard<std::mutex> lock(g_XercesInitMutex);
    if (g_XercesInitRefCount == 0)
    {
        try
        {
            XMLPlatformUtils::Initialize();
        }
        catch (const XMLException& toCatch) {
            char* message = XMLString::transcode(toCatch.getMessage());
            std::cout << "Error during initialization! :\n"
                << message << "\n" << std::endl;
            XMLString::release(&message);
            return;
        }
        
    }
    ++g_XercesInitRefCount;
}

void ReleaseXMLLib()
{
    std::lock_guard<std::mutex> lock(g_XercesInitMutex);
    if (g_XercesInitRefCount <= 0)
        return;
    --g_XercesInitRefCount;
    if (g_XercesInitRefCount == 0)
    {
        try
        {
            XMLPlatformUtils::Terminate();
        }
        catch (const XMLException& toCatch) {
            char* message = XMLString::transcode(toCatch.getMessage());
            std::cout << "Error during termination! :\n"
                << message << "\n" << std::endl;
            XMLString::release(&message);
            return;
        }
    }
}

bool IsXMLLibInit()
{
    return g_XercesInitRefCount > 0;
}
}

