#include "StdInc.h"

#include "RunningScript.h"
#include "TheScripts.h"
#include "CarGenerator.h"
#include "Hud.h"
#include "spdlog/sinks/stdout_color_sinks.h"

static notsa::log_ptr logger;

//static auto logger = NOTSA_MAKE_LOGGER("script");

//! Define it to dump out all commands that don't have a custom handler (that is, they aren't reversed)
//! Makes compilation slow, so don't enable unless necessary!
//#define DUMP_CUSTOM_COMMAND_HANDLERS_TO_FILE

#ifdef DUMP_CUSTOM_COMMAND_HANDLERS_TO_FILE
#include <fstream>
#endif

#include "CommandParser/Parser.hpp"
#include "CommandParser/LUTGenerator.hpp"
#include "ReversibleHooks/ReversibleHook/ScriptCommand.h"

#include "Commands/Commands.hpp"
#ifdef NOTSA_USE_CLEO_COMMANDS // TODO: Add premake/cmake option for this define
#include "Commands/CLEO/Commands.hpp"
#include "Commands/CLEO/Extensions/Commands.hpp"
#endif

// https://library.sannybuilder.com/#/sa

//! Holds all custom command handlers (or null for commands with no custom handler)
static inline std::array<notsa::script::CommandHandlerFunction, (size_t)(COMMAND_HIGHEST_ID_TO_HOOK) + 1> s_CustomCommandHandlerTable{};

void CRunningScript::InjectHooks() {
    logger = NOTSA_MAKE_LOGGER("script");

    InjectCustomCommandHooks();

    RH_ScopedClass(CRunningScript);
    RH_ScopedCategory("Scripts");

    RH_ScopedInstall(Init, 0x4648E0);
    RH_ScopedInstall(GetCorrectPedModelIndexForEmergencyServiceType, 0x464F50);

    RH_ScopedInstall(PlayAnimScriptCommand, 0x470150, { .reversed = false });
    RH_ScopedInstall(LocateCarCommand, 0x487A20, { .reversed = false });
    RH_ScopedInstall(LocateCharCommand, 0x486D80, { .reversed = false });
    RH_ScopedInstall(LocateObjectCommand, 0x487D10, { .reversed = false });
    RH_ScopedInstall(LocateCharCarCommand, 0x487420, { .reversed = false });
    RH_ScopedInstall(LocateCharCharCommand, 0x4870F0, { .reversed = false });
    RH_ScopedInstall(LocateCharObjectCommand, 0x487720, { .reversed = false });
    RH_ScopedInstall(CarInAreaCheckCommand, 0x488EC0, { .reversed = false });
    RH_ScopedInstall(CharInAreaCheckCommand, 0x488B50, { .reversed = false });
    RH_ScopedInstall(ObjectInAreaCheckCommand, 0x489150, { .reversed = false });
    RH_ScopedInstall(CharInAngledAreaCheckCommand, 0x487F60, { .reversed = false });
    RH_ScopedInstall(FlameInAngledAreaCheckCommand, 0x488780, { .reversed = false });
    RH_ScopedInstall(ObjectInAngledAreaCheckCommand, 0x4883F0, { .reversed = false });
    RH_ScopedInstall(CollectParameters, 0x464080, { .stackArguments = 1 });
    RH_ScopedInstall(CollectNextParameterWithoutIncreasingPC, 0x464250, { .stackArguments = 0 });
    RH_ScopedInstall(StoreParameters, 0x464370, { .stackArguments = 1 });
    RH_ScopedInstall(ReadArrayInformation, 0x463CF0, { .stackArguments = 3 });
    RH_ScopedInstall(ReadParametersForNewlyStartedScript, 0x464500, { .stackArguments = 1 });
    RH_ScopedInstall(ReadTextLabelFromScript, 0x463D50, { .stackArguments = 2 });
    RH_ScopedInstall(GetIndexOfGlobalVariable, 0x464700, { .stackArguments = 0 });
    RH_ScopedInstall(GetPadState, 0x485B10);
    RH_ScopedInstall(GetPointerToLocalVariable, 0x463CA0, { .stackArguments = 1 });
    RH_ScopedInstall(GetPointerToLocalArrayElement, 0x463CC0, { .stackArguments = 3 });
    RH_ScopedInstall(GetPointerToScriptVariable, 0x464790, { .stackArguments = 1 });
    RH_ScopedInstall(DoDeathArrestCheck, 0x485A50);
    RH_ScopedInstall(SetCharCoordinates, 0x464DC0);
    RH_ScopedInstall(AddScriptToList, 0x464C00, { .stackArguments = 1 });
    RH_ScopedInstall(RemoveScriptFromList, 0x464BD0, { .stackArguments = 1 });
    RH_ScopedInstall(ShutdownThisScript, 0x465AA0, { .reversed = false });
    RH_ScopedInstall(IsPedDead, 0x464D70);
    RH_ScopedInstall(ThisIsAValidRandomPed, 0x489490);
    RH_ScopedInstall(ScriptTaskPickUpObject, 0x46AF50, { .reversed = false });
    RH_ScopedInstall(UpdateCompareFlag, 0x4859D0, { .stackArguments = 1 });
    RH_ScopedInstall(UpdatePC, 0x464DA0, { .stackArguments = 1 });
    RH_ScopedInstall(ProcessOneCommand, 0x469EB0);
    RH_ScopedInstall(Process, 0x469F00);
    RH_ScopedOverloadedInstall(GivePedScriptedTask, "OG", 0x465C20, void(CRunningScript::*)(int32, CTask*, int32));
}

