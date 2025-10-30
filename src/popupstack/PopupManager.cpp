#include "PopupManager.h"
#include <algorithm>
#include "tween/imguiTween.h"

static void CleanPopupAnimState(const PopupManager::PopupAnimState& st)
{
    if (st.pushed_modal_bg) ImGui::PopStyleColor();
    if (st.pushed_popup_bg) ImGui::PopStyleColor();
    if (st.pushed_alpha)    ImGui::PopStyleVar();
    if (st.pushed_window_bg)ImGui::PopStyleColor();
}

// Create a unique "Title###popup_<id>" label so visible title can rename safely.
std::string PopupManager::makeLabel(const std::string& title)
{
    ++m_token_counter;
    return title + "###popup_" + std::to_string(m_token_counter);
}

PopupHandle PopupManager::Reserve(const std::string& title,
    const PopupConfig& cfg, std::function<void()> draw,
    bool open_immediately, bool reusable)
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
        AddToDrawStack(h, true, reusable);
    }
    return h;
}

void PopupManager::ModifyReserved(PopupHandle h, const PopupConfig& cfg, std::function<void()> draw)
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

PopupHandle PopupManager::Open(const std::string& title,
    const PopupConfig& cfg,
    std::function<void()> draw)
{
    // open_immediately=true, reusable=false
    return Reserve(title, cfg, std::move(draw), /*open_immediately=*/true, /*reusable=*/false);
}

void PopupManager::AddToDrawStack(PopupHandle h, bool float_up, bool reusable)
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
    a.animating = (m_popupDefs[h].cfg.anim_cfg.mode != PopupAnimMode::NONE);
    a.anim_t = 0.0f;
    m_drawStack.push_back(a);
}

void PopupManager::Close(PopupHandle h)
{
    if (!h)
    {
        if (m_drawStack.empty())
            return;
        auto& a = m_drawStack.back();
        if (a.animating)
            return; // avoid changes states during anim
        a.close_requested = true;
        const auto* def = GetDef(a.handle);
        if (def && def->cfg.anim_cfg.mode != PopupAnimMode::NONE)
        {
            a.animating = true;
            a.anim_t = 1.0f;    
        }
        return;
    }
    for (auto& a : m_drawStack)
    {
        if (a.handle == h)
        {
            if (a.animating)
                return; // avoid changes states during anim
            a.close_requested = true;
            const auto* def = GetDef(h);
            if (def && def->cfg.anim_cfg.mode != PopupAnimMode::NONE)
            {
                a.animating = true;
                a.anim_t = 1.0f;
            }
            return;
        }
    }
    assert(false); // Handle was not found within the draw stack!
}

