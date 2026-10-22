#pragma once

#include "Red/InkWindow.hpp"

namespace Raw::inkLayer
{
// Checked on game 2.5 (verifier run 20261022-093218): slot 0x128 of inkLayer's virtual table reads
// the layer's window (0x40) and returns the window's pointer handler (0x268).
constexpr auto GetPointerHandler = Core::RawVFunc<
    /* addr = */ 0x128,
    /* type = */ Red::inkPointerHandler* (Red::inkLayer::*)()>();

constexpr auto AttachLibraryInstance = Core::RawFunc<
    /* addr = */ Red::AddressLib::InkLayer_AttachLibraryInstance,
    /* type = */ void (*)(Red::inkLayer* aLayer,
                          const Red::Handle<Red::inkWidgetLibraryItemInstance>& aInstance,
                          const Red::Handle<Red::inkWidgetLibraryResource>& aLibrary)>();
}
