#include "BCFDocumentStore.hpp"
namespace
{
    struct Slot
    {
        uint32_t generation = 1;
        bool occupied = false;
        XMLLib::XMLDocumentHandle document;
    };

    std::vector<Slot> g_Slots;
    std::vector<uint32_t> g_FreeSlots;
}

bool DocumentRef::IsValid() const
{
    return DocumentStore::IsValid(*this);
}

XMLLib::XMLDocumentHandle* DocumentRef::Get()
{
    return DocumentStore::Resolve(*this);
}

const XMLLib::XMLDocumentHandle* DocumentRef::Get() const
{
    return DocumentStore::Resolve(*this);
}

DocumentRef DocumentStore::Add(XMLLib::XMLDocumentHandle doc)
{
    DocumentRef ref;

    uint32_t slotIndex;
    if (!g_FreeSlots.empty())
    {
        slotIndex = g_FreeSlots.back();
        g_FreeSlots.pop_back();

        Slot& slot = g_Slots[slotIndex];
        slot.document = std::move(doc);
        slot.occupied = true;

        ref.m_Slot = slotIndex;
        ref.m_Generation = slot.generation;
        return ref;
    }

    slotIndex = static_cast<uint32_t>(g_Slots.size());
    g_Slots.emplace_back();

    Slot& slot = g_Slots.back();
    slot.document = std::move(doc);
    slot.occupied = true;

    ref.m_Slot = slotIndex;
    ref.m_Generation = slot.generation;
    return ref;
}

bool DocumentStore::Remove(const DocumentRef& ref)
{
    if (!IsValid(ref))
        return false;

    Slot& slot = g_Slots[ref.m_Slot];

    slot.document = XMLLib::XMLDocumentHandle{};
    slot.occupied = false;
    ++slot.generation;

    g_FreeSlots.push_back(ref.m_Slot);
    return true;
}

XMLLib::XMLDocumentHandle DocumentStore::Adopt(const DocumentRef& ref)
{
    if (!IsValid(ref))
        return XMLLib::XMLDocumentHandle{};

    Slot& slot = g_Slots[ref.m_Slot];

    XMLLib::XMLDocumentHandle out = std::move(slot.document);

    slot.document = XMLLib::XMLDocumentHandle{};
    slot.occupied = false;
    ++slot.generation;

    g_FreeSlots.push_back(ref.m_Slot);

    return out;
}

bool DocumentStore::IsValid(const DocumentRef& ref)
{
    if (ref.m_Slot == DocumentRef::InvalidSlot)
        return false;

    if (ref.m_Slot >= g_Slots.size())
        return false;

    const Slot& slot = g_Slots[ref.m_Slot];
    if (!slot.occupied)
        return false;

    return slot.generation == ref.m_Generation;
}

XMLLib::XMLDocumentHandle* DocumentStore::Resolve(const DocumentRef& ref)
{
    if (!IsValid(ref))
        return nullptr;

    return &g_Slots[ref.m_Slot].document;
}

const XMLLib::XMLDocumentHandle* DocumentStore::ResolveConst(const DocumentRef& ref)
{
    if (!IsValid(ref))
        return nullptr;

    return &g_Slots[ref.m_Slot].document;
}
