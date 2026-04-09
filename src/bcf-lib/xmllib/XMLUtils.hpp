#include <xercesc/util/TransService.hpp>
#include <string>

namespace XMLLib
{
class XMLChGuard
{
public:
    explicit XMLChGuard(const char* text)
        : m_Ptr(xercesc::XMLString::transcode(text)) {
    }

    ~XMLChGuard()
    {
        if (m_Ptr)
            xercesc::XMLString::release(&m_Ptr);
    }

    XMLCh* get() const { return m_Ptr; }

private:
    XMLCh* m_Ptr = nullptr;
};

static std::string XmlChToString(const XMLCh* text)
{
    if (!text)
        return {};

    char* temp = xercesc::XMLString::transcode(text);
    if (!temp)
        return {};

    std::string out(temp);
    xercesc::XMLString::release(&temp);
    return out;
}

}