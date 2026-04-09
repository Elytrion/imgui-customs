#pragma once
#include <string>
#include <unordered_map>
#include "xmllib/XMLDocumentHandle.h"

class DocumentRef
{
public:
    DocumentRef() = default;

    bool IsValid() const;
    XMLLib::XMLDocumentHandle* Get();
    const XMLLib::XMLDocumentHandle* Get() const;

    explicit operator bool() const { return IsValid(); }

private:
    uint32_t m_Slot = InvalidSlot;
    uint32_t m_Generation = 0;

    static constexpr uint32_t InvalidSlot = 0xFFFFFFFFu;

    friend class DocumentStore;
};

struct DocumentStore // pointer stable static slot map
{
    static DocumentRef Add(XMLLib::XMLDocumentHandle doc);
    static bool Remove(const DocumentRef& ref);
    static XMLLib::XMLDocumentHandle Adopt(const DocumentRef& ref);

    static XMLLib::XMLDocumentHandle* Resolve(const DocumentRef& ref);
    static const XMLLib::XMLDocumentHandle* ResolveConst(const DocumentRef& ref);

    static bool IsValid(const DocumentRef& ref);
};