//! Register our custom script command handlers
void CRunningScript::InjectCustomCommandHooks() {
    // Uncommenting any call will prevent it from being hooked, so
    // feel free to do so when debugging (Just don't forget to undo the changes!)

    using namespace notsa::script::commands;

    basic::RegisterHandlers();
    camera::RegisterHandlers();
    character::RegisterHandlers();
    clock::RegisterHandlers();
    comparasion::RegisterHandlers();
    game::RegisterHandlers();
    generic::RegisterHandlers();
    math::RegisterHandlers();
    mission::RegisterHandlers();
    object::RegisterHandlers();
    pad::RegisterHandlers();
    ped::RegisterHandlers();
    player::RegisterHandlers();
    script::RegisterHandlers();
    sequence::RegisterHandlers();
    text::RegisterHandlers();
    unused::RegisterHandlers();
    utility::RegisterHandlers();
    vehicle::RegisterHandlers();

#ifdef NOTSA_USE_CLEO_COMMANDS
    cleo::audiostream::RegisterHandlers();
    cleo::character::RegisterHandlers();
    cleo::dynamiclibrary::RegisterHandlers();
    cleo::fs::RegisterHandlers();
    cleo::game::RegisterHandlers();
    cleo::generic::RegisterHandlers();
    cleo::memory::RegisterHandlers();
    cleo::pad::RegisterHandlers();
    cleo::script::RegisterHandlers();
    cleo::vehicle::RegisterHandlers();
    cleo::world::RegisterHandlers();

    cleo::extensions::cleoplus::RegisterHandlers();
    cleo::extensions::clipboard::RegisterHandlers();
    cleo::extensions::fs::RegisterHandlers();
    cleo::extensions::imgui::RegisterHandlers();
    cleo::extensions::intoperations::RegisterHandlers();
#endif

    // To enable use premake: `./premake5.exe vs2022 --allow-script-cmd-hooks`
#ifdef ENABLE_SCRIPT_COMMAND_HOOKS
    // After injecting all hooks, we can create their reversible hook
    for (auto&& [idx, cmd] : notsa::enumerate(s_CustomCommandHandlerTable)) {
        const auto id = (eScriptCommands)(idx);

        ReversibleHooks::AddItemToCategory(
            "Scripts/Commands",
            std::make_shared<ReversibleHooks::ReversibleHook::ScriptCommand>(id)
        );
    }
#endif

#ifdef DUMP_CUSTOM_COMMAND_HANDLERS_TO_FILE
    auto reversed{0}, total{0};
    std::ofstream ofsrev{ "reversed_script_command_handlers.txt" }, ofsnotrev{ "NOT_reversed_script_command_handlers.txt" };
    for (auto&& [idx, handler] : notsa::enumerate(s_CustomCommandHandlerTable)) {
        const auto id = (eScriptCommands)(idx);
        ++total;
        if (handler) ++reversed;
        (handler ? ofsrev : ofsnotrev) << ::notsa::script::GetScriptCommandName(id) << '\n';
    }
    DEV_LOG("Script cmds dumped! Find them in `<GTA Directory>/Scripts`!");
    DEV_LOG("Script cmds reverse progress: {}/{} ({:.2f}% done)", reversed, total, 100.0f * ((float)reversed / (float)total));
#endif
}

// 0x4648E0
void CRunningScript::Init() {
    SetName("noname");
    m_pBaseIP = nullptr;
    m_pPrev = nullptr;
    m_pNext = nullptr;
    m_IP = nullptr;
    memset(m_IPStack, 0, sizeof(m_IPStack));
    m_StackDepth = 0;
    m_nWakeTime = 0;
    m_bIsActive = false;
    m_bCondResult = false;
    m_bUseMissionCleanup = false;
    m_bIsExternal = false;
    m_bTextBlockOverride = false;
    m_nExternalType = -1;
    memset(m_aLocalVars, 0, sizeof(m_aLocalVars));
    m_nLogicalOp = 0;
    m_bNotFlag = false;
    m_bDeathArrestExecuted = false;
    m_pSceneSkipIP = nullptr;
    m_bIsMission = false;
    m_bDeathArrestEnabled = true;
}

/*!
 * Adds script to list
 * @addr 0x464C00
 */
void CRunningScript::AddScriptToList(CRunningScript** queueList) {
    m_pNext = *queueList;
    m_pPrev = nullptr;
    if (*queueList)
        (*queueList)->m_pPrev = this;
    *queueList = this;
}

/*!
 * Removes script from list
 * @addr 0x464BD0
 */
void CRunningScript::RemoveScriptFromList(CRunningScript** queueList) {
    if (m_pPrev)
        m_pPrev->m_pNext = m_pNext;
    else
        *queueList = m_pNext;

    if (m_pNext)
        m_pNext->m_pPrev = m_pPrev;
}

/*!
 * Terminates a script
 * @addr 0x465AA0
 */
void CRunningScript::ShutdownThisScript() {
    return plugin::CallMethod<0x465AA0>(this);
    /*
    if (m_bIsExternal) {
        const auto idx = CTheScripts::StreamedScripts.GetStreamedScriptWithThisStartAddress(m_pBaseIP);
        CTheScripts::StreamedScripts.m_aScripts[idx].m_nStatus--;
    }

    switch (m_nExternalType) {
    case 0:
    case 2:
    case 3:
    case 5: {
        const auto pedRef = m_bIsMission
            ? CTheScripts::LocalVariablesForCurrentMission.front().iParam
            : m_aLocalVars[0].iParam;
        if (const auto ped = GetPedPool()->GetAtRef(pedRef)) {
            ped->bHasAScriptBrain = false;
            if (m_nExternalType == 5) {
                CScriptedBrainTaskStore::SetTask(ped, new CTaskSimpleFinishBrain{});
            }
        }
        break;
    }
    }
    */
}

