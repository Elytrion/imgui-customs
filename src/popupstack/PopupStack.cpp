#include "PopupStack.h"
#include <algorithm>

// Create a unique "Title###popup_<id>" label so visible title can rename safely.
std::string PopupStack::makeLabel(const std::string& title)
{
    ++m_token_counter;
    return title + "###popup_" + std::to_string(m_token_counter);
}

PopupHandle PopupStack::Reserve(const std::string& title,
    const PopupConfig& cfg,
    std::function<void()> draw,
    bool open_immediately,
    bool reusable)
{
    // Create/own label text in handles_ list so its c_str() is stable.
    m_popupHandles.emplace_back(makeLabel(title));
    HandleNodeIt it = std::prev(m_popupHandles.end());
    PopupHandle  h = it->c_str();

    // Insert/replace store definition
    PopupDef d;
    d.title = title;
    d.label = *it;
    d.cfg = cfg;
    d.draw = std::move(draw);
	d.reusable = reusable; 
    m_popupDefs[h] = std::move(d);

    if (open_immediately) {
        AddToDrawStack(h, /*float_up=*/true, reusable);
    }
    return h;
}

void PopupStack::ModifyReserved( PopupHandle h, const PopupConfig& cfg, std::function<void()> draw)
{
    if (!h || !m_popupDefs.count(h))
        return;
    PopupDef& d = m_popupDefs[h];
    d.cfg = cfg; // update config
    if (draw)
        d.draw = std::move(draw); // update draw function
    // If already active, re-arm for next draw
    for (auto& a : m_drawStack)
    {
        if (a.handle == h)
        {
            a.opened_once = false; // re-arm for next draw
            break;
        }
	}
}

PopupHandle PopupStack::Open(const std::string& title,
    const PopupConfig& cfg,
    std::function<void()> draw)
{
    // open_immediately=true, reusable=false
    return Reserve(title, cfg, std::move(draw), /*open_immediately=*/true, /*reusable=*/false);
}

void PopupStack::AddToDrawStack(PopupHandle h, bool float_up, bool reusable)
{
    if (!h)
        return;
    if (!m_popupDefs.count(h))
        return;
	bool reusable_def = m_popupDefs[h].reusable;
	reusable = reusable || reusable_def; // respect def reusability

    // If already active, optionally float to back (top-most)
    for (size_t i = 0; i < m_drawStack.size(); ++i)
    {
        if (m_drawStack[i].handle == h)
        {
            if (float_up && i + 1 != m_drawStack.size())
            {
                auto tmp = std::move(m_drawStack[i]);
                m_drawStack.erase(m_drawStack.begin() + int(i));
                m_drawStack.push_back(std::move(tmp));
            }
            // Re-arm if necessary (will be handled in DrawAll prepass)
            m_drawStack.back().opened_once = false;
            return;
        }
    }

    // Push new active item
    ActiveItem a;
    a.handle = h;
    a.reusable = reusable;
    a.opened_once = false;      // must call ImGui::OpenPopup once
    a.close_requested = false;
    m_drawStack.push_back(a);
}

void PopupStack::Close(PopupHandle h)
{
    if (!h)
    {
        if (!m_drawStack.empty())
            m_drawStack.back().close_requested = true;
        return;
    }
    for (auto& a : m_drawStack)
    {
        if (a.handle == h)
        {
            a.close_requested = true;
            return;
        }
    }
    assert(false); // Handle was not found within the draw stack!
}

void PopupStack::DeleteReserved(PopupHandle h)
{
    if (!h) return;

    // If active: mark to close and make non-reusable; clean after closing
    bool found_active = false;
    for (auto& a : m_drawStack)
    {
        if (a.handle == h)
        {
            a.close_requested = true;
            a.reusable = false;
            found_active = true;
            break;
        }
    }
    // Remove def immediately; keep handle node until clean (so 'h' stays valid until inactive)
    m_popupDefs.erase(h);
    m_pending_clean = true;

    // If not active: also remove handle node now
    if (!found_active)
        removeHandleNode(h);
}

