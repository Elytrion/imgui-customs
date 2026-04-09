#include "XMLNodeHandle.h"

#include <xercesc/dom/DOM.hpp>
#include "XMLUtils.hpp"

using namespace xercesc;

namespace
{
    inline DOMNode* ToNode(void* ptr)
    {
        return static_cast<DOMNode*>(ptr);
    }

    inline const DOMNode* ToNode(const void* ptr)
    {
        return static_cast<const DOMNode*>(ptr);
    }

    XMLLib::XMLNodeType ConvertNodeType(const DOMNode* node)
    {
        if (!node)
            return XMLLib::XMLNodeType::Unknown;

        switch (node->getNodeType())
        {
        case DOMNode::DOCUMENT_NODE:      return XMLLib::XMLNodeType::Document;
        case DOMNode::ELEMENT_NODE:       return XMLLib::XMLNodeType::Element;
        case DOMNode::ATTRIBUTE_NODE:     return XMLLib::XMLNodeType::Attribute;
        case DOMNode::TEXT_NODE:          return XMLLib::XMLNodeType::Text;
        case DOMNode::COMMENT_NODE:       return XMLLib::XMLNodeType::Comment;
        case DOMNode::CDATA_SECTION_NODE: return XMLLib::XMLNodeType::CData;
        default:                          return XMLLib::XMLNodeType::Unknown;
        }
    }

    DOMElement* AsElement(DOMNode* node)
    {
        if (!node || node->getNodeType() != DOMNode::ELEMENT_NODE)
            return nullptr;
        return static_cast<DOMElement*>(node);
    }

    const DOMElement* AsElement(const DOMNode* node)
    {
        if (!node || node->getNodeType() != DOMNode::ELEMENT_NODE)
            return nullptr;
        return static_cast<const DOMElement*>(node);
    }
}

namespace XMLLib
{
    XMLNodeHandle::XMLNodeHandle(void* rawNode)
        : m_Node(rawNode)
    {
    }

    bool XMLNodeHandle::IsValid() const
    {
        return m_Node != nullptr;
    }

    XMLNodeType XMLNodeHandle::GetType() const
    {
        return ConvertNodeType(ToNode(m_Node));
    }

    std::string XMLNodeHandle::GetName() const
    {
        const DOMNode* node = ToNode(m_Node);
        if (!node)
            return {};

        return XmlChToString(node->getNodeName());
    }

    std::string XMLNodeHandle::GetValue() const
    {
        const DOMNode* node = ToNode(m_Node);
        if (!node)
            return {};

        return XmlChToString(node->getNodeValue());
    }

    std::string XMLNodeHandle::GetText() const
    {
        const DOMNode* node = ToNode(m_Node);
        if (!node)
            return {};

        return XmlChToString(node->getTextContent());
    }

    bool XMLNodeHandle::SetText(const std::string& value)
    {
        DOMNode* node = ToNode(m_Node);
        if (!node)
            return false;

        XMLChGuard xmlValue(value.c_str());
        node->setTextContent(xmlValue.get());
        return true;
    }

    bool XMLNodeHandle::IsElement() const
    {
        const DOMNode* node = ToNode(m_Node);
        return node && node->getNodeType() == DOMNode::ELEMENT_NODE;
    }

    XMLNodeHandle XMLNodeHandle::GetParent() const
    {
        const DOMNode* node = ToNode(m_Node);
        return node ? XMLNodeHandle(node->getParentNode()) : XMLNodeHandle();
    }

    XMLNodeHandle XMLNodeHandle::GetFirstChild() const
    {
        const DOMNode* node = ToNode(m_Node);
        return node ? XMLNodeHandle(node->getFirstChild()) : XMLNodeHandle();
    }

    XMLNodeHandle XMLNodeHandle::GetNextSibling() const
    {
        const DOMNode* node = ToNode(m_Node);
        return node ? XMLNodeHandle(node->getNextSibling()) : XMLNodeHandle();
    }

    std::vector<XMLNodeHandle> XMLNodeHandle::GetChildren() const
    {
        std::vector<XMLNodeHandle> out;
        const DOMNode* node = ToNode(m_Node);
        if (!node)
            return out;

        for (DOMNode* child = node->getFirstChild(); child; child = child->getNextSibling())
            out.push_back(XMLNodeHandle(child));

        return out;
    }