// 0x465C20
void CRunningScript::GivePedScriptedTask(int32 pedHandle, CTask* task, int32 opcode) {
    if (pedHandle == -1) {
        CTaskSequences::AddTaskToActiveSequence(task);
        return;
    }

    CPed* ped = GetPedPool()->GetAtRef(pedHandle);
    assert(ped);
    CPedGroup* pedGroup = CPedGroups::GetPedsGroup(ped);

    CPed* otherPed = nullptr;
    if (m_nExternalType == 5 || m_nExternalType == 2 || !m_nExternalType || m_nExternalType == 3) {
        auto* localVariable = reinterpret_cast<int32*>(GetPointerToLocalVariable(0));
        otherPed = GetPedPool()->GetAtRef(*localVariable);
    }

    if (ped->bHasAScriptBrain && otherPed != ped) {
        delete task;
    } else if (otherPed && m_nExternalType == 5) {
        if (CScriptedBrainTaskStore::SetTask(ped, task)) {
            const int32 slot = CPedScriptedTaskRecord::GetVacantSlot();
            CPedScriptedTaskRecord::ms_scriptedTasks[slot].SetAsAttractorScriptTask(ped, opcode, task);
        }
    } else if (!pedGroup || ped->IsPlayer()) {
        CEventScriptCommand eventScriptCommand(TASK_PRIMARY_PRIMARY, task, false);
        auto* event = static_cast<CEventScriptCommand*>(ped->GetEventGroup().Add(&eventScriptCommand, false));
        if (event) {
            const int32 slot = CPedScriptedTaskRecord::GetVacantSlot();
            CPedScriptedTaskRecord::ms_scriptedTasks[slot].Set(ped, opcode, event);
        }
    } else {
        pedGroup->GetIntelligence().SetScriptCommandTask(ped, task);
        CTask* scriptedTask = pedGroup->GetIntelligence().GetTaskScriptCommand(ped);
        const int32 slot = CPedScriptedTaskRecord::GetVacantSlot();
        CPedScriptedTaskRecord::ms_scriptedTasks[slot].SetAsGroupTask(ped, opcode, scriptedTask);
        delete task;
    }
}

void CRunningScript::GivePedScriptedTask(CPed* ped, CTask* task, int32 opcode) {
    GivePedScriptedTask(GetPedPool()->GetRef(ped), task, opcode); // Must do it like this, otherwise unhooking of the original `GivePedScriptedTask` will do nothing
}

// 0x470150
void CRunningScript::PlayAnimScriptCommand(int32 commandId) {
    plugin::CallMethod<0x470150, CRunningScript*, int32>(this, commandId);
}

// 0x487A20
void CRunningScript::LocateCarCommand(int32 commandId) {
    plugin::CallMethod<0x487A20, CRunningScript*, int32>(this, commandId);
}

// 0x486D80
void CRunningScript::LocateCharCommand(int32 commandId) {
    plugin::CallMethod<0x486D80, CRunningScript*, int32>(this, commandId);
}

// 0x487D10
void CRunningScript::LocateObjectCommand(int32 commandId) {
    plugin::CallMethod<0x487D10, CRunningScript*, int32>(this, commandId);
}

// 0x487420
void CRunningScript::LocateCharCarCommand(int32 commandId) {
    plugin::CallMethod<0x487420, CRunningScript*, int32>(this, commandId);
}

// 0x4870F0
void CRunningScript::LocateCharCharCommand(int32 commandId) {
    plugin::CallMethod<0x4870F0, CRunningScript*, int32>(this, commandId);
}

// 0x487720
void CRunningScript::LocateCharObjectCommand(int32 commandId) {
    plugin::CallMethod<0x487720, CRunningScript*, int32>(this, commandId);
}

// 0x488EC0
void CRunningScript::CarInAreaCheckCommand(int32 commandId) {
    plugin::CallMethod<0x488EC0, CRunningScript*, int32>(this, commandId);
}

// 0x488B50
void CRunningScript::CharInAreaCheckCommand(int32 commandId) {
    plugin::CallMethod<0x488B50, CRunningScript*, int32>(this, commandId);
}

// 0x489150
void CRunningScript::ObjectInAreaCheckCommand(int32 commandId) {
    plugin::CallMethod<0x489150, CRunningScript*, int32>(this, commandId);
}

// 0x487F60
void CRunningScript::CharInAngledAreaCheckCommand(int32 commandId) {
    plugin::CallMethod<0x487F60, CRunningScript*, int32>(this, commandId);
}

// 0x488780
void CRunningScript::FlameInAngledAreaCheckCommand(int32 commandId) {
    plugin::CallMethod<0x488780, CRunningScript*, int32>(this, commandId);
}

// 0x4883F0
void CRunningScript::ObjectInAngledAreaCheckCommand(int32 commandId) {
    plugin::CallMethod<0x4883F0, CRunningScript*, int32>(this, commandId);
}

// 0x464D70
bool CRunningScript::IsPedDead(CPed* ped) const {
    ePedState pedState = ped->m_nPedState;
    return pedState == PEDSTATE_DEAD || pedState == PEDSTATE_DIE || pedState == PEDSTATE_DIE_BY_STEALTH;
}

// 0x489490
bool CRunningScript::ThisIsAValidRandomPed(ePedType pedType, bool civilian, bool gang, bool criminal) {
    switch (pedType) {
    case PED_TYPE_CIVMALE:
    case PED_TYPE_CIVFEMALE:
        return civilian;
    case PED_TYPE_GANG1:
    case PED_TYPE_GANG2:
    case PED_TYPE_GANG3:
    case PED_TYPE_GANG4:
    case PED_TYPE_GANG5:
    case PED_TYPE_GANG6:
    case PED_TYPE_GANG7:
    case PED_TYPE_GANG8:
    case PED_TYPE_GANG9:
        return gang;
    case PED_TYPE_CRIMINAL:
    case PED_TYPE_PROSTITUTE:
        return criminal;
    default:
        return false;
    }
}