void PopupStack::setupNext(const PopupDef& d)
{
    ImGuiViewport* vp = ImGui::GetMainViewport();

    // Size: only hint on appearing; allow auto on any axis = 0
    ImVec2 size{ 0,0 };
    switch (d.cfg.size_mode)
    {
    case PopupSizeMode::PIXELS:
        size = d.cfg.size;
        ImGui::SetNextWindowSize(size, ImGuiCond_Appearing);
        break;
    case PopupSizeMode::PERCENT:
        size.x = (d.cfg.size.x > 0.f) ? vp->Size.x * d.cfg.size.x : 0.f;
        size.y = (d.cfg.size.y > 0.f) ? vp->Size.y * d.cfg.size.y : 0.f;
        ImGui::SetNextWindowSize(size, ImGuiCond_Appearing);
        break;
    case PopupSizeMode::AUTO: default:
        break;
    }

	// we set the other positions in the recursive drawFrom() call
	// since we need the actual size of the popup to accurately calculate the position
    ImVec2 pos = vp->GetCenter();
    ImVec2 pivot = ImVec2(0.5f, 0.5f);
    switch (d.cfg.position)
    {
    case PopupPosition::TOP_LEFT:
        pos = ImVec2(vp->Pos.x + d.cfg.padding, vp->Pos.y + d.cfg.padding);
        pivot = ImVec2(0.f, 0.f);
        break;
    case PopupPosition::TOP_RIGHT:
        pos = ImVec2(vp->Pos.x + vp->Size.x - d.cfg.padding, vp->Pos.y + d.cfg.padding);
        pivot = ImVec2(1.f, 0.f);
        break;
    case PopupPosition::BOTTOM_LEFT:
        pos = ImVec2(vp->Pos.x + d.cfg.padding, vp->Pos.y + vp->Size.y - d.cfg.padding);
        pivot = ImVec2(0.f, 1.f);
        break;
    case PopupPosition::BOTTOM_RIGHT:
        pos = ImVec2(vp->Pos.x + vp->Size.x - d.cfg.padding, vp->Pos.y + vp->Size.y - d.cfg.padding);
        pivot = ImVec2(1.f, 1.f);
        break;
    case PopupPosition::CENTERED: default:
        // already set to center
        break;
    }
	ImGui::SetNextWindowPos(pos, ImGuiCond_Appearing, pivot);

    ImGui::SetNextWindowViewport(vp->ID);
}

