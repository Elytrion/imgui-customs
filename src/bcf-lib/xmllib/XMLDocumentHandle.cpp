#include "XMLDocumentHandle.h"
#include <xercesc/dom/DOM.hpp>
#include <xercesc/framework/LocalFileFormatTarget.hpp>
#include <xercesc/framework/MemBufFormatTarget.hpp>

#include "XMLUtils.hpp"

using namespace xercesc;

namespace XMLLib
{

class XMLDocumentHandle::ImplXDH
{
public:
    DOMDocument* document = nullptr;
    std::string lastError;
    ImplXDH() = default;
    ~ImplXDH()
    {
        if (document)
        {
            document->release();
            document = nullptr;
        }
    }
};


XMLDocumentHandle::XMLDocumentHandle()
    : m_ImplXDH(std::make_unique<ImplXDH>())
{
}

XMLDocumentHandle::~XMLDocumentHandle()
{
    m_ImplXDH.reset();
}
XMLDocumentHandle::XMLDocumentHandle(XMLDocumentHandle&& other) noexcept:
    m_ImplXDH(std::move(other.m_ImplXDH))
{
}
XMLDocumentHandle& XMLDocumentHandle::operator=(XMLDocumentHandle&& other) noexcept
{
    if (this != &other) {
        m_ImplXDH = std::move(other.m_ImplXDH);
    }
    return *this;
}

bool XMLDocumentHandle::IsValid() const
{
    return m_ImplXDH && m_ImplXDH->document != nullptr;
}

const std::string& XMLDocumentHandle::GetLastError() const
{
    return m_ImplXDH->lastError;
}

void XMLDocumentHandle::adoptDocument(void* doc)
{
    DOMDocument* domDoc = static_cast<DOMDocument*>(doc);
    if (!m_ImplXDH)
        m_ImplXDH = std::make_unique<ImplXDH>();

    if (m_ImplXDH->document)
        m_ImplXDH->document->release();

    m_ImplXDH->document = domDoc;
    m_ImplXDH->lastError.clear();
}

void XMLDocumentHandle::setError(const std::string& err)
{
    if (!m_ImplXDH)
        m_ImplXDH = std::make_unique<ImplXDH>();

    m_ImplXDH->lastError = err;
}

std::string XMLDocumentHandle::ToString() const
{
    if (!m_ImplXDH || !m_ImplXDH->document)
        return "";

    XMLChGuard domImplement("LS");
    DOMImplementation* impl = DOMImplementationRegistry::getDOMImplementation(domImplement.get());
    if (!impl)
        return "";

    DOMLSSerializer* serializer = ((DOMImplementationLS*)impl)->createLSSerializer();

    // Pretty print (optional but nice)
    DOMConfiguration* config = serializer->getDomConfig();
    if (config->canSetParameter(XMLUni::fgDOMWRTFormatPrettyPrint, true))
        config->setParameter(XMLUni::fgDOMWRTFormatPrettyPrint, true);

    MemBufFormatTarget target;
    DOMLSOutput* output = ((DOMImplementationLS*)impl)->createLSOutput();

    output->setByteStream(&target);

    serializer->write(m_ImplXDH->document, output);

    std::string result(
        reinterpret_cast<const char*>(target.getRawBuffer()),
        target.getLen()
    );

    output->release();
    serializer->release();

    return result;
}

bool XMLDocumentHandle::SaveToFile(const std::string& filePath, bool prettyPrint)
{
    if (!m_ImplXDH || !m_ImplXDH->document)
    {
        setError("No XML document to save");
        return false;
    }

    try
    {
        XMLChGuard domImplement("LS");
        DOMImplementation* impl = DOMImplementationRegistry::getDOMImplementation(domImplement.get());
        if (!impl)
        {
            setError("Failed to get DOM LS implementation");
            return false;
        }

        auto* implLS = dynamic_cast<DOMImplementationLS*>(impl);
        if (!implLS)
        {
            setError("DOM implementation does not support LS");
            return false;
        }

        DOMLSSerializer* serializer = implLS->createLSSerializer();
        DOMConfiguration* config = serializer->getDomConfig();

        if (config && config->canSetParameter(XMLUni::fgDOMWRTFormatPrettyPrint, prettyPrint))
            config->setParameter(XMLUni::fgDOMWRTFormatPrettyPrint, prettyPrint);

        DOMLSOutput* output = implLS->createLSOutput();

        XMLFormatTarget* target = new LocalFileFormatTarget(filePath.c_str());
        output->setByteStream(target);

        const bool ok = serializer->write(m_ImplXDH->document, output);

        delete target;
        output->release();
        serializer->release();

        if (!ok)
        {
            setError("Serializer failed to write XML file");
            return false;
        }

        m_ImplXDH->lastError.clear();
        return true;
    }
    catch (const XMLException& e)
    {
        setError("XMLException while saving: " + XmlChToString(e.getMessage()));
        return false;
    }
    catch (const DOMException& e)
    {
        setError("DOMException while saving: " + XmlChToString(e.msg));
        return false;
    }
    catch (const std::exception& e)
    {
        setError(std::string("std::exception while saving: ") + e.what());
        return false;
    }
    catch (...)
    {
        setError("Unknown exception while saving XML");
        return false;
    }
}



XMLNodeHandle XMLDocumentHandle::GetRootElement() const
{
    if (!HasDocumentElement())
        return XMLNodeHandle();

    xercesc::DOMElement* root = m_ImplXDH->document->getDocumentElement();
    if (!root)
        return XMLNodeHandle();

    return XMLNodeHandle(static_cast<void*>(root));
}

XMLNodeHandle XMLDocumentHandle::CreateElement(const std::string& name)
{
    if (!m_ImplXDH || !m_ImplXDH->document)
        return XMLNodeHandle();

    try
    {
        XMLChGuard xmlName(name.c_str());
        xercesc::DOMElement* elem = m_ImplXDH->document->createElement(xmlName.get());
        if (!elem)
            return XMLNodeHandle();

        return XMLNodeHandle(static_cast<void*>(elem));
    }
    catch (const xercesc::DOMException& e)
    {
        setError("DOMException creating element: " + XmlChToString(e.msg));
        return XMLNodeHandle();
    }
}

XMLNodeHandle XMLDocumentHandle::CreateTextNode(const std::string& text)
{
    if (!m_ImplXDH || !m_ImplXDH->document)
        return XMLNodeHandle();

    try
    {
        XMLChGuard xmlText(text.c_str());
        xercesc::DOMText* textNode = m_ImplXDH->document->createTextNode(xmlText.get());
        if (!textNode)
            return XMLNodeHandle();

        return XMLNodeHandle(static_cast<void*>(textNode));
    }
    catch (const xercesc::DOMException& e)
    {
        setError("DOMException creating text node: " + XmlChToString(e.msg));
        return XMLNodeHandle();
    }
}

XMLDocumentHandle XMLDocumentHandle::CreateEmpty(const std::string& rootName)
{
    XMLDocumentHandle result;

    try
    {
        XMLChGuard coreText("Core");
        DOMImplementation* impl = DOMImplementationRegistry::getDOMImplementation(coreText.get());
        if (!impl)
        {
            result.setError("Failed to get DOM Core implementation");
            return result;
        }

        DOMDocument* doc = nullptr;

        if (rootName.empty())
        {
            doc = impl->createDocument();
        }
        else
        {
            XMLChGuard xmlRootName(rootName.c_str());
            doc = impl->createDocument(
                nullptr,                 
                xmlRootName.get(),       
                nullptr                  
            );
        }

        if (!doc)
        {
            result.setError("Failed to create empty XML document");
            return result;
        }

        result.adoptDocument(static_cast<void*>(doc));
        return result;
    }
    catch (const XMLException& e)
    {
        result.setError("XMLException creating empty document: " + XmlChToString(e.getMessage()));
        return result;
    }
    catch (const DOMException& e)
    {
        result.setError("DOMException creating empty document: " + XmlChToString(e.msg));
        return result;
    }
    catch (const std::exception& e)
    {
        result.setError(std::string("std::exception creating empty document: ") + e.what());
        return result;
    }
    catch (...)
    {
        result.setError("Unknown exception creating empty document");
        return result;
    }
}

bool XMLDocumentHandle::HasDocumentElement() const
{
    return m_ImplXDH && m_ImplXDH->document && m_ImplXDH->document->getDocumentElement() != nullptr;
}
XMLNodeHandle XMLDocumentHandle::AppendChild(const XMLNodeHandle& child)
{
    if (!m_ImplXDH || !m_ImplXDH->document)
    {
        setError("No XML document");
        return XMLNodeHandle();
    }

    if (!child.IsValid())
    {
        setError("Invalid child node");
        return XMLNodeHandle();
    }

    try
    {
        DOMNode* rawChild = static_cast<DOMNode*>(child.m_Node);
        if (!rawChild)
        {
            setError("Invalid raw child node");
            return XMLNodeHandle();
        }

        DOMDocument* ownerDoc = nullptr;
        if (rawChild->getNodeType() == DOMNode::DOCUMENT_NODE)
        {
            setError("Cannot append a document node to a document");
            return XMLNodeHandle();
        }
        else
        {
            ownerDoc = rawChild->getOwnerDocument();
        }

        if (ownerDoc != m_ImplXDH->document)
        {
            setError("Child node belongs to a different document");
            return XMLNodeHandle();
        }

        DOMNode* appended = m_ImplXDH->document->appendChild(rawChild);
        m_ImplXDH->lastError.clear();
        return XMLNodeHandle(static_cast<void*>(appended));
    }
    catch (const DOMException& e)
    {
        setError("DOMException appending child to document: " + XmlChToString(e.msg));
        return XMLNodeHandle();
    }
    catch (...)
    {
        setError("Unknown exception appending child to document");
        return XMLNodeHandle();
    }
}
}