#pragma once
#include <memory>
#include <string>
#include "XMLNodeHandle.h"

namespace XMLLib
{
//XMLDocumentHandle is an owning class holding the parsed XML document in DOM format
//since the class holds the actual data, it is non-copyable
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

    std::string ToString() const;

    bool SaveToFile(const std::string& filePath, bool prettyPrint = true);

    XMLNodeHandle GetRootElement() const;
    XMLNodeHandle AppendChild(const XMLNodeHandle& child);

    XMLNodeHandle CreateElement(const std::string& name);
    XMLNodeHandle CreateTextNode(const std::string& text);

    bool HasDocumentElement() const;

    static XMLDocumentHandle CreateEmpty(const std::string& rootName = ""); // used to create new empty xml documents that did were not parsed in
private:
    class ImplXDH;
    std::unique_ptr<ImplXDH> m_ImplXDH; // used to hide xerces headers

    friend class XMLParser;

    void adoptDocument(void* doc);
    void setError(const std::string& err);
};
}