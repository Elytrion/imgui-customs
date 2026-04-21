#pragma once
#include <string>
#include <unordered_map>
#include "xmllib/XMLDocumentHandle.h"

class BCFDocumentRef
{
public:
    BCFDocumentRef() = default;

    bool IsValid() const;
    XMLLib::XMLDocumentHandle* Get();
    const XMLLib::XMLDocumentHandle* Get() const;

    explicit operator bool() const { return IsValid(); }

private:
    uint32_t m_Slot = InvalidSlot;
    uint32_t m_Generation = 0;

    static constexpr uint32_t InvalidSlot = 0xFFFFFFFFu;

    friend class BCFDocumentStore;
};

struct BCFDocumentStore // pointer stable static slot map
{
    static BCFDocumentRef Add(XMLLib::XMLDocumentHandle doc);
    static bool Remove(const BCFDocumentRef& ref);
    static XMLLib::XMLDocumentHandle Adopt(const BCFDocumentRef& ref);

    static XMLLib::XMLDocumentHandle* Resolve(const BCFDocumentRef& ref);
    static const XMLLib::XMLDocumentHandle* ResolveConst(const BCFDocumentRef& ref);

    static bool IsValid(const BCFDocumentRef& ref);

	static void Clear();
};