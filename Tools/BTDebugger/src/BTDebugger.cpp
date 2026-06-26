#include "BTDebugger.h"
#include <fstream>
#include <sstream>
#include <codecvt>
#include <map>
#include <CommCtrl.h>

#pragma comment(lib, "comctl32.lib")
#pragma comment(linker, "\"/manifestdependency:type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

CBTDebugger* CBTDebugger::s_pInstance = nullptr;

// 内置示例数据
static const std::map<std::wstring, std::string> g_builtInSamples = {
    { L"战士战斗行为树", R"(<?xml version="1.0" encoding="GBK"?>
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
</BehaviorTree>)" },

    { L"法师战斗行为树", R"(<?xml version="1.0" encoding="GBK"?>
<BehaviorTree name="法师战斗行为树 - Mage v2.0">
    <Selector name="法师主决策">
        <Sequence name="安全区行为">
            <ConditionInSafeArea name="在安全区" />
            <Parallel name="安全区并行行为">
                <Sequence name="安全区回血">
                    <ConditionLowHP name="HP<60%" percent="60" />
                    <ActionUsePotion name="喝HP药水" hpType="1" />
                </Sequence>
                <Sequence name="安全区回蓝">
                    <ConditionLowMP name="MP<50%" percent="50" />
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
            <ConditionLowHP name="HP<25%濒死" percent="25" />
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
                        <ConditionLowHP name="HP<45%" percent="45" />
                        <ActionUsePotion name="喝HP药水" hpType="1" />
                    </Sequence>
                    <Sequence name="战斗喝蓝">
                        <ConditionLowMP name="MP<40%" percent="40" />
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
</BehaviorTree>)" },

    { L"道士战斗行为树", R"(<?xml version="1.0" encoding="GBK"?>
<BehaviorTree name="道士战斗行为树 - Taoist v2.0">
    <Selector name="道士主决策">
        <Sequence name="安全区自动恢复">
            <ConditionInSafeArea name="在安全区" />
            <Parallel name="安全区行为">
                <Sequence name="回血">
                    <ConditionLowHP name="HP<65%" percent="65" />
                    <ActionUsePotion name="喝HP" hpType="1" />
                </Sequence>
                <Sequence name="回蓝">
                    <ConditionLowMP name="MP<35%" percent="35" />
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
            <ConditionLowHP name="HP<30%" percent="30" />
            <ActionFlee name="逃跑" />
        </Sequence>
        <Sequence name="战斗主流程">
            <ConditionHasTarget name="有目标" />
            <ActionChangeAttackMode name="切善恶模式" attackMode="0" />
            <Sequence name="战前MP检查">
                <ConditionLowMP name="MP<50%" percent="50" />
                <ActionUsePotion name="战前喝蓝" hpType="0" />
            </Sequence>
            <Parallel name="战斗并行策略">
                <Sequence name="自我治疗">
                    <ConditionLowHP name="HP<50%加血" percent="50" />
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
                            <ConditionLowHP name="HP<40%" percent="40" />
                            <ActionUsePotion name="喝HP" hpType="1" />
                        </Sequence>
                        <Sequence name="喝MP">
                            <ConditionLowMP name="MP<25%" percent="25" />
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
</BehaviorTree>)" }
};

CBTDebugger::CBTDebugger(HINSTANCE hInstance)
    : m_hInstance(hInstance), m_hWnd(nullptr)
    , m_hTreeView(nullptr), m_hPropList(nullptr), m_hLogEdit(nullptr)
    , m_hStatusBar(nullptr), m_hBtnLoad(nullptr), m_hBtnStep(nullptr)
    , m_hBtnAuto(nullptr), m_hBtnReset(nullptr), m_hSpeedSlider(nullptr)
    , m_hSpeedLabel(nullptr)
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

    // 创建窗口
    m_hWnd = CreateWindowExW(
        WS_EX_APPWINDOW,
        L"BTDebuggerWindow",
        L"行为树可视化调试器 - DM Engine",
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

    m_hBtnStep = CreateWindowW(L"BUTTON", L"▶| 单步执行",
        WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
        136, 8, 100, 28, m_hWnd, (HMENU)ID_BTN_STEP, m_hInstance, nullptr);

    m_hBtnAuto = CreateWindowW(L"BUTTON", L"▶ 自动播放",
        WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
        244, 8, 100, 28, m_hWnd, (HMENU)ID_BTN_AUTO, m_hInstance, nullptr);

    m_hBtnReset = CreateWindowW(L"BUTTON", L"↺ 重置",
        WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
        352, 8, 80, 28, m_hWnd, (HMENU)ID_BTN_RESET, m_hInstance, nullptr);

    m_hSpeedLabel = CreateWindowW(L"STATIC", L"速度: 500ms",
        WS_CHILD | WS_VISIBLE | SS_LEFT,
        440, 12, 100, 20, m_hWnd, (HMENU)ID_SPEED_LABEL, m_hInstance, nullptr);

    m_hSpeedSlider = CreateWindowW(TRACKBAR_CLASSW, L"",
        WS_CHILD | WS_VISIBLE | TBS_HORZ | TBS_AUTOTICKS,
        540, 8, 150, 28, m_hWnd, (HMENU)ID_SPEED_SLIDER, m_hInstance, nullptr);
    SendMessage(m_hSpeedSlider, TBM_SETRANGE, TRUE, MAKELONG(100, 2000));
    SendMessage(m_hSpeedSlider, TBM_SETPOS, TRUE, 500);
    SendMessage(m_hSpeedSlider, TBM_SETTICFREQ, 200, 0);

    // 树形视图
    m_hTreeView = CreateWindowExW(WS_EX_CLIENTEDGE,
        WC_TREEVIEWW, L"",
        WS_CHILD | WS_VISIBLE | TVS_HASLINES | TVS_LINESATROOT | TVS_HASBUTTONS | TVS_SHOWSELALWAYS,
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

void CBTDebugger::OnNotify(NMHDR* pnmh)
{
    if (pnmh->idFrom == ID_TREEVIEW && pnmh->code == TVN_SELCHANGEDW)
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
    SetWindowTextW(m_hBtnAuto, L"⏸ 暂停");
    SetTimer(m_hWnd, ID_TIMER_AUTO, m_autoSpeed, nullptr);
    StepExecute();
}

void CBTDebugger::StopAuto()
{
    m_isAutoRunning = false;
    KillTimer(m_hWnd, ID_TIMER_AUTO);
    SetWindowTextW(m_hBtnAuto, L"▶ 自动播放");
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
    addRow(L"节点类型", GetNodeTypeName(node->type));
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
        HMENU hMenu = (HMENU)GetPropW(hWnd, L"ContextMenu");
        if (hMenu)
        {
            POINT pt = { LOWORD(lParam), HIWORD(lParam) };
            TrackPopupMenu(hMenu, TPM_LEFTALIGN | TPM_RIGHTBUTTON, pt.x, pt.y, 0, hWnd, nullptr);
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
        default: pApp->OnCommand(id); break;
        }
        break;
    }

    case WM_NOTIFY:
        pApp->OnNotify((NMHDR*)lParam);
        break;

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