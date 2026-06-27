#include "BTDebugger.h"
#include <fstream>
#include <sstream>
#include <codecvt>
#include <map>
#include <algorithm>
#include <CommCtrl.h>

#pragma comment(lib, "comctl32.lib")
#pragma comment(linker, "\"/manifestdependency:type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

// ============================================================================
// 自制对话框窗口过程
// ============================================================================
static LRESULT CALLBACK DlgFrameWndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg)
    {
    case WM_COMMAND:
        // 将 WM_COMMAND 转发为自定义消息，让模态循环能捕获
        PostMessageW(hWnd, WM_USER + 100, wParam, lParam);
        return 0;
    case WM_CLOSE:
        PostMessageW(hWnd, WM_USER + 101, 0, 0);
        return 0;
    }
    return DefWindowProcW(hWnd, msg, wParam, lParam);
}

CBTDebugger* CBTDebugger::s_pInstance = nullptr;

// 内置示例数据
static const std::map<std::wstring, std::string> g_builtInSamples = {
    { L"战士战斗行为树", R"BT(<?xml version="1.0" encoding="GBK"?>
<BehaviorTree name="战士战斗行为树 - Warrior v2.0">
    <Selector name="战士主决策">
        <Sequence name="安全区恢复行为">
            <ConditionInSafeArea name="检测是否在安全区" />
            <Parallel name="安全区内多任务处理">
                <Sequence name="安全区回血">
                    <ConditionLowHP name="HP不足70%" percent="70" />
                    <Probability name="吃药犹豫(30%)" chance="30">
                        <ActionUsePotion name="使用HP药水" hpType="1" />
                    </Probability>
                </Sequence>
                <Sequence name="安全区回蓝">
                    <ConditionLowMP name="MP不足40%" percent="40" />
                    <ActionUsePotion name="使用MP药水" hpType="0" />
                </Sequence>
                <Probability name="巡逻概率(40%)" chance="40">
                    <ActionPatrol name="安全区内巡逻" />
                </Probability>
                <Sequence name="安全区社交">
                    <Probability name="聊天概率(60%)" chance="60">
                        <ActionChat name="随机聊天" />
                    </Probability>
                </Sequence>
            </Parallel>
        </Sequence>
        <Sequence name="背包已满处理">
            <ConditionBagFull name="背包是否已满" />
            <ActionUseItem name="使用回城卷" itemName="回城卷" />
        </Sequence>
        <Sequence name="紧急逃生策略">
            <ConditionLowHP name="HP低于20%濒死" percent="20" />
            <Parallel name="逃生多动作">
                <ActionFlee name="向安全方向逃跑" />
                <Probability name="传送概率(20%)" chance="20">
                    <ActionUseItem name="随机传送卷逃脱" itemName="随机传送卷" />
                </Probability>
            </Parallel>
        </Sequence>
        <Sequence name="战斗策略">
            <ConditionHasTarget name="检查是否有目标" />
            <ActionChangeAttackMode name="切换全体攻击模式" attackMode="1" />
            <Parallel name="战斗并行行为">
                <Sequence name="战斗主循环">
                    <ActionMoveToTarget name="移向目标" />
                    <Selector name="攻击方式选择">
                        <Sequence name="技能攻击尝试">
                            <Probability name="技能犹豫(10%)" chance="10">
                                <ActionUseSkill name="使用战士技能攻击" magicId="0" />
                            </Probability>
                        </Sequence>
                        <ActionAttack name="普通攻击" />
                    </Selector>
                </Sequence>
                <Sequence name="战斗补给">
                    <Sequence name="战斗喝红">
                        <ConditionLowHP name="HP低于50%" percent="50" />
                        <Probability name="立刻喝药概率(80%)" chance="80">
                            <ActionUsePotion name="战斗中喝HP药水" hpType="1" />
                        </Probability>
                    </Sequence>
                    <Sequence name="战斗喝蓝">
                        <ConditionLowMP name="MP低于30%" percent="30" />
                        <ActionUsePotion name="战斗中喝MP药水" hpType="0" />
                    </Sequence>
                </Sequence>
            </Parallel>
        </Sequence>
        <Sequence name="自由漫游策略">
            <ActionPickupItem name="拾取地上物品" />
            <Random name="闲逛随机行为">
                <ActionPatrol name="随机巡逻走动" />
                <ActionRest name="原地休息" duration="5000" />
                <ActionChat name="随机聊天" />
            </Random>
        </Sequence>
    </Selector>
</BehaviorTree>)BT" },

    { L"法师战斗行为树", R"BT(<?xml version="1.0" encoding="GBK"?>
<BehaviorTree name="法师战斗行为树 - Mage v2.0">
    <Selector name="法师主决策">
        <Sequence name="安全区行为">
            <ConditionInSafeArea name="在安全区" />
            <Parallel name="安全区并行行为">
                <Sequence name="安全区回血">
                    <ConditionLowHP name="HP不足60%" percent="60" />
                    <ActionUsePotion name="喝HP药水" hpType="1" />
                </Sequence>
                <Sequence name="安全区回蓝">
                    <ConditionLowMP name="MP不足50%" percent="50" />
                    <ActionUsePotion name="喝MP药水" hpType="0" />
                </Sequence>
                <Probability name="安全区巡逻(30%)" chance="30">
                    <ActionPatrol name="安全区巡逻" />
                </Probability>
                <Probability name="安全区聊天(50%)" chance="50">
                    <ActionChat name="安全区聊天" />
                </Probability>
            </Parallel>
        </Sequence>
        <Sequence name="背包满处理">
            <ConditionBagFull name="背包满" />
            <ActionUseItem name="使用回城卷" itemName="回城卷" />
        </Sequence>
        <Sequence name="法师紧急逃生">
            <ConditionLowHP name="HP不足25%濒死" percent="25" />
            <Parallel name="逃生多动作">
                <Probability name="随机传送(40%)" chance="40">
                    <ActionUseItem name="随机传送逃跑" itemName="随机传送卷" />
                </Probability>
                <ActionFlee name="向安全方向逃跑" />
            </Parallel>
        </Sequence>
        <Sequence name="战斗策略">
            <ConditionHasTarget name="有目标" />
            <Sequence name="法师战斗流程">
                <ActionChangeAttackMode name="切换全体攻击" attackMode="1" />
                <Selector name="法师攻击方式选择">
                    <Sequence name="远程技能攻击">
                        <ActionUseSkill name="远程技能攻击" magicId="0" />
                        <Probability name="攻击后停顿(20%)" chance="20">
                            <ActionRest name="短暂停顿" duration="800" />
                        </Probability>
                    </Sequence>
                    <Sequence name="普通攻击">
                        <ActionAttack name="普通攻击" />
                    </Sequence>
                </Selector>
                <ActionMoveToTarget name="调整与目标距离" />
                <Parallel name="战斗补给并行">
                    <Sequence name="战斗喝红">
                        <ConditionLowHP name="HP不足45%" percent="45" />
                        <ActionUsePotion name="喝HP药水" hpType="1" />
                    </Sequence>
                    <Sequence name="战斗喝蓝">
                        <ConditionLowMP name="MP不足40%" percent="40" />
                        <ActionUsePotion name="喝MP药水" hpType="0" />
                    </Sequence>
                </Parallel>
            </Sequence>
        </Sequence>
        <Sequence name="自由漫游">
            <ActionPickupItem name="拾取物品" />
            <Random name="漫游随机行为">
                <ActionPatrol name="巡逻" />
                <ActionRest name="休息" duration="3000" />
                <ActionChat name="聊天" />
                <Sequence name="发呆">
                    <Probability name="发呆概率" chance="10" />
                </Sequence>
            </Random>
        </Sequence>
    </Selector>
</BehaviorTree>)BT" },

    { L"道士战斗行为树", R"BT(<?xml version="1.0" encoding="GBK"?>
<BehaviorTree name="道士战斗行为树 - Taoist v2.0">
    <Selector name="道士主决策">
        <Sequence name="安全区自动恢复">
            <ConditionInSafeArea name="在安全区" />
            <Parallel name="安全区行为">
                <Sequence name="回血">
                    <ConditionLowHP name="HP不足65%" percent="65" />
                    <ActionUsePotion name="喝HP" hpType="1" />
                </Sequence>
                <Sequence name="回蓝">
                    <ConditionLowMP name="MP不足35%" percent="35" />
                    <ActionUsePotion name="喝MP" hpType="0" />
                </Sequence>
                <Probability name="巡逻(25%)" chance="25">
                    <ActionPatrol name="安全区巡逻" />
                </Probability>
                <Probability name="聊天(65%)" chance="65">
                    <ActionChat name="安全区聊天" />
                </Probability>
            </Parallel>
        </Sequence>
        <Sequence name="背包已满">
            <ConditionBagFull name="包满" />
            <ActionUseItem name="回城" itemName="回城卷" />
        </Sequence>
        <Sequence name="紧急自救">
            <ConditionLowHP name="HP不足30%" percent="30" />
            <ActionFlee name="逃跑" />
        </Sequence>
        <Sequence name="战斗主流程">
            <ConditionHasTarget name="有目标" />
            <ActionChangeAttackMode name="切善恶模式" attackMode="0" />
            <Sequence name="战前MP检查">
                <ConditionLowMP name="MP不足50%" percent="50" />
                <ActionUsePotion name="战前喝蓝" hpType="0" />
            </Sequence>
            <Parallel name="战斗并行策略">
                <Sequence name="自我治疗">
                    <ConditionLowHP name="HP不足50%加血" percent="50" />
                    <ActionUseSkill name="使用治愈术" magicId="0" />
                </Sequence>
                <Sequence name="攻击序列">
                    <ActionUseSkill name="道术攻击" magicId="0" />
                    <Probability name="追加攻击(20%)" chance="20">
                        <ActionUseSkill name="追加攻击" magicId="0" />
                    </Probability>
                    <ActionMoveToTarget name="逼近目标" />
                </Sequence>
                <Sequence name="战斗补给">
                    <Selector name="补给选择">
                        <Sequence name="喝HP">
                            <ConditionLowHP name="HP不足40%" percent="40" />
                            <ActionUsePotion name="喝HP" hpType="1" />
                        </Sequence>
                        <Sequence name="喝MP">
                            <ConditionLowMP name="MP不足25%" percent="25" />
                            <ActionUsePotion name="喝MP" hpType="0" />
                        </Sequence>
                    </Selector>
                </Sequence>
            </Parallel>
        </Sequence>
        <Sequence name="自由行为">
            <ActionPickupItem name="拾取物品" />
            <Random name="随机行为">
                <ActionPatrol name="巡逻走动" />
                <ActionRest name="原地休息" duration="5000" />
                <ActionChat name="聊天" />
                <Sequence name="反转示例">
                    <Inverter name="反转安全区判断">
                        <ConditionInSafeArea name="不在安全区" />
                    </Inverter>
                    <ActionPatrol name="反向巡逻" />
                </Sequence>
            </Random>
        </Sequence>
    </Selector>
</BehaviorTree>)BT" }
};

