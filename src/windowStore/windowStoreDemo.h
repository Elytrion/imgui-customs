#pragma once
#include "imguiWindowStore.h"
#include "demo_module.h"
#include <vector>
#include <algorithm>

// Example data model for the table
struct DemoRow
{
    int         Id;
    const char* Name;
    float       Value;
    bool        Favorite;
};

class WindowStoreDemo : public DemoModule
{
public:
    WindowStoreDemo() : DemoModule("Window Store", "Window Store Panel") { has_popout = false; }
    ~WindowStoreDemo()
    {
        // Clean up any heap objects we put into ImGuiStorage via SetPtr().
        for (void* p : owned_ptrs)
            IM_FREE(p); // matches IM_ALLOC/IM_NEW family allocations
    }

protected:
    void DrawSelectedDemo() override;

private:
    std::vector<void*> owned_ptrs;

    template <typename T>
    T* GetOrCreatePtr(ImGui::WindowStore& store, const char* tag)
    {
        if (T* p = (T*)store.GetPtr(tag))
            return p;
        T* p = IM_NEW(T)();
        store.SetPtr(tag, p);
        owned_ptrs.push_back(p);
        return p;
    }
};

inline void WindowStoreDemo::DrawSelectedDemo()
{
	ImGui::TextWrapped("This demo shows how to use WindowStore, a constructable API to access ImGui's window local state storage, to persist UI state.");
    ImGui::TextWrapped(
        "The combo below contains a table. WindowStore persists UI state even though this UI lives inside a popup "
        "(selection, filter, column toggles, and row favorites).");
    ImGui::BulletText("Click a row to select it.");
    ImGui::BulletText("Click the * column to toggle favorite.");
    ImGui::BulletText("Right-click the '(right-click here...)' text to toggle columns.");
    ImGui::BulletText("Sort by clicking headers; it persists after closing/reopening the combo.");
    ImGui::Spacing();

    ImGui::Separator();

    // Must be called inside Begin()/End() of a window.
    // Base key scopes this demo module inside the window.
    ImGui::WindowStore store = ImGui::GetWindowStore(ImGui::GetID("WindowStoreDemo"));

    // Some stable demo data
    static std::vector<DemoRow> rows = {
        { 1001, "Alpha",   1.2f,  true  },
        { 1002, "Bravo",   5.8f,  false },
        { 1003, "Charlie", 3.1f,  false },
        { 1004, "Delta",   9.4f,  true  },
        { 1005, "Echo",    2.6f,  false },
        { 1006, "Foxtrot", 7.7f,  true  },
        { 1007, "Golf",    4.0f,  false },
        { 1008, "Hotel",   6.3f,  false },
    };

	// Note: All variables are local to this function, but their values persist across frames thanks to WindowStore.
    // This can be used as a flexible alternative to stored variables, especially useful for imgui custom widgets

    // Persisted state (in ImGuiStorage via WindowStore)
    int selected_id = store.GetInt("selected_id", rows.empty() ? 0 : rows[0].Id);
    bool only_favorites = store.GetBool("only_favorites", false);

    // Store a pointer to an ImGuiTextFilter to demonstrate SetPtr/GetPtr.
    ImGuiTextFilter* filter = GetOrCreatePtr<ImGuiTextFilter>(store, "filter");

    // Manual "sort memory" showcase 
    int sort_col = store.GetInt("sort_col", 0);   // 0=Id, 1=Name, 2=Value, 3=Fav
    int sort_dir = store.GetInt("sort_dir", 1);  // +1 asc, -1 desc

    // Column visibility toggles (also persisted)
    bool col_id = store.GetBool("col_id", true);
    bool col_name = store.GetBool("col_name", true);
    bool col_value = store.GetBool("col_value", true);
    bool col_fav = store.GetBool("col_fav", true);

    // Preview label for the combo
    const char* preview = "<none>";
    for (const auto& r : rows)
        if (r.Id == selected_id) { preview = r.Name; break; }

    ImGui::TextUnformatted("Dropdown table picker (state persisted by WindowStore):");

    // A "small dropdown" combo containing the table
    ImGui::SetNextItemWidth(260.0f);
    if (ImGui::BeginCombo("Pick item", preview, ImGuiComboFlags_HeightLargest))
    {
        // Controls inside the popup
        ImGui::Checkbox("Only favorites", &only_favorites);
        store.SetBool("only_favorites", only_favorites);

        filter->Draw("Filter", 200.0f);

        // Column visibility menu
        if (ImGui::BeginPopupContextItem("##cols_menu", ImGuiPopupFlags_MouseButtonRight))
        {
            ImGui::TextUnformatted("Columns");
            ImGui::Separator();
            if (ImGui::MenuItem("Id", nullptr, col_id)) { col_id = !col_id;    store.SetBool("col_id", col_id); }
            if (ImGui::MenuItem("Name", nullptr, col_name)) { col_name = !col_name;  store.SetBool("col_name", col_name); }
            if (ImGui::MenuItem("Value", nullptr, col_value)) { col_value = !col_value; store.SetBool("col_value", col_value); }
            if (ImGui::MenuItem("Fav", nullptr, col_fav)) { col_fav = !col_fav;   store.SetBool("col_fav", col_fav); }
            ImGui::EndPopup();
        }
        ImGui::SameLine();
        ImGui::TextDisabled("(right-click here for columns)");

        // A child region so the table scrolls inside the combo
        ImGui::BeginChild("##combo_table_region", ImVec2(520.0f, 240.0f), true);

        ImGuiTableFlags flags =
            ImGuiTableFlags_Resizable |
            ImGuiTableFlags_Reorderable |
            ImGuiTableFlags_Hideable |
            ImGuiTableFlags_Sortable |
            ImGuiTableFlags_RowBg |
            ImGuiTableFlags_BordersInnerV |
            ImGuiTableFlags_ScrollY;

        if (ImGui::BeginTable("##picker_table", 4, flags))
        {
            // Apply column enablement based on persisted state
            ImGui::TableSetupColumn("Id", ImGuiTableColumnFlags_DefaultSort, 0.0f, 0);
            ImGui::TableSetupColumn("Name", ImGuiTableColumnFlags_None, 0.0f, 1);
            ImGui::TableSetupColumn("Value", ImGuiTableColumnFlags_PreferSortDescending, 0.0f, 2);
            ImGui::TableSetupColumn("Fav", ImGuiTableColumnFlags_NoSort, 0.0f, 3);

            ImGui::TableSetColumnEnabled(0, col_id);
            ImGui::TableSetColumnEnabled(1, col_name);
            ImGui::TableSetColumnEnabled(2, col_value);
            ImGui::TableSetColumnEnabled(3, col_fav);

            ImGui::TableHeadersRow();

            // Read current table sort specs and persist them to WindowStore (demo-friendly)
            if (ImGuiTableSortSpecs* specs = ImGui::TableGetSortSpecs())
            {
                if (specs->SpecsDirty && specs->SpecsCount > 0)
                {
                    const ImGuiTableColumnSortSpecs& s = specs->Specs[0];
                    sort_col = (int)s.ColumnUserID;
                    sort_dir = (s.SortDirection == ImGuiSortDirection_Ascending) ? +1 : -1;

                    store.SetInt("sort_col", sort_col);
                    store.SetInt("sort_dir", sort_dir);

                    specs->SpecsDirty = false;
                }
            }

            // Build a view of rows that pass filter/toggles
            std::vector<const DemoRow*> view;
            view.reserve(rows.size());
            for (const auto& r : rows)
            {
                if (only_favorites && !r.Favorite)
                    continue;
                if (filter && !filter->PassFilter(r.Name))
                    continue;
                view.push_back(&r);
            }

            // Apply persisted sort to the view (so it persists even after closing/reopening combo)
            auto cmp = [&](const DemoRow* a, const DemoRow* b)
                {
                    int dir = sort_dir;
                    switch (sort_col)
                    {
                    case 0: return dir > 0 ? (a->Id < b->Id) : (a->Id > b->Id);
                    case 1: return dir > 0 ? (strcmp(a->Name, b->Name) < 0) : (strcmp(a->Name, b->Name) > 0);
                    case 2: return dir > 0 ? (a->Value < b->Value) : (a->Value > b->Value);
                    case 3: return dir > 0 ? (a->Favorite < b->Favorite) : (a->Favorite > b->Favorite);
                    default: return a->Id < b->Id;
                    }
                };
            std::sort(view.begin(), view.end(), cmp);

            // Table rows
            for (const DemoRow* r : view)
            {
                ImGui::TableNextRow();
                ImGui::TableNextColumn();
                ImGui::Text("%d", r->Id);

                ImGui::TableNextColumn();
                // Make the name selectable; selecting closes the combo and persists selection
                bool selected = (r->Id == selected_id);
                if (ImGui::Selectable(r->Name, selected, ImGuiSelectableFlags_SpanAllColumns))
                {
                    selected_id = r->Id;
                    store.SetInt("selected_id", selected_id);
                    ImGui::CloseCurrentPopup();
                }

                ImGui::TableNextColumn();
                ImGui::Text("%.2f", r->Value);

                ImGui::TableNextColumn();
                ImGui::TextUnformatted(r->Favorite ? "*" : "");
            }

            ImGui::EndTable();
        }

        ImGui::EndChild();
        ImGui::EndCombo();
    }

    // Show persisted values outside the combo
    ImGui::Separator();
    ImGui::Text("Persisted:");
    ImGui::BulletText("selected_id: %d", store.GetInt("selected_id", 0));
    ImGui::BulletText("only_favorites: %s", store.GetBool("only_favorites") ? "true" : "false");
    ImGui::BulletText("sort: col=%d dir=%s", store.GetInt("sort_col", 0), store.GetInt("sort_dir", +1) > 0 ? "asc" : "desc");

    if (ImGui::Button("Reset picker state"))
    {
        store.SetInt("selected_id", rows.empty() ? 0 : rows[0].Id);
        store.SetBool("only_favorites", false);
        store.SetInt("sort_col", 0);
        store.SetInt("sort_dir", +1);
        store.SetBool("col_id", true);
        store.SetBool("col_name", true);
        store.SetBool("col_value", true);
        store.SetBool("col_fav", true);

        if (filter) filter->Clear();
    }
}