// 0x485A50
void CRunningScript::DoDeathArrestCheck() {
    if (!m_bDeathArrestEnabled)
        return;

    if (!CTheScripts::IsPlayerOnAMission())
        return;

    auto& playerInfo = FindPlayerInfo();
    if (!playerInfo.IsRestartingAfterDeath() && !playerInfo.IsRestartingAfterArrest())
        return;

    // TODO/NOTE: This is buggy, it will decrease SP to 0, and then `--m_nSP` will underflow :D
    NOTSA_UNREACHABLE(); // Prevent random bugs
    if (m_StackDepth > 1u) { // todo: refactor
        do
            --m_StackDepth;
        while (m_StackDepth > 1u);
    }

    m_IP = m_IPStack[--m_StackDepth];
    CMessages::ClearSmallMessagesOnly();
    CTheScripts::ScriptSpace[CTheScripts::OnAMissionFlag] = 0;
    m_bDeathArrestExecuted = true;
    m_nWakeTime = 0;
}

// 0x464F50
void CRunningScript::GetCorrectPedModelIndexForEmergencyServiceType(ePedType pedType, uint32* typeSpecificModelId) {
    switch (*typeSpecificModelId) {
    case MODEL_LAPD1:
    case MODEL_SFPD1:
    case MODEL_LVPD1:
    case MODEL_LAPDM1:
        if (pedType == PED_TYPE_COP) {
            *typeSpecificModelId = COP_TYPE_CITYCOP;
        }
        break;
    case MODEL_CSHER:
        if (pedType == PED_TYPE_COP) {
            *typeSpecificModelId = COP_TYPE_CSHER;
        }
        break;
    case MODEL_SWAT:
        if (pedType == PED_TYPE_COP) {
            *typeSpecificModelId = COP_TYPE_SWAT1;
        }
        break;
    case MODEL_FBI:
        if (pedType == PED_TYPE_COP) {
            *typeSpecificModelId = COP_TYPE_FBI;
        }
        break;
    case MODEL_ARMY:
        if (pedType == PED_TYPE_COP) {
            *typeSpecificModelId = COP_TYPE_ARMY;
        }
        break;
    default:
        return;
    }
}

// Returns state of pad button
// 0x485B10
int16 CRunningScript::GetPadState(uint16 playerIndex, eButtonId buttonId) {
    const auto* pad = CPad::GetPad(playerIndex);
    switch (buttonId) {
    case BUTTON_LEFT_STICK_X:    return pad->NewState.LeftStickX;
    case BUTTON_LEFT_STICK_Y:    return pad->NewState.LeftStickY;
    case BUTTON_RIGHT_STICK_X:   return pad->NewState.RightStickX;
    case BUTTON_RIGHT_STICK_Y:   return pad->NewState.RightStickY;
    case BUTTON_LEFT_SHOULDER1:  return pad->NewState.LeftShoulder1;
    case BUTTON_LEFT_SHOULDER2:  return pad->NewState.LeftShoulder2;
    case BUTTON_RIGHT_SHOULDER1: return pad->NewState.RightShoulder1;
    case BUTTON_RIGHT_SHOULDER2: return pad->NewState.RightShoulder2;
    case BUTTON_DPAD_UP:         return pad->NewState.DPadUp;
    case BUTTON_DPAD_DOWN:       return pad->NewState.DPadDown;
    case BUTTON_DPAD_LEFT:       return pad->NewState.DPadLeft;
    case BUTTON_DPAD_RIGHT:      return pad->NewState.DPadRight;
    case BUTTON_START:           return pad->NewState.Start;
    case BUTTON_SELECT:          return pad->NewState.Select;
    case BUTTON_SQUARE:          return pad->NewState.ButtonSquare;
    case BUTTON_TRIANGLE:        return pad->NewState.ButtonTriangle;
    case BUTTON_CROSS:           return pad->NewState.ButtonCross;
    case BUTTON_CIRCLE:          return pad->NewState.ButtonCircle;
    case BUTTON_LEFTSHOCK:       return pad->NewState.ShockButtonL;
    case BUTTON_RIGHTSHOCK:      return pad->NewState.ShockButtonR;
    default:                     return OR_CONTINUE;
    }
}

// 0x46AF50
void CRunningScript::ScriptTaskPickUpObject(int32 commandId) {
    plugin::CallMethod<0x46AF50, CRunningScript*, int32>(this, commandId);
}

// 0x464DC0
void CRunningScript::SetCharCoordinates(CPed& ped, CVector posn, bool warpGang, bool offset) {
    CWorld::PutToGroundIfTooLow(posn);

    CVehicle* vehicle = ped.bInVehicle ? ped.m_pVehicle : nullptr;
    if (vehicle) {
        posn.z += vehicle->GetDistanceFromCentreOfMassToBaseOfModel();
        vehicle->Teleport(posn, false);
        CTheScripts::ClearSpaceForMissionEntity(&posn, vehicle);
    } else {
        posn.z += offset ? ped.GetDistanceFromCentreOfMassToBaseOfModel() : 0.0f;
        CTheScripts::ClearSpaceForMissionEntity(&posn, &ped);
        auto* group = CPedGroups::GetPedsGroup(&ped);
        if (group && group->GetMembership().IsLeader(&ped) && warpGang) {
            group->Teleport(&posn);
        } else {
            ped.Teleport(posn, false);
        }
    }
}

// 0x463CA0
tScriptParam* CRunningScript::GetPointerToLocalVariable(int32 varIndex) {
    if (m_bIsMission)
        return reinterpret_cast<tScriptParam*>(&CTheScripts::LocalVariablesForCurrentMission[varIndex]);
    else
        return reinterpret_cast<tScriptParam*>(&m_aLocalVars[varIndex]);
}