CBTDebugger::CBTDebugger(HINSTANCE hInstance)
    : m_hInstance(hInstance), m_hWnd(nullptr)
    , m_hTreeView(nullptr), m_hPropList(nullptr), m_hLogEdit(nullptr)
    , m_hStatusBar(nullptr), m_hBtnLoad(nullptr), m_hBtnStep(nullptr)
    , m_hBtnAuto(nullptr), m_hBtnReset(nullptr), m_hBtnSave(nullptr)
    , m_hSpeedSlider(nullptr), m_hSpeedLabel(nullptr)
    , m_splitterPos(480), m_dragging(false), m_rightSplitterPos(300)
    , m_pRoot(nullptr), m_isAutoRunning(false), m_autoSpeed(500)
    , m_width(1200), m_height(750)
{
    s_pInstance = this;
}

CBTDebugger::~CBTDebugger()
{
    s_pInstance = nullptr;
}

bool CBTDebugger::Init(int nCmdShow)
{
    // 注册窗口类
    WNDCLASSEXW wcex = {};
    wcex.cbSize = sizeof(WNDCLASSEXW);
    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = WndProc;
    wcex.hInstance = m_hInstance;
    wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground = CreateSolidBrush(RGB(10, 10, 20));
    wcex.lpszClassName = L"BTDebuggerWindow";
    wcex.hIcon = LoadIcon(nullptr, IDI_APPLICATION);

    RegisterClassExW(&wcex);

    // 注册自制对话框窗口类
    WNDCLASSEXW dlgClass = {};
    dlgClass.cbSize = sizeof(WNDCLASSEXW);
    dlgClass.lpfnWndProc = DlgFrameWndProc;
    dlgClass.hInstance = m_hInstance;
    dlgClass.hCursor = LoadCursor(nullptr, IDC_ARROW);
    dlgClass.hbrBackground = (HBRUSH)(COLOR_BTNFACE + 1);
    dlgClass.lpszClassName = L"BTDlgFrame";
    RegisterClassExW(&dlgClass);

    // 创建窗口
    m_hWnd = CreateWindowExW(
        WS_EX_APPWINDOW,
        L"BTDebuggerWindow",
        L"机器人行为树调试器 - 达摩引擎",
        WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN,
        CW_USEDEFAULT, CW_USEDEFAULT,
        m_width, m_height,
        nullptr, nullptr, m_hInstance, nullptr
    );

    if (!m_hWnd) return false;

    CreateControls();
    LayoutControls();

    ShowWindow(m_hWnd, nCmdShow);
    UpdateWindow(m_hWnd);

    return true;
}

void CBTDebugger::CreateControls()
{
    // 工具栏
    m_hBtnLoad = CreateWindowW(L"BUTTON", L"加载XML文件",
        WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
        8, 8, 120, 28, m_hWnd, (HMENU)ID_BTN_LOAD, m_hInstance, nullptr);

    m_hBtnSave = CreateWindowW(L"BUTTON", L"? 保存XML",
        WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
        136, 8, 100, 28, m_hWnd, (HMENU)ID_BTN_SAVE, m_hInstance, nullptr);

    m_hBtnStep = CreateWindowW(L"BUTTON", L"?| 单步执行",
        WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
        244, 8, 100, 28, m_hWnd, (HMENU)ID_BTN_STEP, m_hInstance, nullptr);

    m_hBtnAuto = CreateWindowW(L"BUTTON", L"? 自动播放",
        WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
        352, 8, 100, 28, m_hWnd, (HMENU)ID_BTN_AUTO, m_hInstance, nullptr);

    m_hBtnReset = CreateWindowW(L"BUTTON", L"? 重置",
        WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
        460, 8, 80, 28, m_hWnd, (HMENU)ID_BTN_RESET, m_hInstance, nullptr);

    m_hSpeedLabel = CreateWindowW(L"STATIC", L"速度: 500ms",
        WS_CHILD | WS_VISIBLE | SS_LEFT,
        548, 12, 100, 20, m_hWnd, (HMENU)ID_SPEED_LABEL, m_hInstance, nullptr);

    m_hSpeedSlider = CreateWindowW(TRACKBAR_CLASSW, L"",
        WS_CHILD | WS_VISIBLE | TBS_HORZ | TBS_AUTOTICKS,
        648, 8, 130, 28, m_hWnd, (HMENU)ID_SPEED_SLIDER, m_hInstance, nullptr);
    SendMessage(m_hSpeedSlider, TBM_SETRANGE, TRUE, MAKELONG(100, 2000));
    SendMessage(m_hSpeedSlider, TBM_SETPOS, TRUE, 500);
    SendMessage(m_hSpeedSlider, TBM_SETTICFREQ, 200, 0);

    // 树形视图
    m_hTreeView = CreateWindowExW(WS_EX_CLIENTEDGE,
        WC_TREEVIEWW, L"",
        WS_CHILD | WS_VISIBLE | TVS_HASLINES | TVS_LINESATROOT | TVS_HASBUTTONS | TVS_SHOWSELALWAYS | TVS_EDITLABELS,
        0, 44, m_splitterPos, 500,
        m_hWnd, (HMENU)ID_TREEVIEW, m_hInstance, nullptr);

    // 属性列表
    m_hPropList = CreateWindowExW(WS_EX_CLIENTEDGE,
        WC_LISTVIEWW, L"",
        WS_CHILD | WS_VISIBLE | LVS_REPORT | LVS_SINGLESEL | LVS_NOSORTHEADER,
        m_splitterPos + 4, 44, 300, 300,
        m_hWnd, (HMENU)ID_PROPLIST, m_hInstance, nullptr);

    // 日志编辑框
    m_hLogEdit = CreateWindowExW(WS_EX_CLIENTEDGE,
        L"EDIT", L"",
        WS_CHILD | WS_VISIBLE | ES_MULTILINE | ES_AUTOVSCROLL | ES_READONLY | WS_VSCROLL,
        m_splitterPos + 4, 348, 300, 200,
        m_hWnd, (HMENU)ID_LOGEDIT, m_hInstance, nullptr);

    // 状态栏
    m_hStatusBar = CreateWindowW(STATUSCLASSNAMEW, L"",
        WS_CHILD | WS_VISIBLE | SBARS_SIZEGRIP,
        0, 0, 0, 0, m_hWnd, (HMENU)ID_STATUS_BAR, m_hInstance, nullptr);

    // 设置字体
    HFONT hFont = CreateFontW(16, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
        DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
        DEFAULT_QUALITY, FIXED_PITCH | FF_MODERN, L"Consolas");
    SendMessage(m_hTreeView, WM_SETFONT, (WPARAM)hFont, TRUE);
    SendMessage(m_hPropList, WM_SETFONT, (WPARAM)hFont, TRUE);
    SendMessage(m_hLogEdit, WM_SETFONT, (WPARAM)hFont, TRUE);

    // 初始化 ListView 列
    LVCOLUMNW lvc = {};
    lvc.mask = LVCF_TEXT | LVCF_WIDTH | LVCF_FMT;
    lvc.fmt = LVCFMT_LEFT;
    lvc.cx = 120;
    lvc.pszText = (LPWSTR)L"属性";
    ListView_InsertColumn(m_hPropList, 0, &lvc);
    lvc.cx = 200;
    lvc.pszText = (LPWSTR)L"值";
    ListView_InsertColumn(m_hPropList, 1, &lvc);
    ListView_SetExtendedListViewStyle(m_hPropList, LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);
}

