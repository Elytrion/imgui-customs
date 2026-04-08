#include "XMLDocumentHandle.h"
#include <xercesc/dom/DOM.hpp>

namespace XMLLib
{

class XMLDocumentHandle::ImplXDH
{
public:
    xercesc::DOMDocument* document = nullptr;
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
    xercesc::DOMDocument* domDoc = static_cast<xercesc::DOMDocument*>(doc);
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

}