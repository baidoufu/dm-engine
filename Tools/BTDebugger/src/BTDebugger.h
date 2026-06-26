#pragma once
#include <windows.h>
#include <windowsx.h>
#include <commctrl.h>
#include <commdlg.h>
#include <string>
#include <vector>
#include <memory>
#include "BTNode.h"
#include "BTEngine.h"

// 控件ID
#define ID_TREEVIEW      1001
#define ID_PROPLIST      1002
#define ID_LOGEDIT       1003
#define ID_BTN_LOAD      2001
#define ID_BTN_STEP      2002
#define ID_BTN_AUTO      2003
#define ID_BTN_RESET     2004
#define ID_SPEED_SLIDER  2005
#define ID_SPEED_LABEL   2006
#define ID_STATUS_BAR    3001
#define ID_TIMER_AUTO    4001

// 窗口类
class CBTDebugger
{
public:
    CBTDebugger(HINSTANCE hInstance);
    ~CBTDebugger();

    bool Init(int nCmdShow);
    int Run();

    static LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

private:
    HINSTANCE m_hInstance;
    HWND m_hWnd;
    HWND m_hTreeView;
    HWND m_hPropList;
    HWND m_hLogEdit;
    HWND m_hStatusBar;
    HWND m_hBtnLoad;
    HWND m_hBtnStep;
    HWND m_hBtnAuto;
    HWND m_hBtnReset;
    HWND m_hSpeedSlider;
    HWND m_hSpeedLabel;

    // 分隔条
    int m_splitterPos;
    bool m_dragging;
    int m_rightSplitterPos;

    // 行为树数据
    std::shared_ptr<BTNode> m_pRoot;
    BTEngine m_engine;
    std::wstring m_currentFile;

    // 自动执行
    bool m_isAutoRunning;
    int m_autoSpeed; // ms

    // 搜索状态
    std::wstring m_selectedNodeId;

    // 窗口尺寸
    int m_width, m_height;

    // 初始化
    void CreateControls();
    void LayoutControls();
    void InitTreeViewImages();

    // 消息处理
    void OnSize(int width, int height);
    void OnCommand(WORD id);
    void OnNotify(NMHDR* pnmh);
    void OnTimer(UINT_PTR id);
    void OnPaint(HDC hdc);

    // 行为树操作
    void LoadXMLFile();
    void LoadXMLFromString(const std::string& xml);
    bool LoadBuiltInSample(const std::wstring& name);
    void PopulateTreeView();
    void PopulateTreeViewRecursive(HTREEITEM hParent, std::shared_ptr<BTNode> node);
    void UpdateNodeStates();
    void UpdateNodeStateRecursive(HTREEITEM hItem);
    HTREEITEM FindTreeItem(std::shared_ptr<BTNode> node);
    void StepExecute();
    void StartAuto();
    void StopAuto();
    void ResetExecution();
    void UpdatePropertyPanel();
    void UpdateLogPanel();
    void UpdateStatusBar();

    // 分隔条
    void OnLButtonDown(int x, int y);
    void OnMouseMove(int x, int y);
    void OnLButtonUp();

    // 工具函数
    std::string WStringToUTF8(const std::wstring& wstr);
    std::wstring UTF8ToWString(const std::string& str);
    void SetLogText(const std::wstring& text);
    void AppendLogText(const std::wstring& text);

    // 静态实例指针
    static CBTDebugger* s_pInstance;
};