void PopupManager::DeleteReserved(PopupHandle h)
{
    if (!h) return;

    // If active: mark to close and make non-reusable; clean after closing
    bool found_active = false;
    for (auto& a : m_drawStack)
    {
        if (a.handle == h)
        {
            a.reusable = false;
            found_active = true;
            Close(h);
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

void PopupManager::setupNext(const PopupDef& d)
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

int PopupManager::indexOfActive(PopupHandle h) const
{
    if (!h) return -1;
    for (int i = 0; i < m_drawStack.size(); ++i)
        if (m_drawStack[i].handle == h)
            return i;
    return -1;
}

void PopupManager::DrawAll()
{
    if (m_drawStack.empty())
    {
        if (m_pending_clean)
            cleanHandles();
        return;
    }

    std::vector<size_t> to_remove;
    to_remove.reserve(m_drawStack.size());

    for (size_t i = 0; i < m_drawStack.size(); ++i)
    {
        auto& item = m_drawStack[i];
        if (!item.opened_once)
        {
            auto it = m_popupDefs.find(item.handle);
            if (it != m_popupDefs.end())
            {
                ImGui::OpenPopup(it->second.label.c_str());
                item.opened_once = true;
            }
        }
        const PopupHandle s_handle = item.handle;
        auto it = m_popupDefs.find(s_handle);
        PopupDef& d = it->second;

        setupNext(d);
        PopupAnimState st{};
        if (item.animating)
            st = updateAnim(item, d);

        const bool open = d.cfg.modal
            ? ImGui::BeginPopupModal(d.label.c_str(), nullptr, d.cfg.flags)
            : ImGui::BeginPopup(d.label.c_str(), d.cfg.flags);

        bool anim_finished = false;
        if (item.animating)
        {
            if (!item.close_requested && item.anim_t >= 1.0f - 1e-3f)
            {
                item.animating = false;
                anim_finished = true;
            }
            else if (item.close_requested && item.anim_t <= 1e-3f)
            {
                item.animating = false;
                anim_finished = true;
            }
        }

        if (!open)
        {
            bool was_open = ImGui::IsPopupOpen(d.label.c_str(), ImGuiPopupFlags_AnyPopupId);
            if (!was_open)
                to_remove.push_back(i);
            else if (item.close_requested && anim_finished) // i have no idea why this is required
            {
                ImGui::CloseCurrentPopup();
                CleanPopupAnimState(st);
                to_remove.push_back(i);
                continue;
            }
            CleanPopupAnimState(st);
            continue;
        }

        if (item.close_requested && anim_finished)
        {
            ImGui::CloseCurrentPopup();
            CleanPopupAnimState(st);
            ImGui::EndPopup();
            to_remove.push_back(i);
            continue;
        }

        // User content
        const size_t size_before = m_drawStack.size();
        if (d.draw) d.draw();

        // if this popup was marked as closed during its own draw call
        // we have to handle it here to allow for nested popups to open/close properly.
        int idx = indexOfActive(s_handle);
        if (idx >= 0 && idx < m_drawStack.size())
        {
            ActiveItem& ai = m_drawStack[i];
            if (ai.close_requested && !item.animating)
            {
                if (d.cfg.anim_cfg.mode != PopupAnimMode::NONE)
                {
                    item.animating = true;
                    item.anim_t = 1.0f;
                }
                else
                {
                    ImGui::CloseCurrentPopup();
                    CleanPopupAnimState(st);
                    ImGui::EndPopup();
                    to_remove.push_back(i);
                }
                continue;
            }
        }
        CleanPopupAnimState(st);
        ImGui::EndPopup();
        // If closed via overlay/X this frame, remove it.
        if (!item.animating && !ImGui::IsPopupOpen(d.label.c_str(), ImGuiPopupFlags_AnyPopupId))
            to_remove.push_back(i);
    }

    // remove all popups marked to be removed
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

void PopupManager::removeHandleNode(PopupHandle h)
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

void PopupManager::cleanHandles()
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

PopupManager::PopupAnimState PopupManager::updateAnim(ActiveItem& a, const PopupDef& d)
{
    PopupAnimState st;
    const PopupAnimConfig& acfg = d.cfg.anim_cfg;
    if (acfg.duration <= 0.0f)
    {
        a.anim_t = a.close_requested ? 0.0f : 1.0f;
        return st;
    }
    ImGuiTweenFlags flag = a.close_requested ? ImGuiTweenFlags_StartMax : ImGuiTweenFlags_StartMin;
    a.anim_t = ImGui::Tween<float>(d.label.c_str(), !a.close_requested,
        acfg.duration, acfg.duration, 0.0f, 1.0f, flag, acfg.easeFunc);
    a.anim_t = ImClamp(a.anim_t, 0.0f, 1.0f);
    ImVec4 popupBg = ImGui::GetStyleColorVec4(ImGuiCol_PopupBg);
    ImVec4 modalBg = ImGui::GetStyleColorVec4(ImGuiCol_ModalWindowDimBg);

    switch (acfg.mode)
    {
    case PopupAnimMode::FADE:
        // 1) overall alpha
        ImGui::PushStyleVar(ImGuiStyleVar_Alpha, a.anim_t);
        st.pushed_alpha = true;

        // 2) fade window bg
        popupBg.w *= a.anim_t;
        ImGui::PushStyleColor(ImGuiCol_PopupBg, popupBg);
        st.pushed_popup_bg = true;

        // 3) fade modal dim bg (for modal windows)
        modalBg.w *= a.anim_t;
        ImGui::PushStyleColor(ImGuiCol_ModalWindowDimBg, modalBg);
        st.pushed_modal_bg = true;
        break;
    }

    return st;
}