/*!
 * @addr 0x463CC0
 * @brief Returns pointer to a local script variable.
 *
 * @param arrayBaseOffset  The offset of the array (In terms of the number of `tScriptParam`s before it)
 * @param index            Index of the variable inside the array
 * @param arrayEntriesSize Size of 1 variable in the array (In terms of `tScriptParam`'s - So for a regular `int` (or float, etc) variable this will be `1`, for long strings it's `4` and for short one's it's `2`)
 */
tScriptParam* CRunningScript::GetPointerToLocalArrayElement(int32 arrayBaseOffset, uint16 index, uint8 arrayEntriesSize) {
    return GetPointerToLocalVariable(arrayBaseOffset + arrayEntriesSize * index);
}

/*!
 * Returns pointer to script variable of any type.
 * @addr 0x464790
 */
tScriptParam* CRunningScript::GetPointerToScriptVariable(eScriptVariableType) {
    uint8  arrElemSize;
    uint16 arrVarOffset;
    int32  arrElemIdx;

    int8 type = CTheScripts::Read1ByteFromScript(m_IP);
    switch (type) {
    case SCRIPT_PARAM_GLOBAL_NUMBER_VARIABLE:
    case SCRIPT_PARAM_GLOBAL_SHORT_STRING_VARIABLE:
    case SCRIPT_PARAM_GLOBAL_LONG_STRING_VARIABLE:
    {
        uint16 index = CTheScripts::Read2BytesFromScript(m_IP);
        return reinterpret_cast<tScriptParam*>(&CTheScripts::ScriptSpace[index]);
    }
    case SCRIPT_PARAM_LOCAL_NUMBER_VARIABLE:
    case SCRIPT_PARAM_LOCAL_SHORT_STRING_VARIABLE:
    case SCRIPT_PARAM_LOCAL_LONG_STRING_VARIABLE:
    {
        uint16 index = CTheScripts::Read2BytesFromScript(m_IP);
        return GetPointerToLocalVariable(index);
    }

    case SCRIPT_PARAM_GLOBAL_NUMBER_ARRAY:
    case SCRIPT_PARAM_GLOBAL_SHORT_STRING_ARRAY:
    case SCRIPT_PARAM_GLOBAL_LONG_STRING_ARRAY:
        ReadArrayInformation(true, &arrVarOffset, &arrElemIdx);
        if (type == SCRIPT_PARAM_GLOBAL_LONG_STRING_ARRAY)
            return reinterpret_cast<tScriptParam*>(&CTheScripts::ScriptSpace[LONG_STRING_SIZE * arrElemIdx + arrVarOffset]);
        else if (type == SCRIPT_PARAM_GLOBAL_SHORT_STRING_ARRAY)
            return reinterpret_cast<tScriptParam*>(&CTheScripts::ScriptSpace[SHORT_STRING_SIZE * arrElemIdx + arrVarOffset]);
        else // SCRIPT_PARAM_GLOBAL_NUMBER_ARRAY
            return reinterpret_cast<tScriptParam*>(&CTheScripts::ScriptSpace[4 * arrElemIdx + arrVarOffset]);

    case SCRIPT_PARAM_LOCAL_NUMBER_ARRAY:
    case SCRIPT_PARAM_LOCAL_SHORT_STRING_ARRAY:
    case SCRIPT_PARAM_LOCAL_LONG_STRING_ARRAY:
        ReadArrayInformation(true, &arrVarOffset, &arrElemIdx);
        if (type == SCRIPT_PARAM_LOCAL_LONG_STRING_ARRAY)
            arrElemSize = 4;
        else if (type == SCRIPT_PARAM_LOCAL_SHORT_STRING_ARRAY)
            arrElemSize = 2;
        else // SCRIPT_PARAM_LOCAL_NUMBER_ARRAY
            arrElemSize = 1;
        return GetPointerToLocalArrayElement(arrVarOffset, arrElemIdx, arrElemSize);

    default:
        NOTSA_UNREACHABLE();
    }
}

/*!
 * Returns offset of global variable
 * @addr 0x464700
 */
uint16 CRunningScript::GetIndexOfGlobalVariable() {
    uint16 arrVarOffset;
    int32  arrElemIdx;

    switch (CTheScripts::Read1ByteFromScript(m_IP)) {
    case SCRIPT_PARAM_GLOBAL_NUMBER_VARIABLE:
        return CTheScripts::Read2BytesFromScript(m_IP);
    case SCRIPT_PARAM_GLOBAL_NUMBER_ARRAY:
        ReadArrayInformation(true, &arrVarOffset, &arrElemIdx);
        return arrVarOffset + 4 * arrElemIdx;
    default:
        // todo: ???
        return (uint16)(uint32)this;
    }
}