void PopupStack::drawFrom(size_t k, std::vector<size_t>& to_remove)
{
    if (k >= m_drawStack.size())
        return;

    ActiveItem& a = m_drawStack[k];

    // Lost definition? Remove and (crucially) try to continue the chain.
    auto it = m_popupDefs.find(a.handle);
    if (it == m_popupDefs.end())
    {
        to_remove.push_back(k);
        if (k + 1 < m_drawStack.size()) drawFrom(k + 1, to_remove);
        return;
    }
    PopupDef& d = it->second;

    setupNext(d);

    // Arm THIS popup right before its Begin, but only once per open.
    if (!a.opened_once)
    {
        ImGui::OpenPopup(d.label.c_str());
        a.opened_once = true;
    }

    const bool open = d.cfg.modal
        ? ImGui::BeginPopupModal(d.label.c_str(), nullptr, d.cfg.flags)
        : ImGui::BeginPopup(d.label.c_str(), d.cfg.flags);

    if (!open)
    {
        // If parent didn't open, no child can open this frame (nesting requirement).
        // If ImGui fully closed it, queue removal and attempt to continue chain
        // (this won't draw anything because child needs parent, but it prevents
        // us from "stalling" cleanup when multiple items are queued).
        if (!ImGui::IsPopupOpen(d.label.c_str(), ImGuiPopupFlags_AnyPopupId))
            to_remove.push_back(k);
        return;
    }

    // If this popup is marked to close: open child FIRST
    if (a.close_requested)
    {
        if (k + 1 < m_drawStack.size())
        {
            ActiveItem& child = m_drawStack[k + 1];
            if (!child.opened_once)
            {
                // if a parent is closed before a child is even open
                // we assume that the child will open anyways
                // if not the child will never open!
                // We do this by deferring the open to the next frame
                child.defer_open = true;
            }
            //drawFrom(k + 1, to_remove);
        }
        ImGui::CloseCurrentPopup();
        ImGui::EndPopup();
        to_remove.push_back(k);
        return;
    }

    // Ensure the child is armed while THIS popup is open (so it nests correctly).
    if (k + 1 < m_drawStack.size())
    {
        ActiveItem& child = m_drawStack[k + 1];
        if (!child.opened_once)
        {
            auto itc = m_popupDefs.find(child.handle);
            if (itc != m_popupDefs.end())
            {
                ImGui::OpenPopup(itc->second.label.c_str());
                child.opened_once = true;
            }
        }
    }

    // User content
    if (d.draw) d.draw();

    // Draw child while parent is still open
    if (k + 1 < m_drawStack.size())
        drawFrom(k + 1, to_remove);

    ImGui::EndPopup();

    // If closed via overlay/X this frame, remove it.
    if (!ImGui::IsPopupOpen(d.label.c_str(), ImGuiPopupFlags_AnyPopupId))
        to_remove.push_back(k);
}

void PopupStack::DrawAll()
{
    if (m_drawStack.empty())
    {
        if (m_pending_clean)
            cleanHandles();
        return;
    }

    std::vector<size_t> to_remove;
    to_remove.reserve(m_drawStack.size());

    // open all children who have been deferred open before we do drawFrom
    for (auto& item : m_drawStack)
    {
        if (item.defer_open && !item.opened_once)
        {
            auto it = m_popupDefs.find(item.handle);
            if (it != m_popupDefs.end())
            {
                ImGui::OpenPopup(it->second.label.c_str());
                item.opened_once = true;
            }
            item.defer_open = false;
        }
    }
    // Always start at root (index 0). Arming is handled inside drawFrom().
    drawFrom(0, to_remove);

    if (!to_remove.empty())
    {
        std::sort(to_remove.begin(), to_remove.end());
        to_remove.erase(std::unique(to_remove.begin(), to_remove.end()), to_remove.end());
        for (int i = int(to_remove.size()) - 1; i >= 0; --i)
        {
            const size_t idx = to_remove[size_t(i)];
            const PopupHandle h = m_drawStack[idx].handle;
            const bool reusable = m_drawStack[idx].reusable;
            m_drawStack.erase(m_drawStack.begin() + int(idx));
            if (!reusable)
            {
                m_popupDefs.erase(h);
                removeHandleNode(h);
            }
        }
    }

    if (m_pending_clean)
        cleanHandles();
}

void PopupStack::removeHandleNode(PopupHandle h)
{
	if (!h) return;
    for (auto it = m_popupHandles.begin(); it != m_popupHandles.end(); ++it)
    {
        if (it->c_str() == h)
        { 
            m_popupHandles.erase(it);
            break;
        }
    }
}

void PopupStack::cleanHandles()
{
    // Remove any handle nodes that have no def and are not active.
    for (auto it = m_popupHandles.begin(); it != m_popupHandles.end(); )
    {
        PopupHandle h = it->c_str();
        bool still_active = false;
        for (auto& a : m_drawStack)
        {
            if (a.handle == h)
            {
                still_active = true;
                break;
            }
        }
        if (!still_active && !m_popupDefs.count(h))
            it = m_popupHandles.erase(it);
        else
            ++it;
    }
    m_pending_clean = false;
}
