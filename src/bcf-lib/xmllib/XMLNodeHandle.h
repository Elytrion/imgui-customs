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

    struct XMLAttribute
    {
        std::string name;
        std::string value;
    };

    // XMLNodeHandle is just a reference to the nodes in the original XMLDocument implemented DOMDocument ptr
    // As such, it is non-owning and copyable and movable, but copies will simply access the same node
    // If the base XMLDocument is deleted the nodes will become invalidated and SHOULD NOT BE USED (this responsibility is on the user of the API)
    class XMLNodeHandle
    {
    public:
        XMLNodeHandle() = default;

		bool IsValid() const;                       // indicates whether this handle currently references a valid XML node. This may return true even if the underlying XML document has been deleted, in which case the handle is considered a dangling reference and should not be used.
		XMLNodeType GetType() const;                // returns the type of the XML node this handle references, or XMLNodeType::Unknown if the handle is invalid

        std::string GetName() const;                // Returns the name of the XML node, or empty if the handle is invalid
		std::string GetValue() const;               // Returns the string value of the XML node, if applicable (e.g. for text nodes or attribute nodes), or an empty string if not applicable or if the handle is invalid
		std::string GetText() const;                // Returns the text content of the XML node and its children, if applicable (e.g. for element nodes), or an empty string if not applicable or if the handle is invalid
		bool SetText(const std::string& value);     // Sets the text content of the XML node, if applicable (e.g. for element nodes), returns true if successful, false if not applicable or if the handle is invalid

		bool IsElement() const;                     // returns true if this handle references an element node, false if it references a different type of node or if the handle is invalid

		XMLNodeHandle GetParent() const;                    // returns a handle to the parent node of this XML node, or an invalid handle if this node has no parent or if the handle is invalid
		XMLNodeHandle GetFirstChild() const;                // returns a handle to the first child node of this XML node, or an invalid handle if this node has no children or if the handle is invalid
		XMLNodeHandle GetNextSibling() const;               // returns a handle to the next sibling node of this XML node, or an invalid handle if this node has no next sibling or if the handle is invalid
		std::vector<XMLNodeHandle> GetChildren() const;     // returns a vector of handles to all child nodes of this XML node, or an empty vector if this node has no children or if the handle is invalid

		XMLNodeHandle GetFirstChildElement(const std::string& name = "") const;             // returns a handle to the first child element node of this XML node that matches the given name (if name is empty, matches any element), or an invalid handle if no matching child element is found or if the handle is invalid
		std::vector<XMLNodeHandle> GetChildElements(const std::string& name = "") const;    // returns a vector of handles to all child element nodes of this XML node that match the given name (if name is empty, matches any element), or an empty vector if no matching child elements are found or if the handle is invalid

		bool HasAttribute(const std::string& name) const;                       // returns true if this XML node is an element and has an attribute with the given name, false if it does not or if the handle is invalid
		std::string GetAttribute(const std::string& name) const;                // returns the value of the attribute with the given name if this XML node is an element and has such an attribute, or an empty string if it does not or if the handle is invalid
		bool SetAttribute(const std::string& name, const std::string& value);   // sets the value of the attribute with the given name if this XML node is an element, creating the attribute if it does not already exist, returns true if successful, false if this node is not an element or if the handle is invalid
		bool RemoveAttribute(const std::string& name);                          // removes the attribute with the given name if this XML node is an element and has such an attribute, returns true if successful, false if it does not or if the handle is invalid

		std::vector<XMLAttribute> GetAttributes() const;                        // returns a vector of all attributes of this XML node if it is an element, or an empty vector if this node is not an element or if the handle is invalid

		XMLNodeHandle AppendChildElement(const std::string& name);      // creates a new element with the given name and appends it as a child of this XML node, returns a handle to the newly created child element, or an invalid handle if this node is not an element or if the handle is invalid
		bool AppendChild(const XMLNodeHandle& child);                   // appends the given child node to this XML node, returns true if successful, false if this node is not an element, if the child node is invalid, or if the handle is invalid

		const void* GetRawNode() const { return m_Node; }               // returns the raw pointer to the underlying XML node, for advanced users who need direct access to the Xerces DOMNode. This is a non-owning pointer and should not be deleted or used after the base XMLDocumentHandle has been deleted, as it will become a dangling pointer.
    private:
        void* m_Node = nullptr;

        explicit XMLNodeHandle(void* rawNode);

        friend class XMLDocumentHandle;
    };
}