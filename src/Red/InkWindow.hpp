#pragma once

#include "Red/InkInput.hpp"

namespace Raw::inkWindow
{
// Game 2.5: inkWindow grew by 0x10, and TogglePointerInput sets the handler at 0x268 (0x258 on 2.31).
using PointerHandler = Core::OffsetPtr<0x268, Red::SharedPtr<Red::inkPointerHandler>>;

constexpr auto Construct = Core::RawFunc<
    /* addr = */ Red::AddressLib::InkWindow_ctor,
    /* type = */ void (*)(Red::inkWindow* aWindow)>();

constexpr auto Destruct = Core::RawFunc<
    /* addr = */ Red::AddressLib::InkWindow_ctor,
    /* type = */ void (*)(Red::inkWindow* aWindow)>();

constexpr auto TogglePointerInput = Core::RawFunc<
    /* addr = */ Red::AddressLib::InkWindow_TogglePointerInput,
    /* type = */ void (*)(Red::inkWindow* aWindow, bool aEnabled)>();

constexpr auto IsPointerVisible = Core::RawFunc<
    /* addr = */ Red::AddressLib::InkWindow_IsPointerVisible,
    /* type = */ bool (*)(Red::inkWindow* aWindow)>();

constexpr auto SetPointerVisibility = Core::RawFunc<
    /* addr = */ Red::AddressLib::InkWindow_SetPointerVisibility,
    /* type = */ void (*)(Red::inkWindow* aWindow, bool aVisible)>();

constexpr auto SetPointerWidget = Core::RawFunc<
    /* addr = */ Red::AddressLib::InkWindow_SetPointerWidget,
    /* type = */ void (*)(Red::inkWindow* aWindow, const Red::Handle<Red::inkWidget>& aCursor)>();
}
