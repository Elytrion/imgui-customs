#include <xercesc/util/TransService.hpp>
#include <string>

namespace XMLLib
{
// ease of access class to free transcoded text
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

// convenience function to transcode XMLCh* to std::string, returns empty string if input is null or transcoding fails
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