    XMLNodeHandle XMLNodeHandle::GetFirstChildElement(const std::string& name) const
    {
        const DOMNode* node = ToNode(m_Node);
        if (!node)
            return XMLNodeHandle();

        for (DOMNode* child = node->getFirstChild(); child; child = child->getNextSibling())
        {
            if (child->getNodeType() != DOMNode::ELEMENT_NODE)
                continue;

            if (name.empty() || XmlChToString(child->getNodeName()) == name)
                return XMLNodeHandle(child);
        }

        return XMLNodeHandle();
    }

    std::vector<XMLNodeHandle> XMLNodeHandle::GetChildElements(const std::string& name) const
    {
        std::vector<XMLNodeHandle> out;
        const DOMNode* node = ToNode(m_Node);
        if (!node)
            return out;

        for (DOMNode* child = node->getFirstChild(); child; child = child->getNextSibling())
        {
            if (child->getNodeType() != DOMNode::ELEMENT_NODE)
                continue;

            if (name.empty() || XmlChToString(child->getNodeName()) == name)
                out.push_back(XMLNodeHandle(child));
        }

        return out;
    }

    bool XMLNodeHandle::HasAttribute(const std::string& name) const
    {
        const DOMElement* elem = AsElement(ToNode(m_Node));
        if (!elem)
            return false;

        XMLChGuard xmlName(name.c_str());
        return elem->hasAttribute(xmlName.get());
    }

    std::vector<XMLAttribute> XMLNodeHandle::GetAttributes() const
    {
        std::vector<XMLAttribute> out;

        DOMElement* elem = AsElement(ToNode(m_Node));
        if (!elem)
            return out;

        DOMNamedNodeMap* attrs = elem->getAttributes();
        if (!attrs)
            return out;

        const XMLSize_t count = attrs->getLength();
        out.reserve(static_cast<size_t>(count));

        for (XMLSize_t i = 0; i < count; ++i)
        {
            DOMNode* attr = attrs->item(i);
            if (!attr)
                continue;

            XMLAttribute a;
            a.name = XmlChToString(attr->getNodeName());
            a.value = XmlChToString(attr->getNodeValue());
            out.push_back(std::move(a));
        }

        return out;
    }

    std::string XMLNodeHandle::GetAttribute(const std::string& name) const
    {
        const DOMElement* elem = AsElement(ToNode(m_Node));
        if (!elem)
            return {};

        XMLChGuard xmlName(name.c_str());
        return XmlChToString(elem->getAttribute(xmlName.get()));
    }

    bool XMLNodeHandle::SetAttribute(const std::string& name, const std::string& value)
    {
        DOMElement* elem = AsElement(ToNode(m_Node));
        if (!elem)
            return false;

        XMLChGuard xmlName(name.c_str());
        XMLChGuard xmlValue(value.c_str());
        elem->setAttribute(xmlName.get(), xmlValue.get());
        return true;
    }

    bool XMLNodeHandle::RemoveAttribute(const std::string& name)
    {
        DOMElement* elem = AsElement(ToNode(m_Node));
        if (!elem)
            return false;

        XMLChGuard xmlName(name.c_str());
        elem->removeAttribute(xmlName.get());
        return true;
    }

    XMLNodeHandle XMLNodeHandle::AppendChildElement(const std::string& name)
    {
        DOMNode* node = ToNode(m_Node);
        if (!node)
            return XMLNodeHandle();

        DOMDocument* doc = nullptr;
        if (node->getNodeType() == DOMNode::DOCUMENT_NODE)
            doc = static_cast<DOMDocument*>(node);
        else
            doc = node->getOwnerDocument();

        if (!doc)
            return XMLNodeHandle();

        XMLChGuard xmlName(name.c_str());
        DOMElement* elem = doc->createElement(xmlName.get());
        if (!elem)
            return XMLNodeHandle();

        return XMLNodeHandle(node->appendChild(elem));
    }

    bool XMLNodeHandle::AppendChild(const XMLNodeHandle& child)
    {
        DOMNode* parentNode = ToNode(m_Node);
        DOMNode* childNode = ToNode(child.m_Node);

        if (!parentNode || !childNode)
            return false;

        parentNode->appendChild(childNode);
        return true;
    }

}