void CBTDebugger::LayoutControls()
{
    // 工具栏高度
    int toolbarH = 44;
    int statusH = 22;

    SetWindowPos(m_hTreeView, nullptr, 0, toolbarH,
        m_splitterPos, m_height - toolbarH - statusH, SWP_NOZORDER);

    int rightX = m_splitterPos + 4;
    int rightW = m_width - rightX;
    int propH = (m_height - toolbarH - statusH) * 3 / 5;

    SetWindowPos(m_hPropList, nullptr, rightX, toolbarH,
        rightW, propH, SWP_NOZORDER);

    SetWindowPos(m_hLogEdit, nullptr, rightX, toolbarH + propH + 4,
        rightW, m_height - toolbarH - propH - 4 - statusH, SWP_NOZORDER);

    SetWindowPos(m_hStatusBar, nullptr, 0, 0, 0, 0, SWP_NOZORDER);
    SendMessage(m_hStatusBar, WM_SIZE, 0, 0);
}

void CBTDebugger::OnSize(int width, int height)
{
    m_width = width;
    m_height = height;
    LayoutControls();
}

void CBTDebugger::OnCommand(WORD id)
{
    switch (id)
    {
    case ID_BTN_LOAD:
        LoadXMLFile();
        break;
    case ID_BTN_SAVE:
        SaveXMLFile();
        break;
    case ID_BTN_STEP:
        StepExecute();
        break;
    case ID_BTN_AUTO:
        if (m_isAutoRunning) StopAuto();
        else StartAuto();
        break;
    case ID_BTN_RESET:
        ResetExecution();
        break;
    }
}

LRESULT CBTDebugger::OnNotify(NMHDR* pnmh)
{
    if (pnmh->idFrom == ID_TREEVIEW)
    {
        if (pnmh->code == TVN_SELCHANGEDW)
        {
            NMTREEVIEWW* pnmtv = (NMTREEVIEWW*)pnmh;
            TVITEMW item = pnmtv->itemNew;
            if (item.lParam)
            {
                BTNode* pNode = (BTNode*)item.lParam;
                m_selectedNodeId = pNode->id;
                UpdatePropertyPanel();
            }
        }
        else if (pnmh->code == TVN_ENDLABELEDITW)
        {
            NMTVDISPINFOW* pnmtv = (NMTVDISPINFOW*)pnmh;
            if (pnmtv->item.pszText)
            {
                BTNode* pNode = (BTNode*)pnmtv->item.lParam;
                if (pNode)
                {
                    pNode->name = pnmtv->item.pszText;
                    UpdatePropertyPanel();
                    AppendLogText(L"[编辑] 节点重命名为 '" + pNode->name + L"'\r\n");
                    return TRUE; // 接受重命名
                }
            }
            return FALSE; // 拒绝重命名
        }
        else if (pnmh->code == NM_RCLICK)
        {
            POINT pt;
            GetCursorPos(&pt);
            OnTreeContextMenu(pt);
        }
    }
    else if (pnmh->idFrom == ID_PROPLIST && pnmh->code == NM_DBLCLK)
    {
        OnPropListDoubleClick();
    }
    return 0;
}

void CBTDebugger::OnTimer(UINT_PTR id)
{
    if (id == ID_TIMER_AUTO && m_isAutoRunning)
    {
        StepExecute();
    }
}

void CBTDebugger::OnPaint(HDC hdc)
{
    // 绘制分隔条
    RECT rc;
    GetClientRect(m_hWnd, &rc);
    RECT splitter = { m_splitterPos, 44, m_splitterPos + 4, rc.bottom - 22 };
    FillRect(hdc, &splitter, (HBRUSH)GetStockObject(DKGRAY_BRUSH));
}

void CBTDebugger::OnLButtonDown(int x, int y)
{
    if (x >= m_splitterPos && x <= m_splitterPos + 4 && y > 44)
    {
        m_dragging = true;
        SetCapture(m_hWnd);
    }
}

void CBTDebugger::OnMouseMove(int x, int y)
{
    if (m_dragging)
    {
        if (x > 200 && x < m_width - 200)
        {
            m_splitterPos = x;
            LayoutControls();
            InvalidateRect(m_hWnd, nullptr, FALSE);
        }
    }
}

void CBTDebugger::OnLButtonUp()
{
    if (m_dragging)
    {
        m_dragging = false;
        ReleaseCapture();
    }
}

void CBTDebugger::LoadXMLFile()
{
    OPENFILENAMEW ofn = {};
    wchar_t szFile[260] = {};
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = m_hWnd;
    ofn.lpstrFile = szFile;
    ofn.nMaxFile = 260;
    ofn.lpstrFilter = L"XML 行为树文件\0*.xml\0所有文件\0*.*\0";
    ofn.nFilterIndex = 1;
    ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

    if (GetOpenFileNameW(&ofn))
    {
        m_currentFile = szFile;
        auto root = XMLParser::ParseFile(m_currentFile);
        if (root)
        {
            m_pRoot = root;
            m_engine.Reset();
            PopulateTreeView();
            UpdateStatusBar();
        }
        else
        {
            MessageBoxW(m_hWnd, L"XML 文件解析失败！请检查文件格式。", L"错误", MB_ICONERROR);
        }
    }
}

void CBTDebugger::LoadXMLFromString(const std::string& xml)
{
    auto root = XMLParser::ParseString(xml);
    if (root)
    {
        m_pRoot = root;
        m_engine.Reset();
        m_currentFile = L"(内置示例)";
        PopulateTreeView();
        UpdateStatusBar();
    }
    else
    {
        MessageBoxW(m_hWnd, L"XML 解析失败！", L"错误", MB_ICONERROR);
    }
}

void CBTDebugger::PopulateTreeView()
{
    TreeView_DeleteAllItems(m_hTreeView);
    if (!m_pRoot) return;
    PopulateTreeViewRecursive(TVI_ROOT, m_pRoot);
}

void CBTDebugger::PopulateTreeViewRecursive(HTREEITEM hParent, std::shared_ptr<BTNode> node)
{
    TVINSERTSTRUCTW tvis = {};
    tvis.hParent = hParent;
    tvis.hInsertAfter = TVI_LAST;
    tvis.item.mask = TVIF_TEXT | TVIF_PARAM;
    tvis.item.pszText = (LPWSTR)node->name.c_str();
    tvis.item.lParam = (LPARAM)node.get();

    HTREEITEM hItem = TreeView_InsertItem(m_hTreeView, &tvis);

    for (auto& child : node->children)
        PopulateTreeViewRecursive(hItem, child);
}

