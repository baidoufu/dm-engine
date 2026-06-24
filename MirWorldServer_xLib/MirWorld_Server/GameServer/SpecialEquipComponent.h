#pragma once
#include <array>

/**
 *  SpecialEquipComponent — 替代 CHumanPlayer 中 m_dwSpecialEquipmentFunctionFlags 位标志数组
 *
 *  注意: special_equipment_func 枚举和 SEF_MAX 定义在 localdefine.h 中,
 *  本组件使用 int 作为 func 参数类型以避免头文件依赖。
 *
 *  改造前:
 *    std::array<DWORD, SEF_MAX> m_dwSpecialEquipmentFunctionFlags;
 *    位31 = 激活标志 (0x80000000)
 *    位0-30 = 生效装备位置标记
 *
 *  改造后:
 *    1 个 SpecialEquipComponent = 语义清晰的 bool + DWORD 分离存储
 */
struct SpecialEquipComponent
{
    static constexpr int SEF_MAX_VAL = 64; // 与 localdefine.h 中 SEF_MAX 一致

    std::array<bool, SEF_MAX_VAL> active{};
    std::array<DWORD, SEF_MAX_VAL> posFlag{};

    SpecialEquipComponent()
    {
        active.fill(false);
        posFlag.fill(0);
    }

    bool IsOn(int func) const
    {
        if (func < 0 || func >= SEF_MAX_VAL) return false;
        return active[func];
    }

    bool SetOn(int func, DWORD dwPosFlag)
    {
        if (func < 0 || func >= SEF_MAX_VAL) return false;
        dwPosFlag |= 0x80000000;
        bool wasActive = active[func];
        posFlag[func] = dwPosFlag;
        active[func] = true;
        return !wasActive;
    }

    bool SetOff(int func)
    {
        if (func < 0 || func >= SEF_MAX_VAL) return false;
        bool wasActive = active[func];
        posFlag[func] = 0;
        active[func] = false;
        return wasActive;
    }

    DWORD GetPosFlag(int func) const
    {
        if (func < 0 || func >= SEF_MAX_VAL) return 0;
        return posFlag[func];
    }
};
