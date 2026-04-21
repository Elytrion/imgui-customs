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

bool BCFDocumentRef::IsValid() const
{
    return BCFDocumentStore::IsValid(*this);
}

XMLLib::XMLDocumentHandle* BCFDocumentRef::Get()
{
    return BCFDocumentStore::Resolve(*this);
}

const XMLLib::XMLDocumentHandle* BCFDocumentRef::Get() const
{
    return BCFDocumentStore::ResolveConst(*this);
}

BCFDocumentRef BCFDocumentStore::Add(XMLLib::XMLDocumentHandle doc)
{
    BCFDocumentRef ref;

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

bool BCFDocumentStore::Remove(const BCFDocumentRef& ref)
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

XMLLib::XMLDocumentHandle BCFDocumentStore::Adopt(const BCFDocumentRef& ref)
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

bool BCFDocumentStore::IsValid(const BCFDocumentRef& ref)
{
    if (ref.m_Slot == BCFDocumentRef::InvalidSlot)
        return false;

    if (ref.m_Slot >= g_Slots.size())
        return false;

    const Slot& slot = g_Slots[ref.m_Slot];
    if (!slot.occupied)
        return false;

    return slot.generation == ref.m_Generation;
}

XMLLib::XMLDocumentHandle* BCFDocumentStore::Resolve(const BCFDocumentRef& ref)
{
    if (!IsValid(ref))
        return nullptr;

    return &g_Slots[ref.m_Slot].document;
}

const XMLLib::XMLDocumentHandle* BCFDocumentStore::ResolveConst(const BCFDocumentRef& ref)
{
    if (!IsValid(ref))
        return nullptr;

    return &g_Slots[ref.m_Slot].document;
}

void BCFDocumentStore::Clear()
{
    g_Slots.clear();
    g_FreeSlots.clear();
}