void CBTDebugger::StepExecute()
{
    if (!m_pRoot) return;

    // 重置引擎状态
    m_engine.Reset();
    m_engine.ResetNodeStates(m_pRoot);
    m_engine.ExecuteFull(m_pRoot);

    UpdateNodeStates();
    UpdateLogPanel();
    UpdateStatusBar();

    // 重新绘制树视图
    InvalidateRect(m_hTreeView, nullptr, TRUE);
}

void CBTDebugger::UpdateNodeStates()
{
    if (!m_pRoot) return;
    HTREEITEM hRoot = TreeView_GetRoot(m_hTreeView);
    if (hRoot)
        UpdateNodeStateRecursive(hRoot);
}

void CBTDebugger::UpdateNodeStateRecursive(HTREEITEM hItem)
{
    if (!hItem) return;

    TVITEMW item = {};
    item.hItem = hItem;
    item.mask = TVIF_PARAM;
    TreeView_GetItem(m_hTreeView, &item);
    BTNode* pNode = (BTNode*)item.lParam;

    if (pNode)
    {
        std::wstring text = pNode->name;
        if (pNode->lastResult != BTResult::IDLE)
        {
            text += L" [";
            text += GetResultName(pNode->lastResult);
            text += L"]";
        }
        item.mask = TVIF_TEXT;
        item.pszText = (LPWSTR)text.c_str();
        TreeView_SetItem(m_hTreeView, &item);
    }

    // 递归处理所有子节点
    HTREEITEM hChild = TreeView_GetChild(m_hTreeView, hItem);
    while (hChild)
    {
        UpdateNodeStateRecursive(hChild);
        hChild = TreeView_GetNextSibling(m_hTreeView, hChild);
    }
}

void CBTDebugger::StartAuto()
{
    if (!m_pRoot) return;
    m_isAutoRunning = true;
    SetWindowTextW(m_hBtnAuto, L"? 暂停");
    SetTimer(m_hWnd, ID_TIMER_AUTO, m_autoSpeed, nullptr);
    StepExecute();
}

void CBTDebugger::StopAuto()
{
    m_isAutoRunning = false;
    KillTimer(m_hWnd, ID_TIMER_AUTO);
    SetWindowTextW(m_hBtnAuto, L"? 自动播放");
}

void CBTDebugger::ResetExecution()
{
    StopAuto();
    m_engine.Reset();
    if (m_pRoot) m_engine.ResetNodeStates(m_pRoot);
    PopulateTreeView();
    ListView_DeleteAllItems(m_hPropList);
    SetLogText(L"");
    UpdateStatusBar();
}

// ============================================================================
// 保存 XML
// ============================================================================
void CBTDebugger::SaveXMLFile()
{
    if (!m_pRoot) return;

    std::wstring savePath;
    if (m_currentFile.empty() || m_currentFile.find(L"(内置") == 0)
    {
        OPENFILENAMEW ofn = {};
        wchar_t szFile[260] = {};
        ofn.lStructSize = sizeof(ofn);
        ofn.hwndOwner = m_hWnd;
        ofn.lpstrFile = szFile;
        ofn.nMaxFile = 260;
        ofn.lpstrFilter = L"XML 行为树文件\0*.xml\0所有文件\0*.*\0";
        ofn.nFilterIndex = 1;
        ofn.Flags = OFN_PATHMUSTEXIST | OFN_OVERWRITEPROMPT;
        ofn.lpstrDefExt = L"xml";

        if (!GetSaveFileNameW(&ofn)) return;
        savePath = szFile;
        m_currentFile = savePath;
    }
    else
    {
        savePath = m_currentFile;
    }

    std::wstring treeName = m_pRoot->name;
    std::string xml = XMLParser::SerializeTree(m_pRoot, treeName);
    if (xml.empty())
    {
        MessageBoxW(m_hWnd, L"序列化失败！", L"错误", MB_ICONERROR);
        return;
    }

    std::ofstream file(savePath, std::ios::binary | std::ios::trunc);
    if (!file.is_open())
    {
        MessageBoxW(m_hWnd, L"无法写入文件！", L"错误", MB_ICONERROR);
        return;
    }
    file.write(xml.c_str(), xml.size());
    file.close();

    wchar_t buf[300];
    wsprintfW(buf, L"已保存到: %s", savePath.c_str());
    SetLogText(buf);
    UpdateStatusBar();
}

// ============================================================================
// 节点编辑功能
// ============================================================================
std::shared_ptr<BTNode> CBTDebugger::GetSelectedNode()
{
    if (m_selectedNodeId.empty() || !m_pRoot) return nullptr;
    return FindNodeById(m_pRoot, m_selectedNodeId);
}

void CBTDebugger::OnTreeContextMenu(POINT pt)
{
    if (!m_pRoot) return;

    // 获取右键点击位置的树节点
    TVHITTESTINFO ht = {};
    ht.pt = pt;
    ScreenToClient(m_hTreeView, &ht.pt);
    TreeView_HitTest(m_hTreeView, &ht);

    if (ht.flags & TVHT_ONITEM)
    {
        // 先选中该节点
        TreeView_SelectItem(m_hTreeView, ht.hItem);

        // 更新选中状态
        TVITEMW item = {};
        item.hItem = ht.hItem;
        item.mask = TVIF_PARAM;
        TreeView_GetItem(m_hTreeView, &item);
        if (item.lParam)
        {
            m_selectedNodeId = ((BTNode*)item.lParam)->id;
            UpdatePropertyPanel();
        }
    }

    // 创建右键菜单
    HMENU hMenu = CreatePopupMenu();
    AppendMenuW(hMenu, MF_STRING, IDM_RENAME_NODE, L"重命名");
    AppendMenuW(hMenu, MF_STRING, IDM_ADD_CHILD,   L"添加子节点");
    AppendMenuW(hMenu, MF_SEPARATOR, 0, nullptr);
    AppendMenuW(hMenu, MF_STRING, IDM_DELETE_NODE, L"删除节点");
    AppendMenuW(hMenu, MF_SEPARATOR, 0, nullptr);
    AppendMenuW(hMenu, MF_STRING, IDM_MOVE_UP,     L"上移");
    AppendMenuW(hMenu, MF_STRING, IDM_MOVE_DOWN,   L"下移");
    AppendMenuW(hMenu, MF_SEPARATOR, 0, nullptr);
    AppendMenuW(hMenu, MF_STRING, IDM_EDIT_PROPS,  L"编辑属性...");
    AppendMenuW(hMenu, MF_STRING, IDM_CHANGE_TYPE, L"更改类型...");

    // 如果没有选中节点，禁用编辑菜单
    if (m_selectedNodeId.empty())
    {
        EnableMenuItem(hMenu, IDM_RENAME_NODE, MF_GRAYED);
        EnableMenuItem(hMenu, IDM_ADD_CHILD,   MF_GRAYED);
        EnableMenuItem(hMenu, IDM_DELETE_NODE, MF_GRAYED);
        EnableMenuItem(hMenu, IDM_MOVE_UP,     MF_GRAYED);
        EnableMenuItem(hMenu, IDM_MOVE_DOWN,   MF_GRAYED);
    }

    TrackPopupMenu(hMenu, TPM_LEFTALIGN | TPM_RIGHTBUTTON, pt.x, pt.y, 0, m_hWnd, nullptr);
    DestroyMenu(hMenu);
}

void CBTDebugger::RenameSelectedNode()
{
    auto node = GetSelectedNode();
    if (!node) return;

    // 使用 TreeView 内置的编辑标签功能
    HTREEITEM hItem = TreeView_GetSelection(m_hTreeView);
    if (hItem)
        TreeView_EditLabel(m_hTreeView, hItem);
}

