#include "XMLMain.hpp"
#include <mutex>
#include <xercesc/util/PlatformUtils.hpp>

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
        XMLPlatformUtils::Initialize();
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
        XMLPlatformUtils::Terminate();
    }
}

bool IsXMLLibInit()
{
    return g_XercesInitRefCount > 0;
}
}

