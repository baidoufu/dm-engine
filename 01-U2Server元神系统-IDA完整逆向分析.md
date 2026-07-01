# U2Server 元神系统 — IDA全模块深度逆向 最终完整版

> 分析范围: 15+函数反编译、网络层、编码层、属性层、包分发层、DB层
> 二进制: NewGs.exe (1.5MB, 0x400000基址)
> 最后更新: 2026-06-29

---

## 零、架构全景图

```
┌─────────────────────────────────────────────────────────────────┐
│ 网络层                                                            │
│  sub_A15600 (主消息循环)                                          │
│    └─ sub_A15810 (收包+解析)                                      │
│         ├─ sub_A18F00 (WinSock recv)                              │
│         └─ sub_A15A20 (消息类型分发: type 1-9)                     │
│              ├─ case 5: CreateThread → sub_A17180 → sub_A16550    │
│              │    └─ sub_A16BE0 (认证连接收包)                     │
│              └─ case 7: sub_A19A00 (创建玩家对象)                  │
├─────────────────────────────────────────────────────────────────┤
│ 包分发层                                                          │
│  sub_823B60 (25KB 注册表构建器)                                    │
│    └─ sub_4035BC(name, funcptr) × 数百次                          │
│         注册 Map: "Move"→sub_40185C, "Attackmode"→sub_403706...   │
│  运行时 dispatch: 按名称查表 → thunk → handler                     │
│  元神相关: sub_8958A0 (→ sub_407680)                              │
├─────────────────────────────────────────────────────────────────┤
│ 业务层                                                            │
│  sub_87EE40 (元神状态机, 2023字节)                                  │
│  sub_896A20 (元神定时器, 每5秒轮询)                                 │
│  sub_8392C0 (职业切换: 战士/魔法师/道士)                            │
│  sub_504D80 (UI消息构建器: name/map/guild/sex/job/phase/...)      │
├─────────────────────────────────────────────────────────────────┤
│ 编码层                                                            │
│  sub_9E6F80 (自定义编码: XOR 0xEB → 5状态循环, 字母表从';'开始)     │
│  最终格式: #<seq>/<encoded_data>!                                  │
├─────────────────────────────────────────────────────────────────┤
│ 网络发送层                                                        │
│  路径A: sub_40877E → sub_4E2FC0 → send(sock, buf, len, 0)        │
│  路径B: sub_404F34 → sub_56C110 → sub_409EE9                     │
│            └─ sub_4D8CA0: [0xAA55AA55][session][type_len][body]...│
│                 └─ sub_9EC5D0 → send()                            │
├─────────────────────────────────────────────────────────────────┤
│ 数据层                                                            │
│  Python脚本 → createShadow → sub_93BA70 → sub_4039D6 → DB写入     │
│  supershadowinfo 表: ShadowName, ShadowBornLevel, ShadowLoginTime │
│  主DB函数: sub_6EF970 (15129字节, 创所有表)                        │
└─────────────────────────────────────────────────────────────────┘
```

---

## 一、网络层

### 1.1 连接接受

```
sub_A15600: 主消息循环入口
  └─ sub_A15810: 游戏socket收包+解析
       ├─ sub_A18F00(this): 调用 this[9] 作为socket → recv()
       └─ sub_A15A20: 消息类型分发
            ├─ type 1: 心跳
            ├─ type 5: 新连接 → sub_A16550 (认证线程)
            │    └─ sub_A16BE0: 认证连接数据包处理
            │         ├─ type 0: 初始
            │         ├─ type 2: 登录
            │         ├─ type 3: 创角
            │         └─ type 5: 未知
            └─ type 7: 创建玩家对象
```

### 1.2 登录前缀识别 (&*)

**认证连接处理**: `sub_A16BE0` → 解密后检查首字符

```
sub_A16BE0 (认证连接收包+解析):
  ├─ sub_A18F00: WinSock recv
  ├─ sub_A19570 / sub_A19520: XOR解密 (identical twin functions)
  │    算法: *v5 = key_table[(a1&1) + 2*(a1&7)] ^ input_byte
  │    这是 Woool 标准加密/解密层
  ├─ switch (message_type):
  │    ├─ type 0: ExitProcess (错误)
  │    ├─ type 2: sub_A16E90(2) → 发送响应
  │    ├─ type 3: 累积128字节后发送
  │    └─ type 5: 设置连接标志
  └─ sub_A19520(this+164, ...) → 解密后的明文存于this+164
       ↓
  (下一层) 检查明文首字符:
       '&' → 元神连接
       '*' → 正常连接
       '$' → 凤凰连接
```

