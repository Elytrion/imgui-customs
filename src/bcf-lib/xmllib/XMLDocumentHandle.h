#pragma once
#include <memory>
#include <string>

namespace XMLLib
{
class XMLDocumentHandle
{
public:
    XMLDocumentHandle();
    ~XMLDocumentHandle();
    // XMLDocumentHandle is movable but not copyable, to allow transfer of ownership of the
    // internal DOMDocument pointer without risking double deletion issues
    XMLDocumentHandle(XMLDocumentHandle&&) noexcept;
    XMLDocumentHandle& operator=(XMLDocumentHandle&&) noexcept;
    XMLDocumentHandle(const XMLDocumentHandle&) = delete;
    XMLDocumentHandle& operator=(const XMLDocumentHandle&) = delete;

    bool IsValid() const;
    const std::string& GetLastError() const;
private:
    class ImplXDH;
    std::unique_ptr<ImplXDH> m_ImplXDH; // used to hide xerces headers

    friend class XMLParser;

    void adoptDocument(void* doc);
    void setError(const std::string& err);
};
}