// 0x464080
void CRunningScript::CollectParameters(int16 count) {
    uint16 arrVarOffset;
    int32  arrElemIdx;

    for (auto i = 0; i < count; i++) {
        switch (CTheScripts::Read1ByteFromScript(m_IP)) {
        case SCRIPT_PARAM_STATIC_INT_32BITS:
            ScriptParams[i].iParam = CTheScripts::Read4BytesFromScript(m_IP);
            break;
        case SCRIPT_PARAM_GLOBAL_NUMBER_VARIABLE:
        {
            uint16 index = CTheScripts::Read2BytesFromScript(m_IP);
            ScriptParams[i].iParam = *reinterpret_cast<int32*>(&CTheScripts::ScriptSpace[index]);
            break;
        }
        case SCRIPT_PARAM_LOCAL_NUMBER_VARIABLE:
        {
            uint16 index = CTheScripts::Read2BytesFromScript(m_IP);
            ScriptParams[i] = *GetPointerToLocalVariable(index);
            break;
        }
        case SCRIPT_PARAM_STATIC_INT_8BITS:
            ScriptParams[i].iParam = CTheScripts::Read1ByteFromScript(m_IP);
            break;
        case SCRIPT_PARAM_STATIC_INT_16BITS:
            ScriptParams[i].iParam = CTheScripts::Read2BytesFromScript(m_IP);
            break;
        case SCRIPT_PARAM_STATIC_FLOAT:
            ScriptParams[i].fParam = CTheScripts::ReadFloatFromScript(m_IP);
            break;
        case SCRIPT_PARAM_GLOBAL_NUMBER_ARRAY:
            ReadArrayInformation(true, &arrVarOffset, &arrElemIdx);
            ScriptParams[i].iParam = *reinterpret_cast<int32*>(&CTheScripts::ScriptSpace[arrVarOffset + 4 * arrElemIdx]);
            break;
        case SCRIPT_PARAM_LOCAL_NUMBER_ARRAY:
            ReadArrayInformation(true, &arrVarOffset, &arrElemIdx);
            ScriptParams[i] = *GetPointerToLocalArrayElement(arrVarOffset, arrElemIdx, 1);
            break;
        }
    }
}

/*!
 * Collects parameter and returns it.
 * @addr 0x464250
 */
int32 CRunningScript::CollectNextParameterWithoutIncreasingPC() {
    uint16 arrVarOffset;
    int32  arrElemIdx;
    uint8* ip = m_IP;
    int32  result = -1;

    switch (CTheScripts::Read1ByteFromScript(m_IP)) {
    case SCRIPT_PARAM_STATIC_INT_32BITS:
    case SCRIPT_PARAM_STATIC_FLOAT:
        result = CTheScripts::Read4BytesFromScript(m_IP);
        break;
    case SCRIPT_PARAM_GLOBAL_NUMBER_VARIABLE:
    {
        uint16 index = CTheScripts::Read2BytesFromScript(m_IP);
        result = *reinterpret_cast<int32*>(&CTheScripts::ScriptSpace[index]);
        break;
    }
    case SCRIPT_PARAM_LOCAL_NUMBER_VARIABLE:
    {
        uint16 index = CTheScripts::Read2BytesFromScript(m_IP);
        result = GetPointerToLocalVariable(index)->iParam;
        break;
    }
    case SCRIPT_PARAM_STATIC_INT_8BITS:
        result = CTheScripts::Read1ByteFromScript(m_IP);
        break;
    case SCRIPT_PARAM_STATIC_INT_16BITS:
        result = CTheScripts::Read2BytesFromScript(m_IP);
        break;
    case SCRIPT_PARAM_GLOBAL_NUMBER_ARRAY:
        ReadArrayInformation(false, &arrVarOffset, &arrElemIdx);
        result = *reinterpret_cast<int32*>(&CTheScripts::ScriptSpace[arrVarOffset + 4 * arrElemIdx]);
        break;
    case SCRIPT_PARAM_LOCAL_NUMBER_ARRAY:
        ReadArrayInformation(false, &arrVarOffset, &arrElemIdx);
        result = GetPointerToLocalArrayElement(arrVarOffset, arrElemIdx, 1)->iParam;
        break;
    }

    m_IP = ip;
    return result;
}

/*!
 * @addr 0x464370
 */
void CRunningScript::StoreParameters(int16 count) {
    uint16 arrVarOffset;
    int32  arrElemIdx;

    for (auto i = 0; i < count; i++) {
        switch (CTheScripts::Read1ByteFromScript(m_IP)) {
        case SCRIPT_PARAM_GLOBAL_NUMBER_VARIABLE:
        {
            uint16 index = CTheScripts::Read2BytesFromScript(m_IP);
            *reinterpret_cast<int32*>(&CTheScripts::ScriptSpace[index]) = ScriptParams[i].iParam;
            break;
        }
        case SCRIPT_PARAM_LOCAL_NUMBER_VARIABLE:
        {
            uint16 index = CTheScripts::Read2BytesFromScript(m_IP);
            *GetPointerToLocalVariable(index) = ScriptParams[i];
            break;
        }
        case SCRIPT_PARAM_GLOBAL_NUMBER_ARRAY:
            ReadArrayInformation(true, &arrVarOffset, &arrElemIdx);
            *reinterpret_cast<int32*>(&CTheScripts::ScriptSpace[arrVarOffset + 4 * arrElemIdx]) = ScriptParams[i].iParam;
            break;
        case SCRIPT_PARAM_LOCAL_NUMBER_ARRAY:
            ReadArrayInformation(true, &arrVarOffset, &arrElemIdx);
            *GetPointerToLocalArrayElement(arrVarOffset, arrElemIdx, 1) = ScriptParams[i];
            break;
        }
    }
}

// Reads array var base offset and element index from index variable.
// 0x463CF0
void CRunningScript::ReadArrayInformation(int32 updateIp, uint16* outArrVarOffset, int32* outArrElemIdx) {
    auto* ip = m_IP;

    *outArrVarOffset      = CTheScripts::Read2BytesFromScript(ip);
    uint16 arrayIndexVar  = CTheScripts::Read2BytesFromScript(ip);
    bool isGlobalIndexVar = CTheScripts::Read2BytesFromScript(ip) < 0; // high bit set

    if (isGlobalIndexVar)
        *outArrElemIdx = *reinterpret_cast<int32*>(&CTheScripts::ScriptSpace[arrayIndexVar]);
    else
        *outArrElemIdx = GetPointerToLocalVariable(arrayIndexVar)->iParam;

    if (updateIp)
        m_IP = ip;
}

