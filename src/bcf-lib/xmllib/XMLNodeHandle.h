#pragma once
#include <string>
#include <vector>

namespace XMLLib
{
    enum class XMLNodeType
    {
        Unknown,
        Document,
        Element,
        Attribute,
        Text,
        Comment,
        CData
    };
    // XMLNodeHandle is just a reference to the nodes in the original XMLDocument implemented DOMDocument ptr
    // As such, it is non-owning and copyable and movable, but copies will simply access the same node
    // If the base XMLDocument is deleted the nodes will become invalidated and SHOULD NOT BE USED
    class XMLNodeHandle
    {
    public:
        XMLNodeHandle() = default;

        bool IsValid() const;
        XMLNodeType GetType() const;

        std::string GetName() const;
        std::string GetValue() const;
        std::string GetText() const;
        bool SetText(const std::string& value);

        bool IsElement() const;

        XMLNodeHandle GetParent() const;
        XMLNodeHandle GetFirstChild() const;
        XMLNodeHandle GetNextSibling() const;
        std::vector<XMLNodeHandle> GetChildren() const;

        XMLNodeHandle GetFirstChildElement(const std::string& name = "") const;
        std::vector<XMLNodeHandle> GetChildElements(const std::string& name = "") const;

        bool HasAttribute(const std::string& name) const;
        std::string GetAttribute(const std::string& name) const;
        bool SetAttribute(const std::string& name, const std::string& value);
        bool RemoveAttribute(const std::string& name);

        XMLNodeHandle AppendChildElement(const std::string& name);
        bool AppendChild(const XMLNodeHandle& child);

    private:
        void* m_Node = nullptr;

        explicit XMLNodeHandle(void* rawNode);

        friend class XMLDocumentHandle;
    };
}