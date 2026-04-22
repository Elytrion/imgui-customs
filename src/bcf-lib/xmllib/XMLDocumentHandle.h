#pragma once
#include <memory>
#include <string>
#include "XMLNodeHandle.h"

namespace XMLLib
{
//XMLDocumentHandle is an owning class holding the parsed XML document in DOM format
//since the class holds the actual data, it is non-copyable and only movable
//The parsed XML document is stored as an internal pointer to the Xerces lib DOMDocument,
//to avoid exposing Xerces headers in the public interface
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

	bool IsValid() const;                                       // indicates whether the handle currently owns a valid DOMDocument
	const std::string& GetLastError() const;                    // if the handle is invalid, this may contain an error message describing why

    // since the XMLDocumentHandle owns the DOMDocument
    // it should be able to save itself
    std::string ToString(bool prettyPrint = true) const;                    // returns the XML document as a string, optionally pretty-printed
    bool SaveToFile(const std::string& filePath, bool prettyPrint = true);  // saves the XML document to a file, optionally pretty-printed

	XMLNodeHandle GetRootElement() const;                       // returns a handle to the root element of the XML document, or an invalid handle if there is no root element
	XMLNodeHandle AppendChild(const XMLNodeHandle& child);      // appends a child node to the root element of the document, returns a handle to the appended child, or an invalid handle if the operation fails (e.g. if there is no root element)

	XMLNodeHandle CreateElement(const std::string& name);       // creates a new element node with the given name, and returns a handle to it (the new node is not yet part of the document tree until it is appended to an existing node)
    XMLNodeHandle CreateTextNode(const std::string& text);      // creates a new text node with the given text, and returns a handle to it (the new node is not yet part of the document tree until it is appended to an existing node)

    bool HasDocumentElement() const;                            // indicates whether the document has a root element

    static XMLDocumentHandle CreateEmpty(const std::string& rootName = ""); // used to create new empty xml documents that were not parsed in
private:
    class ImplXDH;
    std::unique_ptr<ImplXDH> m_ImplXDH; // used to hide xerces headers

	friend class XMLParser;             // XMLParser needs access to adoptDocument to take ownership of parsed DOMDocuments

	void adoptDocument(void* doc);          // used by XMLParser to transfer ownership of a parsed DOMDocument into an XMLDocumentHandle, takes a raw pointer to avoid exposing xerces headers
	void setError(const std::string& err);  // used internally to set the last error message when an operation fails, does not modify the DOMDocument ownership
};
}