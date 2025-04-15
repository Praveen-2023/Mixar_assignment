#pragma once

// Stub header for demo purposes
#define IMGUI_CHECKVERSION()
#define IM_COL32(R,G,B,A) ((unsigned int)((A)<<24 | (B)<<16 | (G)<<8 | (R)))
#define ImGuiWindowFlags_MenuBar 1
#define ImGuiWindowFlags_NoDocking 2
#define ImGuiWindowFlags_NoTitleBar 4
#define ImGuiWindowFlags_NoCollapse 8
#define ImGuiWindowFlags_NoResize 16
#define ImGuiWindowFlags_NoMove 32
#define ImGuiWindowFlags_NoBringToFrontOnFocus 64
#define ImGuiWindowFlags_NoNavFocus 128
#define ImGuiDockNodeFlags_None 0

struct ImVec2 {
    float x, y;
    ImVec2() : x(0), y(0) {}
    ImVec2(float _x, float _y) : x(_x), y(_y) {}
};

struct ImVec4 {
    float x, y, z, w;
    ImVec4() : x(0), y(0), z(0), w(0) {}
    ImVec4(float _x, float _y, float _z, float _w) : x(_x), y(_y), z(_z), w(_w) {}
};

struct ImGuiIO {
    ImVec2 MouseDelta;
    float MouseWheel;
};

struct ImDrawList {
    void AddLine(ImVec2 p1, ImVec2 p2, unsigned int col, float thickness = 1.0f) {}
    void AddRect(ImVec2 p_min, ImVec2 p_max, unsigned int col, float rounding = 0.0f, int flags = 0, float thickness = 1.0f) {}
    void AddRectFilled(ImVec2 p_min, ImVec2 p_max, unsigned int col, float rounding = 0.0f, int flags = 0) {}
    void AddCircleFilled(ImVec2 center, float radius, unsigned int col, int num_segments = 0) {}
    void AddText(ImVec2 pos, unsigned int col, const char* text_begin, const char* text_end = nullptr) {}
    void AddBezierCurve(ImVec2 p1, ImVec2 p2, ImVec2 p3, ImVec2 p4, unsigned int col, float thickness, int num_segments = 0) {}
};

struct ImGuiViewport {
    ImVec2 Pos;
    ImVec2 Size;
    unsigned int ID;
};

typedef unsigned int ImGuiID;
typedef int ImGuiStyleVar;

namespace ImGui {
    void CreateContext();
    void DestroyContext();
    void NewFrame();
    void Render();
    void EndFrame();
    bool Begin(const char* name, bool* p_open = nullptr, int flags = 0);
    void End();
    bool BeginMenuBar();
    void EndMenuBar();
    bool BeginMenu(const char* label, bool enabled = true);
    void EndMenu();
    bool MenuItem(const char* label, const char* shortcut = nullptr, bool selected = false, bool enabled = true);
    void Separator();
    bool Button(const char* label);
    bool SliderFloat(const char* label, float* v, float v_min, float v_max, const char* format = "%.3f", float power = 1.0f);
    bool SliderInt(const char* label, int* v, int v_min, int v_max, const char* format = "%d");
    bool InputText(const char* label, char* buf, size_t buf_size, int flags = 0, void* callback = nullptr, void* user_data = nullptr);
    bool Combo(const char* label, int* current_item, const char* const items[], int items_count, int popup_max_height_in_items = -1);
    void SameLine(float offset_from_start_x = 0.0f, float spacing = -1.0f);
    void Text(const char* fmt, ...);
    void TextWrapped(const char* fmt, ...);
    ImVec2 GetMousePos();
    bool IsMouseClicked(int button, bool repeat = false);
    bool IsMouseDragging(int button, float lock_threshold = -1.0f);
    bool IsWindowHovered(int flags = 0);
    ImGuiIO& GetIO();
    ImVec2 GetContentRegionAvail();
    ImVec2 GetCursorScreenPos();
    ImDrawList* GetWindowDrawList();
    ImGuiViewport* GetMainViewport();
    void SetNextWindowPos(ImVec2 pos, int cond = 0, ImVec2 pivot = ImVec2(0, 0));
    void SetNextWindowSize(ImVec2 size, int cond = 0);
    void SetNextWindowViewport(unsigned int viewport_id);
    void PushStyleVar(ImGuiStyleVar idx, float val);
    void PushStyleVar(ImGuiStyleVar idx, ImVec2 val);
    void PopStyleVar(int count = 1);
    bool BeginPopup(const char* str_id, int flags = 0);
    void EndPopup();
    void OpenPopup(const char* str_id, int popup_flags = 0);
    ImGuiID GetID(const char* str_id);
    void DockSpace(ImGuiID id, ImVec2 size = ImVec2(0, 0), int flags = 0);
    void StyleColorsDark();
}