void CBTDebugger::AddChildToSelectedNode()
{
    auto parent = GetSelectedNode();
    if (!parent) return;

    HWND hDlg = CreateWindowExW(WS_EX_DLGMODALFRAME, L"BTDlgFrame", L"添加子节点",
        WS_POPUP | WS_CAPTION | WS_SYSMENU | WS_VISIBLE,
        0, 0, 380, 180, m_hWnd, nullptr, m_hInstance, nullptr);
    if (!hDlg) return;

    int xM = 12, y = 10, w = 356, rowH = 26;
    CreateWindowW(L"STATIC", L"节点类型:", WS_CHILD | WS_VISIBLE,
        xM, y, 75, 20, hDlg, nullptr, nullptr, nullptr);
    HWND hType = CreateWindowW(L"COMBOBOX", L"",
        WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST | WS_VSCROLL,
        xM + 80, y, w - xM - 80, 200, hDlg, (HMENU)100, nullptr, nullptr);

    const wchar_t* typeNames[] = {
        L"Sequence (序列)", L"Selector (选择)", L"Parallel (并行)", L"Random (随机)",
        L"Probability (概率)", L"MemSequence (记忆序列)", L"MemSelector (记忆选择)",
        L"Inverter (反转)", L"DecoratorRepeat (重复)", L"Succeeder (强制成功)", L"Failer (强制失败)",
        L"ConditionLowHP (低血)", L"ConditionLowMP (低蓝)", L"ConditionHasTarget (有目标)",
        L"ConditionInSafeArea (安全区)", L"ConditionBagFull (背包满)",
        L"ActionAttack (攻击)", L"ActionMoveToTarget (移动到目标)", L"ActionPatrol (巡逻)",
        L"ActionUsePotion (喝药)", L"ActionUseSkill (技能)", L"ActionFlee (逃跑)",
        L"ActionChat (聊天)", L"ActionRest (休息)", L"ActionPickupItem (拾取)", L"ActionUseItem (使用道具)"
    };
    for (auto& tn : typeNames)
        SendMessageW(hType, CB_ADDSTRING, 0, (LPARAM)tn);
    SendMessageW(hType, CB_SETCURSEL, 0, 0);

    y += rowH + 4;
    CreateWindowW(L"STATIC", L"节点名称:", WS_CHILD | WS_VISIBLE,
        xM, y, 75, 20, hDlg, nullptr, nullptr, nullptr);
    HWND hName = CreateWindowW(L"EDIT", L"新节点",
        WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL,
        xM + 80, y, w - xM - 80, 22, hDlg, nullptr, nullptr, nullptr);

    y += rowH + 12;
    CreateWindowW(L"BUTTON", L"确定", WS_CHILD | WS_VISIBLE | BS_DEFPUSHBUTTON,
        200, y, 80, 26, hDlg, (HMENU)IDOK, nullptr, nullptr);
    CreateWindowW(L"BUTTON", L"取消", WS_CHILD | WS_VISIBLE,
        285, y, 80, 26, hDlg, (HMENU)IDCANCEL, nullptr, nullptr);

    RECT wr;
    GetWindowRect(m_hWnd, &wr);
    SetWindowPos(hDlg, nullptr, wr.left + (wr.right - wr.left - 380) / 2,
        wr.top + (wr.bottom - wr.top - 180) / 2, 0, 0, SWP_NOSIZE | SWP_NOZORDER);

    EnableWindow(m_hWnd, FALSE);
    MSG msg2;
    while (GetMessage(&msg2, nullptr, 0, 0))
    {
        if (msg2.message == WM_USER + 100)
        {
            WORD id = LOWORD(msg2.wParam);
            if (id == IDOK)
            {
                int sel = (int)SendMessageW(hType, CB_GETCURSEL, 0, 0);
                if (sel >= 0)
                {
                    wchar_t nameBuf[128] = {};
                    GetWindowTextW(hName, nameBuf, 128);

                    auto child = std::make_shared<BTNode>();
                    child->name = nameBuf;
                    child->parent = parent.get();
                    child->depth = parent->depth + 1;

                    struct TMap { int idx; BTNodeType t; ConditionType ct; ActionType at; };
                    static const TMap map[] = {
                        {0,BTNodeType::SEQUENCE},{1,BTNodeType::SELECTOR},{2,BTNodeType::PARALLEL},{3,BTNodeType::RANDOM},
                        {4,BTNodeType::PROBABILITY},{5,BTNodeType::MEM_SEQUENCE},{6,BTNodeType::MEM_SELECTOR},
                        {7,BTNodeType::INVERTER},{8,BTNodeType::DECORATOR_REPEAT},{9,BTNodeType::SUCCEEDER},{10,BTNodeType::FAILER},
                        {11,BTNodeType::CONDITION,ConditionType::LOW_HP},{12,BTNodeType::CONDITION,ConditionType::LOW_MP},
                        {13,BTNodeType::CONDITION,ConditionType::HAS_TARGET},{14,BTNodeType::CONDITION,ConditionType::IN_SAFE_AREA},{15,BTNodeType::CONDITION,ConditionType::BAG_FULL},
                        {16,BTNodeType::ACTION,ConditionType::NONE,ActionType::ATTACK},{17,BTNodeType::ACTION,ConditionType::NONE,ActionType::MOVE_TO_TARGET},
                        {18,BTNodeType::ACTION,ConditionType::NONE,ActionType::PATROL},{19,BTNodeType::ACTION,ConditionType::NONE,ActionType::USE_POTION},
                        {20,BTNodeType::ACTION,ConditionType::NONE,ActionType::USE_SKILL},{21,BTNodeType::ACTION,ConditionType::NONE,ActionType::FLEE},
                        {22,BTNodeType::ACTION,ConditionType::NONE,ActionType::CHAT},{23,BTNodeType::ACTION,ConditionType::NONE,ActionType::REST},
                        {24,BTNodeType::ACTION,ConditionType::NONE,ActionType::PICKUP_ITEM},{25,BTNodeType::ACTION,ConditionType::NONE,ActionType::USE_ITEM},
                    };
                    for (auto& m : map) {
                        if (m.idx == sel) { child->type = m.t; child->conditionType = m.ct; child->actionType = m.at; break; }
                    }
                    child->id = std::to_wstring(child->depth) + L"_" + GetTagName(child->type, child->conditionType, child->actionType) + L"_" + std::to_wstring((size_t)child.get());
                    child->params = GetDefaultParams(child->type, child->conditionType, child->actionType);

                    parent->children.push_back(child);
                    PopulateTreeView();
                    AppendLogText(L"[编辑] 已在 '" + parent->name + L"' 下添加子节点 '" + child->name + L"'\r\n");
                }
                DestroyWindow(hDlg);
                EnableWindow(m_hWnd, TRUE);
                SetFocus(m_hWnd);
                break;
            }
            else if (id == IDCANCEL)
            {
                DestroyWindow(hDlg);
                EnableWindow(m_hWnd, TRUE);
                SetFocus(m_hWnd);
                break;
            }
        }
        else if (msg2.message == WM_USER + 101)
        {
            DestroyWindow(hDlg);
            EnableWindow(m_hWnd, TRUE);
            SetFocus(m_hWnd);
            break;
        }
        if (!IsWindow(hDlg)) break;
        if (!IsDialogMessageW(hDlg, &msg2))
        {
            TranslateMessage(&msg2);
            DispatchMessage(&msg2);
        }
    }
}

