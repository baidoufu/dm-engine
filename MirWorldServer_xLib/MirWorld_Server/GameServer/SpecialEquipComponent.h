#pragma once
#include <array>

/**
 *  SpecialEquipComponent — 特殊装备位标志数组
 *    1 个 SpecialEquipComponent = 语义清晰的 bool + DWORD 分离存储
 * 
 *  注意: special_equipment_func 枚举和 SEF_MAX 定义在 localdefine.h 中,
 *  本组件使用 int 作为 func 参数类型以避免头文件依赖。
 */
struct SpecialEquipComponent
{
    std::array<bool, SEF_MAX> active{};
    std::array<DWORD, SEF_MAX> posFlag{};

    SpecialEquipComponent()
    {
        active.fill(false);
        posFlag.fill(0);
    }

    bool IsOn(int func) const
    {
        if (func < 0 || func >= SEF_MAX) return false;
        return active[func];
    }

    bool SetOn(int func, DWORD dwPosFlag)
    {
        if (func < 0 || func >= SEF_MAX) return false;
        dwPosFlag |= 0x80000000;
        bool wasActive = active[func];
        posFlag[func] = dwPosFlag;
        active[func] = true;
        return !wasActive;
    }

    bool SetOff(int func)
    {
        if (func < 0 || func >= SEF_MAX) return false;
        bool wasActive = active[func];
        posFlag[func] = 0;
        active[func] = false;
        return wasActive;
    }

    DWORD GetPosFlag(int func) const
    {
        if (func < 0 || func >= SEF_MAX) return 0;
        return posFlag[func];
    }
};
