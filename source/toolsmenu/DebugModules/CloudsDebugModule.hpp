#pragma once

#include "DebugModule.h"

namespace notsa { 
namespace debugmodules {
class CloudsDebugModule final : public DebugModule {
public:
    void RenderWindow() override;
    void RenderMenuEntry() override; 

private:
    bool m_IsOpen{};
};
}; // namespace debugmodules
}; // namespace notsa