void CBTDebugger::EditNodeProperties()
{
    auto node = GetSelectedNode();
    if (!node) return;

    HWND hDlg = CreateWindowExW(WS_EX_DLGMODALFRAME, L"BTDlgFrame", L"",
        WS_POPUP | WS_CAPTION | WS_SYSMENU | WS_VISIBLE,
        0, 0, 400, 270, m_hWnd, nullptr, m_hInstance, nullptr);
    if (!hDlg) return;

    wchar_t title[256];
    wsprintfW(title, L"编辑属性 - %s", node->name.c_str());
    SetWindowTextW(hDlg, title);

    int xM = 12, y = 10, w = 376;
    CreateWindowW(L"STATIC", L"参数 (每行一个, 格式: key=value):",
        WS_CHILD | WS_VISIBLE, xM, y, w - xM, 16, hDlg, nullptr, nullptr, nullptr);

    y += 20;
    std::wstring params;
    for (auto& p : node->params)
        params += p.first + L"=" + p.second + L"\r\n";

    HWND hParams = CreateWindowW(L"EDIT", params.c_str(),
        WS_CHILD | WS_VISIBLE | WS_BORDER | ES_MULTILINE | ES_AUTOVSCROLL | WS_VSCROLL,
        xM, y, w - xM, 150, hDlg, nullptr, nullptr, nullptr);

    y += 158;
    CreateWindowW(L"BUTTON", L"确定", WS_CHILD | WS_VISIBLE | BS_DEFPUSHBUTTON,
        w - 180, y, 80, 26, hDlg, (HMENU)IDOK, nullptr, nullptr);
    CreateWindowW(L"BUTTON", L"取消", WS_CHILD | WS_VISIBLE,
        w - 90, y, 80, 26, hDlg, (HMENU)IDCANCEL, nullptr, nullptr);

    // 居中
    RECT wr;
    GetWindowRect(m_hWnd, &wr);
    SetWindowPos(hDlg, nullptr, wr.left + (wr.right - wr.left - 400) / 2,
        wr.top + (wr.bottom - wr.top - 270) / 2, 0, 0, SWP_NOSIZE | SWP_NOZORDER);

    EnableWindow(m_hWnd, FALSE);
    MSG msg2;
    while (GetMessage(&msg2, nullptr, 0, 0))
    {
        if (msg2.message == WM_USER + 100)
        {
            WORD id = LOWORD(msg2.wParam);
            if (id == IDOK)
            {
                int len = GetWindowTextLengthW(hParams);
                std::wstring text(len + 1, L'\0');
                GetWindowTextW(hParams, &text[0], len + 1);
                text.resize(len);

                node->params.clear();
                size_t pos = 0;
                while (pos < text.length())
                {
                    size_t eol = text.find(L'\n', pos);
                    if (eol == std::wstring::npos) eol = text.length();
                    std::wstring line = text.substr(pos, eol - pos);
                    while (!line.empty() && line.back() == L'\r') line.pop_back();
                    pos = eol + 1;
                    if (line.empty()) continue;
                    size_t eq = line.find(L'=');
                    if (eq != std::wstring::npos)
                        node->params[line.substr(0, eq)] = line.substr(eq + 1);
                }

                UpdatePropertyPanel();
                AppendLogText(L"[编辑] 已更新节点 '" + node->name + L"' 的属性\r\n");
                DestroyWindow(hDlg);
                EnableWindow(m_hWnd, TRUE);
                SetFocus(m_hWnd);
                break;
            }
            else if (id == IDCANCEL)
            {
                DestroyWindow(hDlg);
                EnableWindow(m_hWnd, TRUE);
                SetFocus(m_hWnd);
                break;
            }
        }
        else if (msg2.message == WM_USER + 101)
        {
            DestroyWindow(hDlg);
            EnableWindow(m_hWnd, TRUE);
            SetFocus(m_hWnd);
            break;
        }
        if (!IsWindow(hDlg)) break;
        if (!IsDialogMessageW(hDlg, &msg2))
        {
            TranslateMessage(&msg2);
            DispatchMessage(&msg2);
        }
    }
}

void CBTDebugger::OnPropListDoubleClick()
{
    EditNodeProperties();
}

void CBTDebugger::ChangeNodeType()
{
    auto node = GetSelectedNode();
    if (!node) return;

    HWND hDlg = CreateWindowExW(WS_EX_DLGMODALFRAME, L"BTDlgFrame", L"更改节点类型",
        WS_POPUP | WS_CAPTION | WS_SYSMENU | WS_VISIBLE,
        0, 0, 380, 130, m_hWnd, nullptr, m_hInstance, nullptr);
    if (!hDlg) return;

    int xM = 12, y = 10, w = 356;
    CreateWindowW(L"STATIC", L"选择新类型:",
        WS_CHILD | WS_VISIBLE, xM, y, 85, 20, hDlg, nullptr, nullptr, nullptr);
    HWND hType = CreateWindowW(L"COMBOBOX", L"",
        WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST | WS_VSCROLL,
        xM + 90, y, w - xM - 90, 200, hDlg, (HMENU)100, nullptr, nullptr);

    const wchar_t* typeNames[] = {
        L"Sequence (序列)", L"Selector (选择)", L"Parallel (并行)", L"Random (随机)",
        L"Probability (概率)", L"MemSequence (记忆序列)", L"MemSelector (记忆选择)",
        L"Inverter (反转)", L"DecoratorRepeat (重复)", L"Succeeder (强制成功)", L"Failer (强制失败)",
        L"ConditionLowHP (低血)", L"ConditionLowMP (低蓝)", L"ConditionHasTarget (有目标)",
        L"ConditionInSafeArea (安全区)", L"ConditionBagFull (背包满)",
        L"ActionAttack (攻击)", L"ActionMoveToTarget (移动到目标)", L"ActionPatrol (巡逻)",
        L"ActionUsePotion (喝药)", L"ActionUseSkill (技能)", L"ActionFlee (逃跑)",
        L"ActionChat (聊天)", L"ActionRest (休息)", L"ActionPickupItem (拾取)", L"ActionUseItem (使用道具)"
    };
    for (auto& tn : typeNames)
        SendMessageW(hType, CB_ADDSTRING, 0, (LPARAM)tn);

    // 根据当前类型预选
    struct { BTNodeType t; ConditionType ct; ActionType at; int idx; } preMap[] = {
        {BTNodeType::SEQUENCE, ConditionType::NONE, ActionType::NONE, 0},
        {BTNodeType::SELECTOR, ConditionType::NONE, ActionType::NONE, 1},
        {BTNodeType::PARALLEL, ConditionType::NONE, ActionType::NONE, 2},
        {BTNodeType::RANDOM, ConditionType::NONE, ActionType::NONE, 3},
        {BTNodeType::PROBABILITY, ConditionType::NONE, ActionType::NONE, 4},
        {BTNodeType::MEM_SEQUENCE, ConditionType::NONE, ActionType::NONE, 5},
        {BTNodeType::MEM_SELECTOR, ConditionType::NONE, ActionType::NONE, 6},
        {BTNodeType::INVERTER, ConditionType::NONE, ActionType::NONE, 7},
        {BTNodeType::DECORATOR_REPEAT, ConditionType::NONE, ActionType::NONE, 8},
        {BTNodeType::SUCCEEDER, ConditionType::NONE, ActionType::NONE, 9},
        {BTNodeType::FAILER, ConditionType::NONE, ActionType::NONE, 10},
        {BTNodeType::CONDITION, ConditionType::LOW_HP, ActionType::NONE, 11},
        {BTNodeType::CONDITION, ConditionType::LOW_MP, ActionType::NONE, 12},
        {BTNodeType::CONDITION, ConditionType::HAS_TARGET, ActionType::NONE, 13},
        {BTNodeType::CONDITION, ConditionType::IN_SAFE_AREA, ActionType::NONE, 14},
        {BTNodeType::CONDITION, ConditionType::BAG_FULL, ActionType::NONE, 15},
        {BTNodeType::ACTION, ConditionType::NONE, ActionType::ATTACK, 16},
        {BTNodeType::ACTION, ConditionType::NONE, ActionType::MOVE_TO_TARGET, 17},
        {BTNodeType::ACTION, ConditionType::NONE, ActionType::PATROL, 18},
        {BTNodeType::ACTION, ConditionType::NONE, ActionType::USE_POTION, 19},
        {BTNodeType::ACTION, ConditionType::NONE, ActionType::USE_SKILL, 20},
        {BTNodeType::ACTION, ConditionType::NONE, ActionType::FLEE, 21},
        {BTNodeType::ACTION, ConditionType::NONE, ActionType::CHAT, 22},
        {BTNodeType::ACTION, ConditionType::NONE, ActionType::REST, 23},
        {BTNodeType::ACTION, ConditionType::NONE, ActionType::PICKUP_ITEM, 24},
        {BTNodeType::ACTION, ConditionType::NONE, ActionType::USE_ITEM, 25},
    };
    int preSel = 0;
    for (auto& m : preMap) {
        if (node->type == m.t && node->conditionType == m.ct && node->actionType == m.at)
        { preSel = m.idx; break; }
    }
    SendMessageW(hType, CB_SETCURSEL, preSel, 0);

    y = 50;
    CreateWindowW(L"BUTTON", L"确定", WS_CHILD | WS_VISIBLE | BS_DEFPUSHBUTTON,
        w - 180, y, 80, 26, hDlg, (HMENU)IDOK, nullptr, nullptr);
    CreateWindowW(L"BUTTON", L"取消", WS_CHILD | WS_VISIBLE,
        w - 90, y, 80, 26, hDlg, (HMENU)IDCANCEL, nullptr, nullptr);

    RECT wr;
    GetWindowRect(m_hWnd, &wr);
    SetWindowPos(hDlg, nullptr, wr.left + (wr.right - wr.left - 380) / 2,
        wr.top + (wr.bottom - wr.top - 130) / 2, 0, 0, SWP_NOSIZE | SWP_NOZORDER);

    EnableWindow(m_hWnd, FALSE);
    MSG msg2;
    while (GetMessage(&msg2, nullptr, 0, 0))
    {
        if (msg2.message == WM_USER + 100)
        {
            WORD id = LOWORD(msg2.wParam);
            if (id == IDOK)
            {
                int sel = (int)SendMessageW(hType, CB_GETCURSEL, 0, 0);
                if (sel >= 0)
                {
                    static const struct { int idx; BTNodeType t; ConditionType ct; ActionType at; } map[] = {
                        {0,BTNodeType::SEQUENCE},{1,BTNodeType::SELECTOR},{2,BTNodeType::PARALLEL},{3,BTNodeType::RANDOM},
                        {4,BTNodeType::PROBABILITY},{5,BTNodeType::MEM_SEQUENCE},{6,BTNodeType::MEM_SELECTOR},
                        {7,BTNodeType::INVERTER},{8,BTNodeType::DECORATOR_REPEAT},{9,BTNodeType::SUCCEEDER},{10,BTNodeType::FAILER},
                        {11,BTNodeType::CONDITION,ConditionType::LOW_HP},{12,BTNodeType::CONDITION,ConditionType::LOW_MP},
                        {13,BTNodeType::CONDITION,ConditionType::HAS_TARGET},{14,BTNodeType::CONDITION,ConditionType::IN_SAFE_AREA},{15,BTNodeType::CONDITION,ConditionType::BAG_FULL},
                        {16,BTNodeType::ACTION,ConditionType::NONE,ActionType::ATTACK},{17,BTNodeType::ACTION,ConditionType::NONE,ActionType::MOVE_TO_TARGET},
                        {18,BTNodeType::ACTION,ConditionType::NONE,ActionType::PATROL},{19,BTNodeType::ACTION,ConditionType::NONE,ActionType::USE_POTION},
                        {20,BTNodeType::ACTION,ConditionType::NONE,ActionType::USE_SKILL},{21,BTNodeType::ACTION,ConditionType::NONE,ActionType::FLEE},
                        {22,BTNodeType::ACTION,ConditionType::NONE,ActionType::CHAT},{23,BTNodeType::ACTION,ConditionType::NONE,ActionType::REST},
                        {24,BTNodeType::ACTION,ConditionType::NONE,ActionType::PICKUP_ITEM},{25,BTNodeType::ACTION,ConditionType::NONE,ActionType::USE_ITEM},
                    };
                    for (auto& m : map) {
                        if (m.idx == sel) { node->type = m.t; node->conditionType = m.ct; node->actionType = m.at; break; }
                    }
                    node->id = std::to_wstring(node->depth) + L"_" + GetTagName(node->type, node->conditionType, node->actionType) + L"_" + std::to_wstring((size_t)node.get());
                    // 合并默认参数（保留用户已设置的值）
                    auto defParams = GetDefaultParams(node->type, node->conditionType, node->actionType);
                    for (auto& dp : defParams)
                        if (node->params.find(dp.first) == node->params.end())
                            node->params[dp.first] = dp.second;
                    m_selectedNodeId = node->id;

                    PopulateTreeView();
                    // 恢复树视图选中状态
                    HTREEITEM hItem = TreeView_GetRoot(m_hTreeView);
                    while (hItem)
                    {
                        TVITEMW tv = {}; tv.hItem = hItem; tv.mask = TVIF_PARAM;
                        TreeView_GetItem(m_hTreeView, &tv);
                        if ((BTNode*)tv.lParam == node.get()) { TreeView_SelectItem(m_hTreeView, hItem); break; }
                        hItem = TreeView_GetNextItem(m_hTreeView, hItem, TVGN_NEXTVISIBLE);
                    }
                    UpdatePropertyPanel();
                    AppendLogText(L"[编辑] 节点 '" + node->name + L"' 类型已更改\r\n");
                }
                DestroyWindow(hDlg);
                EnableWindow(m_hWnd, TRUE);
                SetFocus(m_hWnd);
                break;
            }
            else if (id == IDCANCEL)
            {
                DestroyWindow(hDlg);
                EnableWindow(m_hWnd, TRUE);
                SetFocus(m_hWnd);
                break;
            }
        }
        else if (msg2.message == WM_USER + 101)
        {
            DestroyWindow(hDlg);
            EnableWindow(m_hWnd, TRUE);
            SetFocus(m_hWnd);
            break;
        }
        if (!IsWindow(hDlg)) break;
        if (!IsDialogMessageW(hDlg, &msg2))
        {
            TranslateMessage(&msg2);
            DispatchMessage(&msg2);
        }
    }
}