**注意**: `&*` 前缀的检查逻辑在解密之后的明文处理层, 不在 sub_A16BE0 本身。解密后的字符串以 `&*账号/角色名/SessionID/版本` 格式存放, 下一层(`sub_A16550` 登录线程)根据首字符分发。

---

## 二、包分发层

### 2.1 注册表构建

`sub_823B60` (25KB, 0x823B60) — 数百个 handler 注册:

```
sub_4035BC("Move",         sub_40185C)   // 移动
sub_4035BC("Squat",        ...)          // 蹲下
sub_4035BC("Run",          ...)          // 跑步
sub_4035BC("Attackmode",   sub_403706 → sub_8777F0)
sub_4035BC("Horse",        ... → sub_878B40)
...
// 元神相关 (0xB40B40字符串区域):
sub_4035BC("getShadow",    sub_406D70)
sub_4035BC("createShadow", sub_406E65)
...
```

### 2.2 元神handler

`sub_8958A0`: 检查 `a1+252 == 1` (是否元神连接), 是则跳过, 否则调 `sub_407680` 正常处理。

---

## 三、编码层

### 3.1 自定义编码算法

**入口**: `sub_9E6F80` (← `sub_4044E4` thunk)

```c
// 算法: XOR 0xEB → 5状态循环编码
const char BASE = ';';  // 0x3B, 64字符字母表从';'开始
for each byte in input:
    b = byte ^ 0xEB;
    state = (state + 1) % 5;
    
    if (state == 4):
        out[0] = BASE + (b & 0x3F);
        out[1] = BASE + ((b>>2) & 0x30 | carry);
        carry = 0;
    else:
        out[0] = BASE + (b & 3 | (b>>2) & 0x3C);
        carry = (carry << 2) | ((b>>2) & 3);
```

- 字母表: `;` 开始的64字符
- 5状态循环 (标准Base64是4状态)
- 先XOR 0xEB再编码

### 3.2 消息格式

```
业务消息: #<seq_num>/<encoded_body>!

其中:
  seq_num     = 递增全局计数器 (dword_B54524)
  encoded_body = custom_base64(xor_0xEB(plaintext))
  plaintext    = "P" + "name/map/guild/sex/job/phase/flag/value!"  (sub_504D80构建)
```

### 3.3 网络封包

`sub_4D8CA0` 组包:
```
[0xAA55AA55:4] [session_id:4] [type:2|len:2] [body_len:4] [body:body_len] [payload]
                                                                    ↑ #seq/data! 放这里
```

---

## 四、业务层

### 4.1 元神状态机 (sub_87EE40, 2023字节)

```
输入: a1=玩家对象, a2=packet参数
流程:
  1. 检查 a2+8 WORD <= 7 (状态范围)
  2. 检查 a1+220 (m_pShadow) 非空
  3. 保存 a2 到 a1+258
  4. 读 ShadowName 配置键
  5. 比较 a1+2220 (当前ShadowName) 与配置
  6. 如果不同:
     - 查找在线玩家 (遍历 a1+940 链表)
     - 读 v12[270]=性别, v12[271]=阶段
     - 调用 sub_40647E → sub_504D80 构建UI消息
     - 调用 sub_40877E 发送
     - 更新DB
```

### 4.2 元神定时器 (sub_896A20, 1479字节)

```
每触发一次:
  if (!a1+2892) return;     // 定时器未启用
  if (!a1+220) return;       // 无元神对象
  if (tick - a1+2920 < 5000) return;  // 5秒冷却
  
  if (a1+2880 == 2):          // 特殊模式
    if (≥60秒): 执行+重置
  else:
    v10 = a1+2884;           // shadow指针2
    if (!v10): 10秒冷却后执行
    else if (v10[55]): 只更新时间, 不执行 (busy)
    else: 执行
```

### 4.3 职业切换 (sub_8392C0, 1965字节)

```
比较输入字符串 v66:
  == "战士" (0xAF4650)  → v11[568] = 0, sub_401AFA 应用
  == "魔法师" (0xB3C5CC) → v11[568] = 1, sub_401AFA 应用
  == "道士" (0xAF4640)   → v11[568] = 2, sub_401AFA 应用
  != 任一 → 默认处理 (unk_B3C5BC)
  
  通知: sub_404F34 发送更新
```

### 4.4 UI消息构建 (sub_504D80)