// Collects parameters and puts them to local variables of new script
// 0x464500
void CRunningScript::ReadParametersForNewlyStartedScript(CRunningScript* newScript) {
    uint16 arrVarOffset;
    int32  arrElemIdx;
    int8   type = CTheScripts::Read1ByteFromScript(m_IP);

    for (int i = 0; type != SCRIPT_PARAM_END_OF_ARGUMENTS; type = CTheScripts::Read1ByteFromScript(m_IP), i++) {
        switch (type) {
        case SCRIPT_PARAM_STATIC_INT_32BITS:
            newScript->m_aLocalVars[i].iParam = CTheScripts::Read4BytesFromScript(m_IP);
            break;
        case SCRIPT_PARAM_GLOBAL_NUMBER_VARIABLE:
        {
            uint16 index = CTheScripts::Read2BytesFromScript(m_IP);
            newScript->m_aLocalVars[i].iParam = *reinterpret_cast<int32*>(&CTheScripts::ScriptSpace[index]);
            break;
        }
        case SCRIPT_PARAM_LOCAL_NUMBER_VARIABLE:
        {
            uint16 index = CTheScripts::Read2BytesFromScript(m_IP);
            newScript->m_aLocalVars[i] = *GetPointerToLocalVariable(index);
            break;
        }
        case SCRIPT_PARAM_STATIC_INT_8BITS:
            newScript->m_aLocalVars[i].iParam = CTheScripts::Read1ByteFromScript(m_IP);
            break;
        case SCRIPT_PARAM_STATIC_INT_16BITS:
            newScript->m_aLocalVars[i].iParam = CTheScripts::Read2BytesFromScript(m_IP);
            break;
        case SCRIPT_PARAM_STATIC_FLOAT:
            newScript->m_aLocalVars[i].fParam = CTheScripts::ReadFloatFromScript(m_IP);
            break;
        case SCRIPT_PARAM_GLOBAL_NUMBER_ARRAY:
            ReadArrayInformation(true, &arrVarOffset, &arrElemIdx);
            newScript->m_aLocalVars[i].iParam = *reinterpret_cast<int32*>(&CTheScripts::ScriptSpace[arrVarOffset + 4 * arrElemIdx]);
            break;
        case SCRIPT_PARAM_LOCAL_NUMBER_ARRAY:
            ReadArrayInformation(true, &arrVarOffset, &arrElemIdx);
            newScript->m_aLocalVars[i] = *GetPointerToLocalArrayElement(arrVarOffset, arrElemIdx, 1);
            break;
        default:
            break;
        }
    }
}

// Collects string parameter
// 0x463D50
void CRunningScript::ReadTextLabelFromScript(char* buffer, uint8 nBufferLength) {
    uint16 arrVarOffset;
    int32  arrElemIdx;

    int8 type = CTheScripts::Read1ByteFromScript(m_IP);
    switch (type) {
    case SCRIPT_PARAM_STATIC_SHORT_STRING:
        for (auto i = 0; i < SHORT_STRING_SIZE; i++)
            buffer[i] = CTheScripts::Read1ByteFromScript(m_IP);
        break;

    case SCRIPT_PARAM_GLOBAL_SHORT_STRING_VARIABLE:
    {
        uint16 index = CTheScripts::Read2BytesFromScript(m_IP);
        strncpy_s(buffer, SHORT_STRING_SIZE, (char*)&CTheScripts::ScriptSpace[index], SHORT_STRING_SIZE);
        break;
    }

    case SCRIPT_PARAM_LOCAL_SHORT_STRING_VARIABLE:
    {
        uint16 index = CTheScripts::Read2BytesFromScript(m_IP);
        strncpy_s(buffer, SHORT_STRING_SIZE, (char*) GetPointerToLocalVariable(index), SHORT_STRING_SIZE);
        break;
    }

    case SCRIPT_PARAM_GLOBAL_SHORT_STRING_ARRAY:
    case SCRIPT_PARAM_GLOBAL_LONG_STRING_ARRAY:
        ReadArrayInformation(true, &arrVarOffset, &arrElemIdx);
        if (type == SCRIPT_PARAM_GLOBAL_SHORT_STRING_ARRAY)
            strncpy_s(buffer, SHORT_STRING_SIZE, (char*) & CTheScripts::ScriptSpace[SHORT_STRING_SIZE * arrElemIdx + arrVarOffset], SHORT_STRING_SIZE);
        else
            strncpy_s(buffer, SHORT_STRING_SIZE, (char*) & CTheScripts::ScriptSpace[LONG_STRING_SIZE * arrElemIdx + arrVarOffset], std::min<uint8>(nBufferLength, LONG_STRING_SIZE));
        break;

    case SCRIPT_PARAM_LOCAL_SHORT_STRING_ARRAY:
    case SCRIPT_PARAM_LOCAL_LONG_STRING_ARRAY:
        ReadArrayInformation(true, &arrVarOffset, &arrElemIdx);
        if (type == SCRIPT_PARAM_LOCAL_SHORT_STRING_ARRAY)
            strncpy_s(buffer, SHORT_STRING_SIZE, (char*) GetPointerToLocalArrayElement(arrVarOffset, arrElemIdx, 2), SHORT_STRING_SIZE);
        else {
            const auto bufferLength = std::min<uint8>(nBufferLength, LONG_STRING_SIZE);
            strncpy_s(buffer, bufferLength, (char*)GetPointerToLocalArrayElement(arrVarOffset, arrElemIdx, 4), bufferLength);
        }
        break;

    case SCRIPT_PARAM_STATIC_PASCAL_STRING:
    {
        int16 nStringLen = CTheScripts::Read1ByteFromScript(m_IP); // sign extension. max size = 127, not 255
        for (auto i = 0; i < nStringLen; i++)
            buffer[i] = CTheScripts::Read1ByteFromScript(m_IP);

        if (nStringLen < nBufferLength)
            memset(&buffer[(uint8)nStringLen], 0, (uint8)(nBufferLength - nStringLen));
        break;
    }

    case SCRIPT_PARAM_STATIC_LONG_STRING:
        // slightly changed code: original code is a bit messy and calls Read1ByteFromScript
        // in a loop and does some additional checks to ensure that buffer can hold the data
        strncpy_s(buffer, LONG_STRING_SIZE, (char*)m_IP, std::min<uint8>(nBufferLength, LONG_STRING_SIZE));
        m_IP += LONG_STRING_SIZE;
        break;

    case SCRIPT_PARAM_GLOBAL_LONG_STRING_VARIABLE:
    {
        uint16 index = CTheScripts::Read2BytesFromScript(m_IP);
        strncpy_s(buffer, LONG_STRING_SIZE, (char*) & CTheScripts::ScriptSpace[index], std::min<uint8>(nBufferLength, LONG_STRING_SIZE));
        break;
    }

    case SCRIPT_PARAM_LOCAL_LONG_STRING_VARIABLE:
    {
        uint16 index = CTheScripts::Read2BytesFromScript(m_IP);
        strncpy_s(buffer, LONG_STRING_SIZE, (char*) GetPointerToLocalVariable(index), std::min<uint8>(nBufferLength, LONG_STRING_SIZE));
        break;
    }

    default:
        break;
    }
}