void CBTDebugger::DeleteSelectedNode()
{
    auto node = GetSelectedNode();
    if (!node || !node->parent) return; // 不能删除根节点

    int result = MessageBoxW(m_hWnd,
        (L"确定要删除节点 '" + node->name + L"' 及其所有子节点吗？\n此操作不可撤销！").c_str(),
        L"删除节点", MB_YESNO | MB_ICONWARNING);

    if (result != IDYES) return;

    auto parent = node->parent;
    auto& siblings = parent->children;
    siblings.erase(std::remove_if(siblings.begin(), siblings.end(),
        [&](std::shared_ptr<BTNode>& c) { return c == node; }), siblings.end());

    m_selectedNodeId.clear();
    PopulateTreeView();
    UpdatePropertyPanel();
    UpdateStatusBar();
    AppendLogText(L"[编辑] 已删除节点 '" + node->name + L"'\r\n");
}

void CBTDebugger::MoveSelectedNodeUp()
{
    auto node = GetSelectedNode();
    if (!node || !node->parent) return;

    auto& siblings = node->parent->children;
    for (size_t i = 1; i < siblings.size(); i++)
    {
        if (siblings[i] == node)
        {
            std::swap(siblings[i], siblings[i - 1]);
            PopulateTreeView();
            break;
        }
    }
}

void CBTDebugger::MoveSelectedNodeDown()
{
    auto node = GetSelectedNode();
    if (!node || !node->parent) return;

    auto& siblings = node->parent->children;
    for (size_t i = 0; i + 1 < siblings.size(); i++)
    {
        if (siblings[i] == node)
        {
            std::swap(siblings[i], siblings[i + 1]);
            PopulateTreeView();
            break;
        }
    }
}

void CBTDebugger::UpdatePropertyPanel()
{
    ListView_DeleteAllItems(m_hPropList);

    if (m_selectedNodeId.empty() || !m_pRoot) return;

    auto node = FindNodeById(m_pRoot, m_selectedNodeId);
    if (!node) return;

    auto addRow = [&](const wchar_t* key, const std::wstring& val) {
        LVITEMW lvi = {};
        lvi.mask = LVIF_TEXT;
        lvi.pszText = (LPWSTR)key;
        int idx = ListView_InsertItem(m_hPropList, &lvi);
        ListView_SetItemText(m_hPropList, idx, 1, (LPWSTR)val.c_str());
    };

    addRow(L"节点名称", node->name);
    addRow(L"节点类型", GetNodeTypeDetail(node->type, node->conditionType, node->actionType));
    addRow(L"节点分类", GetNodeCategory(node->type) == BTNodeCategory::COMPOSITE ? L"复合节点" :
        GetNodeCategory(node->type) == BTNodeCategory::DECORATOR ? L"装饰节点" :
        GetNodeCategory(node->type) == BTNodeCategory::CONDITION ? L"条件节点" : L"动作节点");
    addRow(L"深度", std::to_wstring(node->depth));
    addRow(L"子节点数", std::to_wstring(node->children.size()));
    addRow(L"结果", GetResultName(node->lastResult));
    addRow(L"执行顺序", node->execOrder >= 0 ? std::to_wstring(node->execOrder) : L"未执行");

    // 参数
    for (auto& param : node->params)
    {
        addRow(param.first.c_str(), param.second);
    }
}