```c
// 18个参数, 构建元神UI字符串
sprintf(Buffer, "%s/%s/%s/%d/%d/%d/%d/%d!",
    a3,     // 角色名
    a7,     // 地图名
    a11,    // 行会名
    a14,    // 性别 (BYTE)
    a15,    // 职业 (BYTE)  
    a16,    // 阶段 (BYTE)
    a17,    // 标志 (BYTE)
    a18     // 值 (int)
);
```

---

## 五、数据层

### 5.1 数据库

| 表名 | 字段 | 说明 |
|------|------|------|
| supershadowinfo | ShadowName varchar(20) | 元神名字 |
| supershadowinfo | ShadowBornLevel int | 初始等级 |
| supershadowinfo | ShadowLoginTime int | 登录时间 |
| supershadowinfo | ShadowNum int | 元神编号 |

建表函数: `sub_6EF970` (0x6EF970, 15129字节)

### 5.2 DB操作

```
CDBPacketBuilder::RequestCreateShadow → DB INSERT
CDBPacketBuilder::RequestDeleteShadow → DB DELETE
```

---

## 六、数据结构偏移总表

| 偏移 | 类型 | 名称 | 用途 |
|------|------|------|------|
| +164 | WORD | wIdent | packet ID |
| +166 | WORD | wParam | packet参数 |
| +188 | char[] | m_sCharName | 角色名 |
| +220 | ptr | m_pShadow | 元神对象指针 |
| +252 | int | m_nStatus | 状态 (1=元神) |
| +258 | WORD | wTag | packet tag |
| +266 | WORD | m_wCurHP | 当前HP (宠物) |
| +468 | WORD | m_wAttr1 | 属性1 (配置'%'指令) |
| +470 | WORD | m_wAttr2 | 属性2 (配置'&'指令) |
| +532 | int | m_nMax | 范围上限 |
| +568 | BYTE | m_btJob | 职业 (0/1/2) |
| +2220 | int | m_nShadowNameHash | 元神名hash |
| +2880 | BYTE | m_btShadowState | 元神状态 (2=特殊) |
| +2884 | ptr | m_pShadow2 | 元神指针2 |
| +2892 | BYTE | m_bShadowTimer | 定时器开关 |
| +2917 | BYTE | m_bShadowExec | 执行标志 |
| +2920 | DWORD | m_dwLastTick | 上次tick |
| +2924 | BYTE | m_btCreateAttempt | 创建尝试计数 |
| v12+270 | BYTE | m_btSex | 元神性别 |
| v12+271 | BYTE | m_btPhase | 元神阶段 |

---

## 七、Python脚本层

| 函数 | thunk | 实际地址 | 功能 |
|------|-------|---------|------|
| createShadow | 0x406E65 | 0x93BA70 | 创建元神 |
| deleteShadow | 0x404566 | 0x93C100 | 删除元神 |
| getShadow | 0x406D70 | 0x93C250 | 获取元神 |
| getShadowName | 0x40711C | 0x93C3B0 | 获取名字 |
| shadowIsOnline | 0x4066F4 | 0x93BEE0 | 在线检查 |
| ShadowInit | 0x408E9A | 0x93A9A0 | 初始化 |
| getShadowPhase | 0x4015E1 | 0x93F220 | 获取阶段 |
| setShadowPhase | 0x401B31 | 0x93FA30 | 设置阶段 |
| getShadowAppellation | 0x40894F | 0x93BDE0 | 获取称号 |
| ShadowHuaXing | 0x401C7B | 0x93FB50 | 化形 |
| ShadowLianTi | 0x40143D | 0x93F070 | 炼体 |
| ShadowQiPo | 0x401C7B | 0x93FB50 | 七魄 |
| ShadowZhongShu | 0x40143D | 0x93F070 | 中枢 |

---

## 八、与Glory Engine/彩虹3的对比

| 模块 | U2Server | Glory Engine | 评价 |
|------|---------|-------------|------|
| 驱动方式 | 轮询定时器(5s) | 事件驱动 | Glory更优 |
| 编码算法 | XOR 0xEB + 5状态 | XOR + #!帧 | 同体系 |
| 网络封包 | 0xAA55AA55 header | TDefaultMessage | 同体系 |
| 包分发 | 字符串名注册表(25KB) | 多态版本管理 | U2简单, Glory灵活 |
| DB存储 | 独立supershadowinfo表 | 共享HumanDB表 | U2更清晰 |
| 脚本层 | Python (PyArg_ParseTuple) | Delphi脚本系统 | 实现不同, 架构同 |
| 对象模型 | 独立对象+标记位 | 独立TPlayObject+标记 | 完全一致 |
