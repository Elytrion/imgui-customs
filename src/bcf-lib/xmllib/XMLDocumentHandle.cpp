#include "XMLDocumentHandle.h"
#include <xercesc/dom/DOM.hpp>
#include <xercesc/framework/LocalFileFormatTarget.hpp>
#include <xercesc/framework/MemBufFormatTarget.hpp>

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

std::string XMLDocumentHandle::toString() const
{
    if (!m_ImplXDH || !m_ImplXDH->document)
        return "";

    DOMImplementation* impl = DOMImplementationRegistry::getDOMImplementation(XMLString::transcode("LS"));
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

}