void CBTDebugger::UpdateLogPanel()
{
    std::wstring text;
    auto& logs = m_engine.GetLogs();

    for (size_t i = 0; i < logs.size(); i++)
    {
        auto& log = logs[i];

        wchar_t buf[256];
        wsprintfW(buf, L"%03d  ", (int)(i + 1));

        // 缩进
        for (int d = 0; d < log.depth; d++)
            text += L"  ";

        text += buf;
        text += L"[";
        text += GetResultName(log.result);
        text += L"]  ";
        text += log.nodeName;
        text += L"  (";
        text += GetNodeTypeName(log.type);
        text += L")\r\n";
    }

    SetLogText(text);
}

void CBTDebugger::UpdateStatusBar()
{
    if (!m_pRoot)
    {
        SendMessageW(m_hStatusBar, SB_SETTEXTW, 0, (LPARAM)L"就绪 - 请加载行为树文件");
        return;
    }

    wchar_t buf[256];
    int totalNodes = CountNodes(m_pRoot);
    int logCount = (int)m_engine.GetLogs().size();
    wsprintfW(buf, L"节点总数: %d | 日志条数: %d | 速度: %dms %s",
        totalNodes, logCount, m_autoSpeed,
        m_isAutoRunning ? L"(自动运行中)" : L"");
    SendMessageW(m_hStatusBar, SB_SETTEXTW, 0, (LPARAM)buf);
}

void CBTDebugger::SetLogText(const std::wstring& text)
{
    SetWindowTextW(m_hLogEdit, text.c_str());
}

void CBTDebugger::AppendLogText(const std::wstring& text)
{
    int len = GetWindowTextLengthW(m_hLogEdit);
    SendMessageW(m_hLogEdit, EM_SETSEL, len, len);
    SendMessageW(m_hLogEdit, EM_REPLACESEL, FALSE, (LPARAM)text.c_str());
}

std::string CBTDebugger::WStringToUTF8(const std::wstring& wstr)
{
    int len = WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), -1, nullptr, 0, nullptr, nullptr);
    std::string result(len - 1, '\0');
    WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), -1, &result[0], len, nullptr, nullptr);
    return result;
}

std::wstring CBTDebugger::UTF8ToWString(const std::string& str)
{
    int len = MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, nullptr, 0);
    std::wstring result(len - 1, L'\0');
    MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, &result[0], len);
    return result;
}

LRESULT CALLBACK CBTDebugger::WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    auto* pApp = s_pInstance;

    switch (msg)
    {
    case WM_CREATE:
    {
        // 创建右键菜单
        HMENU hMenu = CreatePopupMenu();
        AppendMenuW(hMenu, MF_STRING, 10001, L"加载战士行为树");
        AppendMenuW(hMenu, MF_STRING, 10002, L"加载法师行为树");
        AppendMenuW(hMenu, MF_STRING, 10003, L"加载道士行为树");
        AppendMenuW(hMenu, MF_SEPARATOR, 0, nullptr);
        AppendMenuW(hMenu, MF_STRING, 10004, L"打开XML文件...");
        SetPropW(hWnd, L"ContextMenu", hMenu);
        break;
    }

    case WM_CONTEXTMENU:
    {
        // 判断右键来源：TreeView 有自己的 NM_RCLICK 处理，这里只处理窗口空白区域的右键
        if ((HWND)wParam != pApp->m_hTreeView)
        {
            HMENU hMenu = (HMENU)GetPropW(hWnd, L"ContextMenu");
            if (hMenu)
            {
                POINT pt = { LOWORD(lParam), HIWORD(lParam) };
                TrackPopupMenu(hMenu, TPM_LEFTALIGN | TPM_RIGHTBUTTON, pt.x, pt.y, 0, hWnd, nullptr);
            }
        }
        break;
    }

    case WM_COMMAND:
    {
        WORD id = LOWORD(wParam);
        switch (id)
        {
        case 10001: pApp->LoadXMLFromString(g_builtInSamples.at(L"战士战斗行为树")); break;
        case 10002: pApp->LoadXMLFromString(g_builtInSamples.at(L"法师战斗行为树")); break;
        case 10003: pApp->LoadXMLFromString(g_builtInSamples.at(L"道士战斗行为树")); break;
        case 10004: pApp->LoadXMLFile(); break;
        // 节点编辑右键菜单
        case IDM_RENAME_NODE: pApp->RenameSelectedNode(); break;
        case IDM_ADD_CHILD:   pApp->AddChildToSelectedNode(); break;
        case IDM_DELETE_NODE: pApp->DeleteSelectedNode(); break;
        case IDM_MOVE_UP:     pApp->MoveSelectedNodeUp(); break;
        case IDM_MOVE_DOWN:   pApp->MoveSelectedNodeDown(); break;
        case IDM_EDIT_PROPS:  pApp->EditNodeProperties(); break;
        case IDM_CHANGE_TYPE: pApp->ChangeNodeType(); break;
        default: pApp->OnCommand(id); break;
        }
        break;
    }

    case WM_NOTIFY:
    {
        LRESULT result = pApp->OnNotify((NMHDR*)lParam);
        SetWindowLongPtrW(hWnd, DWLP_MSGRESULT, result);
        return (result != 0) ? TRUE : FALSE;
    }

    case WM_HSCROLL:
    {
        if ((HWND)lParam == pApp->m_hSpeedSlider)
        {
            pApp->m_autoSpeed = (int)SendMessage(pApp->m_hSpeedSlider, TBM_GETPOS, 0, 0);
            wchar_t buf[32];
            wsprintfW(buf, L"速度: %dms", pApp->m_autoSpeed);
            SetWindowTextW(pApp->m_hSpeedLabel, buf);
            // 如果正在自动运行，重新设置定时器速率
            if (pApp->m_isAutoRunning)
            {
                KillTimer(hWnd, ID_TIMER_AUTO);
                SetTimer(hWnd, ID_TIMER_AUTO, pApp->m_autoSpeed, nullptr);
            }
        }
        break;
    }

    case WM_TIMER:
        pApp->OnTimer(wParam);
        break;

    case WM_SIZE:
        pApp->OnSize(LOWORD(lParam), HIWORD(lParam));
        break;

    case WM_PAINT:
    {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hWnd, &ps);
        pApp->OnPaint(hdc);
        EndPaint(hWnd, &ps);
        break;
    }

    case WM_LBUTTONDOWN:
        pApp->OnLButtonDown(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
        break;

    case WM_MOUSEMOVE:
        pApp->OnMouseMove(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
        break;

    case WM_LBUTTONUP:
        pApp->OnLButtonUp();
        break;

    case WM_CTLCOLORSTATIC:
    case WM_CTLCOLOREDIT:
    case WM_CTLCOLORLISTBOX:
    {
        HDC hdc = (HDC)wParam;
        SetBkColor(hdc, RGB(15, 15, 30));
        SetTextColor(hdc, RGB(200, 200, 220));
        static HBRUSH hBrush = CreateSolidBrush(RGB(15, 15, 30));
        return (LRESULT)hBrush;
    }

    case WM_DESTROY:
    {
        HMENU hMenu = (HMENU)GetPropW(hWnd, L"ContextMenu");
        if (hMenu) DestroyMenu(hMenu);
        RemovePropW(hWnd, L"ContextMenu");
        PostQuitMessage(0);
        break;
    }

    default:
        return DefWindowProcW(hWnd, msg, wParam, lParam);
    }
    return 0;
}

int CBTDebugger::Run()
{
    MSG msg;
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    return (int)msg.wParam;
}

// ============================================================================
// 程序入口
// ============================================================================
int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE, LPWSTR, int nCmdShow)
{
    // 初始化公共控件
    INITCOMMONCONTROLSEX icc = {};
    icc.dwSize = sizeof(icc);
    icc.dwICC = ICC_WIN95_CLASSES | ICC_TREEVIEW_CLASSES | ICC_LISTVIEW_CLASSES | ICC_BAR_CLASSES;
    InitCommonControlsEx(&icc);

    CBTDebugger app(hInstance);
    if (!app.Init(nCmdShow))
    {
        MessageBoxW(nullptr, L"窗口初始化失败！", L"错误", MB_ICONERROR);
        return 1;
    }

    return app.Run();
}