// Updates comparement flag, used in conditional commands
// 0x4859D0
void CRunningScript::UpdateCompareFlag(bool state) {
    if (m_bNotFlag)
        state = !state;

    if (m_nLogicalOp == ANDOR_NONE) {
        m_bCondResult = state;
        return;
    }

    if (m_nLogicalOp >= ANDS_1 && m_nLogicalOp <= ANDS_8) {
        m_bCondResult &= state;
        if (m_nLogicalOp == ANDS_1)
            m_nLogicalOp = ANDOR_NONE;
        else
            m_nLogicalOp--;

        return;
    }

    if (m_nLogicalOp >= ORS_1 && m_nLogicalOp <= ORS_8) {
        m_bCondResult |= state;
        if (m_nLogicalOp == ORS_1)
            m_nLogicalOp = ANDOR_NONE;
        else
            m_nLogicalOp--;

        return;
    }
}

// Sets instruction pointer, used in GOTO-like commands
// 0x464DA0
void CRunningScript::UpdatePC(int32 newIP) {
    if (newIP >= 0)
        m_IP = &CTheScripts::ScriptSpace[newIP];
    else
        m_IP = m_pBaseIP + std::abs(newIP);
}

// 0x469EB0, inlined
OpcodeResult CRunningScript::ProcessOneCommand() {
    ++CTheScripts::CommandsExecuted;

    union {
        int16 op;
        struct {
            uint16 command : 15;
            uint16 notFlag : 1;
        };
    } op = { CTheScripts::Read2BytesFromScript(m_IP) };

    SPDLOG_LOGGER_TRACE(logger, "[{}][IP: {:#x} + {:#x}]: {} [{:#x}]", m_szName, LOG_PTR(m_pBaseIP), LOG_PTR(m_IP - m_pBaseIP), notsa::script::GetScriptCommandName((eScriptCommands)op.command), (size_t)op.command);
    
    m_bNotFlag = op.notFlag;

    if (const auto handler = CustomCommandHandlerOf((eScriptCommands)(op.command))) {
        return std::invoke(handler, this);
    } else {
        return std::invoke(s_OriginalCommandHandlerTable[(size_t)op.command / 100], this, (eScriptCommands)(op.command));
    }
}

// 0x469F00
OpcodeResult CRunningScript::Process() {
    if (m_pSceneSkipIP && CCutsceneMgr::IsCutsceneSkipButtonBeingPressed()) {
        CHud::m_BigMessage[1][0] = 0;
        UpdatePC(reinterpret_cast<int32>(m_pSceneSkipIP));
        m_pSceneSkipIP = nullptr;
        m_nWakeTime = 0;
    }

    if (m_bUseMissionCleanup)
        DoDeathArrestCheck();

    if (m_bIsMission && CTheScripts::FailCurrentMission == 1) {
        while (m_StackDepth > 1) // // todo: refactor | inline(?): while (stack.size > 1) { stack.pop() }
            --m_StackDepth;

        if (m_StackDepth == 1) {
            m_StackDepth = 0;
            m_IP = m_IPStack[0];
        }
    }
    CTheScripts::ReinitialiseSwitchStatementData();
    if (CTimer::GetTimeInMS() >= (uint32)m_nWakeTime) {
        while (ProcessOneCommand() == OR_CONTINUE); // Process commands
    }

    return OR_CONTINUE;
}

void CRunningScript::HighlightImportantArea(CVector2D from, CVector2D to, float z) {
    CTheScripts::HighlightImportantArea(reinterpret_cast<int32>(this) + reinterpret_cast<int32>(m_IP), from.x, from.y, to.x, to.y, z);
}

void CRunningScript::HighlightImportantArea(CRect area, float z) {
    HighlightImportantArea(area.GetTopLeft(), area.GetBottomRight(), z);
}

void CRunningScript::HighlightImportantArea(CVector from, CVector to) {
    HighlightImportantArea(CVector2D{ from }, CVector2D{ to }, (from.z + to.z) / 2.f);
}

void CRunningScript::HighlightImportantAngledArea(uint32 id, CVector2D a, CVector2D b, CVector2D c, CVector2D d) {
    NOTSA_UNREACHABLE(); // Fuck this, we dont need it!
}

notsa::script::CommandHandlerFunction& CRunningScript::CustomCommandHandlerOf(eScriptCommands command) {
    return s_CustomCommandHandlerTable[(size_t)(command)];
}
