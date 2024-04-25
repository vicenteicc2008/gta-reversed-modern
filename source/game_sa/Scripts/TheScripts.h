/*
    Plugin-SDK file
    Authors: GTA Community. See more here
    https://github.com/DK22Pac/plugin-sdk
    Do not delete this comment block. Respect others' work!
*/
#pragma once

#include "Text/Text.h"
#include "RunningScript.h"
#include "Ped.h"
#include "Vehicle.h"
#include "Object.h"
#include "MissionCleanup.h"
#include "Building.h"
#include "Sprite2d.h"
#include "StreamedScripts.h"
#include "ScriptResourceManager.h"
#include "StuckCarCheck.h"
#include "UpsideDownCarCheck.h"
#include "ScriptsForBrains.h"

class CCheckpoint;

enum class eCrossHairType : uint32 {
    NONE,
    FIXED_DRAW_CIRCLE,
    FIXED_DRAW_1STPERSON_WEAPON,
};

enum eScriptThingType : uint8 {
    SCRIPT_THING_SPHERE         = 0,
    SCRIPT_THING_EFFECT_SYSTEM  = 1,
    SCRIPT_THING_SEARCH_LIGHT   = 2,
    SCRIPT_THING_CHECKPOINT     = 3,
    SCRIPT_THING_SEQUENCE_TASK  = 4,
    SCRIPT_THING_FIRE           = 5,
    SCRIPT_THING_2D_EFFECT      = 6,
    SCRIPT_THING_DECISION_MAKER = 7,
    SCRIPT_THING_PED_GROUP      = 8
};

struct tBuildingSwap {
    CBuilding* m_pCBuilding;
    int32      m_nNewModelIndex;
    int32      m_nOldModelIndex;

    tBuildingSwap() { // 0x469270
        Clear();
    }

    tBuildingSwap(CBuilding* building, int32 nNewModelIndex, int32 nOldModelIndex) {
        m_pCBuilding     = building;
        m_nNewModelIndex = nNewModelIndex;
        m_nOldModelIndex = nOldModelIndex;
    }

    void Clear() { // todo: +-
        m_pCBuilding     = nullptr;
        m_nNewModelIndex = -1;
        m_nOldModelIndex = -1;
    }
};

struct tScriptSwitchCase {
    int32 m_nSwitchValue;
    int32 m_nSwitchLabelAddress;
};
VALIDATE_SIZE(tScriptSwitchCase, 0x8);

struct tScriptCheckpoint {
    bool         m_bUsed;
    char         m_field_1;
    int16        m_nId;
    CCheckpoint* m_Checkpoint;

    tScriptCheckpoint() { // 0x469334
        m_bUsed      = false;
        m_nId        = 1;
        m_Checkpoint = nullptr;
    }

    //! Get script thing ID
    auto GetId()    const { return m_nId; }

    //! If `*this` is currently in use
    auto IsActive() const { return m_bUsed; }
};
VALIDATE_SIZE(tScriptCheckpoint, 0x8);

struct tScriptEffectSystem {
    bool        m_bUsed;
    int16       m_nId;
    FxSystem_c* m_pFxSystem;

    tScriptEffectSystem() {
        m_bUsed     = false;
        m_nId       = 1;
        m_pFxSystem = nullptr;
    }

    //! Get script thing ID
    auto GetId()    const { return m_nId; }

    //! If `*this` is currently in use
    auto IsActive() const { return m_bUsed; }
};
VALIDATE_SIZE(tScriptEffectSystem, 0x8);

struct tScriptSequence {
    bool  m_bUsed;
    int16 m_nId;

    tScriptSequence() {
        m_bUsed = false;
        m_nId   = 1;
    }

    //! Get script thing ID
    auto GetId()    const { return m_nId; }

    //! If `*this` is currently in use
    auto IsActive() const { return m_bUsed; }
};
VALIDATE_SIZE(tScriptSequence, 0x4);

struct tScriptText {
    // values from 0x4690A8
    float     m_fLetterWidth{ 0.48f };
    float     m_fLetterHeight{ 1.12f };
    CRGBA     m_Color{ 225, 225, 225, 255 };
    bool      m_bJustify{ false };
    bool      m_bCentered{ false };
    bool      m_bWithBackground{ false };
    bool      m_bUnk{ false };
    float     m_fLineHeight{ SCREEN_HEIGHT };
    float     m_fLineWidth{ SCREEN_WIDTH };
    CRGBA     m_BackgroundBoxColor{ 128, 128, 128, 128 };
    bool      m_bProportional{ true };
    CRGBA     m_BackgroundColor{ 0, 0, 0, 255 };
    int8      m_nShadowType{ 2 };
    int8      m_nOutlineType{ 0 };
    bool      m_bDrawBeforeFade{ false };
    bool      m_bRightJustify{ false };
    int32     m_nFont{ 1 };
    CVector2D m_Pos{};
    char      m_szGxtEntry[8]{};
    int32     param1{ -1 };
    int32     param2{ -1 };
};
VALIDATE_SIZE(tScriptText, 0x44);

struct tScriptRectangle {
    int32     m_nType;
    bool      m_bDrawBeforeFade;
    char      field_5;
    int16     m_nTextureId;
    CVector2D cornerA;
    CVector2D cornerB;
    int32     m_nAngle;
    CRGBA     m_nTransparentColor;
    char      gxt[8];
    int32     field_28;
    int32     field_2C;
    int32     field_30;
    int32     field_34;
    uint32    m_nTextboxStyle;

    tScriptRectangle() { // 0x4691C8
        m_nType             = 0;
        m_bDrawBeforeFade   = false;
        m_nTextureId        = -1;
        cornerA             = CVector2D();
        cornerB             = CVector2D();
        m_nAngle            = 0;
        m_nTransparentColor = CRGBA(255, 255, 255, 255);
        gxt[0]              = 0;
        m_nTextboxStyle     = 3;
    }
};
VALIDATE_SIZE(tScriptRectangle, 0x3C);

struct tScriptAttachedAnimGroup {
    int32 m_nModelID;
    char  m_IfpName[16];

    tScriptAttachedAnimGroup() {
        m_nModelID = -1;
        m_IfpName[0] = 0;
    }
};

struct tScriptSearchlight {
    bool     m_bUsed{true};
    char     m_field_1{}; // unk flag; m_bNotScriptedLight ?
    bool     m_bEnableShadow{};
    uint8    m_nFlags{};
    int16    m_nId{1};
    CVector  m_Origin{};
    CVector  m_Target{};
    float    m_fTargetRadius{};
    float    m_fBaseRadius{};
    CVector  m_PathCoord1{};
    CVector  m_PathCoord2{};
    float    m_fPathSpeed{};
    CEntity* m_AttachedEntity{};
    CEntity* m_FollowingEntity{};
    CEntity* m_Tower{};
    CEntity* m_Housing{};
    CEntity* m_Bulb{};
    CVector  m_TargetSpot{};
    CVector  vf64{};
    CVector  vf70{};

    //! Script thing ID
    auto GetId() { return m_nId; }

    //! If `*this` is currently in use
    auto IsActive() const { return m_bUsed; }
};
VALIDATE_SIZE(tScriptSearchlight, 0x7C);

struct tUsedObject {
    char  szModelName[24];
    int32 nModelIndex;

    tUsedObject() = default; // 0x468F20
};
VALIDATE_SIZE(tUsedObject, 0x1C);

struct tScriptSphere {
    bool    m_bUsed;
    char    m_f1;
    int16   m_nUniqueId;
    uint32  m_nId;
    CVector m_vCoords;
    float   m_fRadius;

    tScriptSphere() { // 0x469060
        m_vCoords   = CVector();
        m_bUsed     = false;
        m_nUniqueId = 1;
        m_nId       = 0;
        m_fRadius   = 0.0f;
    }

    //! Get script thing ID
    auto GetId()    const { return m_nUniqueId; }

    //! If `*this` is currently in use
    auto IsActive() const { return m_bUsed; }
};
VALIDATE_SIZE(tScriptSphere, 0x18);

struct tStoredLine {
    CVector vecInf;
    CVector vecSup;
    uint32  color1;
    uint32  color2;
};
VALIDATE_SIZE(tStoredLine, 0x20);

struct tScriptBrainWaitEntity {
    CEntity* m_pEntity;
    int16    m_nSpecialModelIndex;
    int16    field_6;

    tScriptBrainWaitEntity() { // 0x468E12
        m_pEntity            = nullptr;
        m_nSpecialModelIndex = -1;
    }
};
VALIDATE_SIZE(tScriptBrainWaitEntity, 0x8);

struct tScriptConnectLodsObject {
    int32 a;
    int32 b;

    tScriptConnectLodsObject() {
        a = -1;
        b = -1;
    }
};
VALIDATE_SIZE(tScriptBrainWaitEntity, 0x8);

enum {
    MAX_NUM_SCRIPTS                             = 96,
    MAX_NUM_SCRIPT_SPRITES                      = 128,
    MAX_NUM_SCRIPT_SPHERES                      = 16,
    MAX_NUM_SCRIPT_RECTANGLES                   = 128,
    MAX_NUM_SCRIPT_SEARCH_LIGHT                 = 8,
    MAX_NUM_SCRIPT_SEQUENCE_TASKS               = 64,
    MAX_NUM_SCRIPT_CHECKPOINTS                  = 20,
    MAX_NUM_SCRIPT_EFFECT_SYSTEMS               = 32,
    MAX_NUM_SCRIPT_CONNECT_LODS_OBJECTS         = 20,
    MAX_NUM_SCRIPT_ATTACHED_ANIM_GROUPS         = 8,
    MAX_NUM_ENTITIES_WAITING_FOR_SCRIPT_BRAIN   = 150,
    MAX_NUM_VEHICLE_MODELS_BLOCKED_BY_SCRIPT    = 20,
    MAX_NUM_MISSION_SCRIPTS                     = 200,
    MAX_NUM_LOCAL_VARIABLES_FOR_CURRENT_MISSION = 1024,
    MAX_NUM_USED_OBJECTS                        = 395,
    MAX_NUM_BUILDING_SWAPS                      = 25,
    MAX_NUM_INVISIBILITY_SETTINGS               = 20,
    MAX_NUM_INTRO_TEXT_LINES                    = 96,
    MAX_NUM_STORED_LINES                        = 1024,
    MAX_NUM_SwitchJumpTable                     = 75,
    MAX_NUM_CARDS                               = 312,
    MAX_NUM_SUPPRESSED_VEHICLE_MODELS           = 40,
};

static constexpr uint32 MISSION_SCRIPT_SIZE = 69000;
class CTheScripts {
public:
    static constexpr uint32 MAIN_SCRIPT_SIZE   = 200000;
    static constexpr uint32 SCRIPT_SPACE_SIZE  = MAIN_SCRIPT_SIZE + MISSION_SCRIPT_SIZE;

    //! Lower `MAIN_SCRIPT_SIZE` is where MAIN.SCM is, remaining `MISSION_SCRIPT_SIZE` is for other loaded scripts.
    static inline uint8(&ScriptSpace)[SCRIPT_SPACE_SIZE] = *(uint8(*)[SCRIPT_SPACE_SIZE])0xA49960;

    //! Reference to \r ScriptSpace's lower portion for MAIN.SCM - Prefer this over `&ScriptSpace[0]`
    static inline uint8(&MainSCMBlock)[MAIN_SCRIPT_SIZE] = *(uint8(*)[MAIN_SCRIPT_SIZE])(0xA49960 + 0); // Can't use `&ScriptSpace[0]` because init order seems to be messed up...

    //! Reference to \r ScriptSpace's upper portion for other scripts - Prefer this over `&ScriptSpace[MAIN_SCRIPT_SIZE]`
    static inline uint8(&MissionBlock)[MISSION_SCRIPT_SIZE] = *(uint8(*)[MISSION_SCRIPT_SIZE])(0xA49960 + MAIN_SCRIPT_SIZE);  // Can't use `&ScriptSpace[MAIN_SCRIPT_SIZE]` because init order seems to be messed up...

    static inline std::array<tScriptSwitchCase, MAX_NUM_SwitchJumpTable>&                        SwitchJumpTable                = *(std::array<tScriptSwitchCase, MAX_NUM_SwitchJumpTable>*)0xA43CF8;
    static inline uint16& NumberOfEntriesInSwitchTable = *reinterpret_cast<uint16*>(0xA43F50);
    static inline uint16& NumberOfEntriesStillToReadForSwitch = *reinterpret_cast<uint16*>(0xA43F60);


    static inline std::array<int16, MAX_NUM_CARDS>&                                              CardStack                      = *(std::array<int16, MAX_NUM_CARDS>*)0xA44218;
    static inline std::array<int32, MAX_NUM_MISSION_SCRIPTS>&                                    MultiScriptArray               = *(std::array<int32, MAX_NUM_MISSION_SCRIPTS>*)0xA444C8;
    static inline std::array<tScriptConnectLodsObject, MAX_NUM_SCRIPT_CONNECT_LODS_OBJECTS>&     ScriptConnectLodsObjects       = *(std::array<tScriptConnectLodsObject, MAX_NUM_SCRIPT_CONNECT_LODS_OBJECTS>*)0xA44800;
    static inline std::array<tScriptAttachedAnimGroup, MAX_NUM_SCRIPT_ATTACHED_ANIM_GROUPS>&     ScriptAttachedAnimGroups       = *(std::array<tScriptAttachedAnimGroup, MAX_NUM_SCRIPT_ATTACHED_ANIM_GROUPS>*)0xA44850;
    static inline std::array<eModelID, MAX_NUM_VEHICLE_MODELS_BLOCKED_BY_SCRIPT>&                VehicleModelsBlockedByScript   = *(std::array<eModelID, MAX_NUM_VEHICLE_MODELS_BLOCKED_BY_SCRIPT>*)0xA448F0;
    static inline std::array<eModelID, MAX_NUM_SUPPRESSED_VEHICLE_MODELS>&                       SuppressedVehicleModels        = *(std::array<eModelID, MAX_NUM_SUPPRESSED_VEHICLE_MODELS>*)0xA44940;
    static inline std::array<CEntity*, MAX_NUM_INVISIBILITY_SETTINGS>&                           InvisibilitySettingArray       = *(std::array<CEntity*, MAX_NUM_INVISIBILITY_SETTINGS>*)0xA449E0;

    static inline std::array<tScriptParam, MAX_NUM_LOCAL_VARIABLES_FOR_CURRENT_MISSION>&         LocalVariablesForCurrentMission =*(std::array<tScriptParam, MAX_NUM_LOCAL_VARIABLES_FOR_CURRENT_MISSION>*)0xA48960;
    static inline uint32& LargestNumberOfMissionScriptLocalVariables = *reinterpret_cast<uint32*>(0xA444B4);

    static inline std::array<tBuildingSwap, MAX_NUM_BUILDING_SWAPS>&                             BuildingSwapArray              = *(std::array<tBuildingSwap, MAX_NUM_BUILDING_SWAPS>*)0xA44A30;

    static inline std::array<tUsedObject, MAX_NUM_USED_OBJECTS>&                                 UsedObjectArray                = *(std::array<tUsedObject, MAX_NUM_USED_OBJECTS>*)0xA44B70;
    static inline uint16& NumberOfUsedObjects = *reinterpret_cast<uint16*>(0xA44B6C);

    static inline std::array<tScriptBrainWaitEntity, MAX_NUM_ENTITIES_WAITING_FOR_SCRIPT_BRAIN>& EntitiesWaitingForScriptBrain  = *(std::array<tScriptBrainWaitEntity, MAX_NUM_ENTITIES_WAITING_FOR_SCRIPT_BRAIN>*)0xA476B0;
    static inline std::array<CRunningScript, MAX_NUM_SCRIPTS>&                                   ScriptsArray                   = *(std::array<CRunningScript, MAX_NUM_SCRIPTS>*)0xA8B430;
    static inline std::array<tScriptText, MAX_NUM_INTRO_TEXT_LINES>&                             IntroTextLines                 = *(std::array<tScriptText, MAX_NUM_INTRO_TEXT_LINES>*)0xA913E8;
    static inline uint16& NumberOfIntroTextLinesThisFrame = *reinterpret_cast<uint16*>(0xA44B68);

    static inline std::array<tScriptRectangle, MAX_NUM_SCRIPT_RECTANGLES>&                       IntroRectangles                = *(std::array<tScriptRectangle, MAX_NUM_SCRIPT_RECTANGLES>*)0xA92D68;
    static inline uint16& NumberOfIntroRectanglesThisFrame = *reinterpret_cast<uint16*>(0xA44B5C);

    static inline std::array<CSprite2d, MAX_NUM_SCRIPT_SPRITES>&                                 ScriptSprites                  = *(std::array<CSprite2d, MAX_NUM_SCRIPT_SPRITES>*)0xA94B68;

    static inline uint16& NumberOfExclusiveMissionScripts = *reinterpret_cast<uint16*>(0xA444B8);
    static inline uint16& NumberOfMissionScripts = *reinterpret_cast<uint16*>(0xA444BC);

    //
    // Script things
    //

    static inline std::array<tScriptSphere, MAX_NUM_SCRIPT_SPHERES>&                             ScriptSphereArray              = *(std::array<tScriptSphere, MAX_NUM_SCRIPT_SPHERES>*)0xA91268;
    static inline std::array<tScriptEffectSystem, MAX_NUM_SCRIPT_EFFECT_SYSTEMS>&                ScriptEffectSystemArray        = *(std::array<tScriptEffectSystem, MAX_NUM_SCRIPT_EFFECT_SYSTEMS>*)0xA44110;
    static inline std::array<tScriptSearchlight, MAX_NUM_SCRIPT_SEARCH_LIGHT>&                   ScriptSearchLightArray         = *(std::array<tScriptSearchlight, MAX_NUM_SCRIPT_SEARCH_LIGHT>*)0xA94D68;

    static inline std::array<tScriptSequence, MAX_NUM_SCRIPT_SEQUENCE_TASKS>&                    ScriptSequenceTaskArray        = *(std::array<tScriptSequence, MAX_NUM_SCRIPT_SEQUENCE_TASKS>*)0xA43F68;
    static inline uint16&                                                                        NumberOfScriptSearchLights     = *reinterpret_cast<uint16*>(0xA90830);

    static inline std::array<tScriptCheckpoint, MAX_NUM_SCRIPT_CHECKPOINTS>&                     ScriptCheckpointArray          = *(std::array<tScriptCheckpoint, MAX_NUM_SCRIPT_CHECKPOINTS>*)0xA44070;
    static inline uint16&                                                                        NumberOfScriptCheckpoints      = *reinterpret_cast<uint16*>(0xA44068);

    static inline bool& DbgFlag = *reinterpret_cast<bool*>(0x859CF8);
    static inline void*& SwitchDefaultAddress = *reinterpret_cast<void**>(0xA43F54);
    static inline bool& SwitchDefaultExists = *reinterpret_cast<bool*>(0xA43F58);
    static inline int32& ValueToCheckInSwitchStatement = *reinterpret_cast<int32*>(0xA43F5C);
    static inline int16& CardStackPosition = *reinterpret_cast<int16*>(0xA44210);
    static inline bool& bDrawSubtitlesBeforeFade = *reinterpret_cast<bool*>(0xA44488);
    static inline bool& bDrawOddJobTitleBeforeFade = *reinterpret_cast<bool*>(0xA44489);
    static inline bool& bScriptHasFadedOut = *reinterpret_cast<bool*>(0xA4448A);
    static inline bool& bAddNextMessageToPreviousBriefs = *reinterpret_cast<bool*>(0xA4448B);
    static inline int32& ForceRandomCarModel = *reinterpret_cast<int32*>(0xA4448C);
    static inline eCrossHairType& bDrawCrossHair = *reinterpret_cast<eCrossHairType*>(0xA44490);
    static inline bool& bEnableCraneRelease = *reinterpret_cast<bool*>(0xA44494);
    static inline bool& bEnableCraneLower = *reinterpret_cast<bool*>(0xA44495);
    static inline bool& bEnableCraneRaise = *reinterpret_cast<bool*>(0xA44496);
    static inline float& fCameraHeadingStepWhenPlayerIsAttached = *reinterpret_cast<float*>(0xA44498);
    static inline float& fCameraHeadingWhenPlayerIsAttached = *reinterpret_cast<float*>(0xA4449C);
    static inline bool& bDisplayHud = *reinterpret_cast<bool*>(0xA444A0);
    static inline bool& HideAllFrontEndMapBlips = *reinterpret_cast<bool*>(0xA444A1);
    static inline bool& RadarShowBlipOnAllLevels = *reinterpret_cast<bool*>(0xA444A2);
    static inline uint8& RadarZoomValue = *reinterpret_cast<uint8*>(0xA444A3);
    static inline bool& bPlayerIsOffTheMap = *reinterpret_cast<bool*>(0xA444A4);
    static inline char& RiotIntensity = *reinterpret_cast<char*>(0xA444A5);
    static inline bool& bPlayerHasMetDebbieHarry = *reinterpret_cast<bool*>(0xA444A6);
    static inline bool& bDisplayNonMiniGameHelpMessages = *reinterpret_cast<bool*>(0xA444A7);
    static inline bool& bMiniGameInProgress = *reinterpret_cast<bool*>(0xA444A8);
    static inline int32& ScriptPickupCycleIndex = *reinterpret_cast<int32*>(0xA444AC);
    static inline int8& FailCurrentMission = *reinterpret_cast<int8*>(0xA444B0);
    static inline bool& bAlreadyRunningAMissionScript = *reinterpret_cast<bool*>(0xA444B1);
    static inline uint32& LargestMissionScriptSize = *reinterpret_cast<uint32*>(0xA444C0);
    static inline uint32& MainScriptSize = *reinterpret_cast<uint32*>(0xA444C4);
    static inline bool& bUsingAMultiScriptFile = *reinterpret_cast<bool*>(0xA447E8);
    static inline int32& StoreVehicleIndex = *reinterpret_cast<int32*>(0xA447EC);
    static inline bool& StoreVehicleWasRandom = *reinterpret_cast<bool*>(0xA447F0);
    static inline uint16& CommandsExecuted = *reinterpret_cast<uint16*>(0xA447F4);
    static inline uint16& ScriptsUpdated = *reinterpret_cast<uint16*>(0xA447F8);
    static inline uint16& MessageWidth = *reinterpret_cast<uint16*>(0xA44B60);
    static inline uint16& MessageCentre = *reinterpret_cast<uint16*>(0xA44B64);
    static inline bool& bUseMessageFormatting = *reinterpret_cast<bool*>(0xA44B66);
    static inline bool& UseTextCommands = *reinterpret_cast<bool*>(0xA44B67);
    static inline int32& LastRandomPedId = *reinterpret_cast<int32*>(0xA476A4);
    static inline uint32& LastMissionPassedTime = *reinterpret_cast<uint32*>(0xA476A8);
    static inline int32& OnAMissionFlag = *reinterpret_cast<int32*>(0xA476AC);
    static inline CStreamedScripts& StreamedScripts = *reinterpret_cast<CStreamedScripts*>(0xA47B60);
    static inline CScriptResourceManager& ScriptResourceManager = *reinterpret_cast<CScriptResourceManager*>(0xA485A8);
    static inline CUpsideDownCarCheck& UpsideDownCars = *reinterpret_cast<CUpsideDownCarCheck*>(0xA4892C);
    static inline CRunningScript*& pIdleScripts = *reinterpret_cast<CRunningScript**>(0xA8B428);
    static inline CRunningScript*& pActiveScripts = *reinterpret_cast<CRunningScript**>(0xA8B42C);
    static inline CMissionCleanup& MissionCleanUp = *reinterpret_cast<CMissionCleanup*>(0xA90850);
    static inline CStuckCarCheck& StuckCars = *reinterpret_cast<CStuckCarCheck*>(0xA90AB0);
    static inline CScriptsForBrains& ScriptsForBrains = *reinterpret_cast<CScriptsForBrains*>(0xA90CF0);

public:
    static void InjectHooks();

    static void Init();
    static void InitialiseAllConnectLodObjects();
    static void InitialiseConnectLodObjects(uint16 index);
    static void InitialiseSpecialAnimGroup(uint16 index);
    static void InitialiseSpecialAnimGroupsAttachedToCharModels();
    static void ReadObjectNamesFromScript();
    static void UpdateObjectIndices();
    static void ReadMultiScriptFileOffsetsFromScript();

    static uint32 AddScriptCheckpoint(CVector at, CVector pointTo, float radius, int32 type);
    static uint32 AddScriptEffectSystem(FxSystem_c* system);
    static uint32 AddScriptSearchLight(CVector start, CEntity* entity, CVector target, float targetRadius, float baseRadius);
    static uint32 AddScriptSphere(uint32 id, CVector posn, float radius);

    static void   AddToBuildingSwapArray(CBuilding* building, int32 oldModelId, int32 newModelId);
    static void   AddToInvisibilitySwapArray(CEntity* entity, bool bVisible);
    static void   AddToListOfConnectedLodObjects(CObject* obj1, CObject* obj2);
    static void   AddToListOfSpecialAnimGroupsAttachedToCharModels(int32 modelId, Const char* ifpName);
    static void   AddToSwitchJumpTable(int32 switchValue, int32 switchLabelLocalAddress);
    static void   AddToVehicleModelsBlockedByScript(int32 modelIndex);
    static void   AddToWaitingForScriptBrainArray(CEntity* entity, int16 arg2);

    static void AttachSearchlightToSearchlightObject(int32 searchLightId, CObject* tower, CObject* housing, CObject* bulb, float offsetX, float offsetY, float offsetZ);
    static char CheckStreamedScriptVersion(RwStream* stream, char* arg2);
    static void CleanUpThisObject(CObject* obj);
    static void CleanUpThisPed(CPed* ped);
    static void CleanUpThisVehicle(CVehicle* vehicle);
    static void ClearAllSuppressedCarModels();
    static void ClearAllVehicleModelsBlockedByScript();
    static void ClearSpaceForMissionEntity(const CVector& pos, CEntity* entity);
    static void DoScriptSetupAfterPoolsHaveLoaded();

    static int32 GetActualScriptThingIndex(int32 index, eScriptThingType type);
    static int32 GetNewUniqueScriptThingIndex(int32 index, eScriptThingType type);
    static int32 GetScriptIndexFromPointer(CRunningScript* thread);
    static int32 GetUniqueScriptThingIndex(int32 playerGroup, eScriptThingType type);

    static bool IsEntityWithinAnySearchLight(CEntity* entity, int32* pIndex);
    static bool IsEntityWithinSearchLight(uint32 index, CEntity* entity);
    static bool IsPedStopped(CPed* ped);
    static bool IsPlayerOnAMission();
    static bool IsPointWithinSearchLight(CVector* pointPosn, int32 index);
    static bool IsVehicleStopped(CVehicle* vehicle);

    static bool Load();
    static bool Save();

    static void MoveSearchLightBetweenTwoPoints(int32 index, float x1, float y1, float z1, float x2, float y2, float z2, float pathSpeed);
    static void MoveSearchLightToEntity(int32 index, CEntity* entity, float pathSpeed);
    static void MoveSearchLightToPointAndStop(int32 index, float x, float y, float z, float pathSpeed);

    static void Process();
    static void ProcessAllSearchLights();
    static void ProcessWaitingForScriptBrainArray();

    static void ReinitialiseSwitchStatementData();

    static void RemoveFromVehicleModelsBlockedByScript(int32 modelIndex);
    static void RemoveFromWaitingForScriptBrainArray(CEntity* entity, int16 modelIndex);
    static void RemoveScriptCheckpoint(int32 scriptIndex);
    static void RemoveScriptEffectSystem(int32 scriptIndex);
    static void RemoveScriptSearchLight(int32 scriptIndex);
    static void RemoveScriptSphere(int32 scriptIndex);
    static void RemoveScriptTextureDictionary();
    static void RemoveThisPed(CPed* ped);

    static void RenderAllSearchLights();
    static bool ScriptAttachAnimGroupToCharModel(int32 modelId, char* ifpName); // 0x474800
    static void ScriptConnectLodsFunction(int32 objectHandle1, int32 objectHandle2);
    static void ScriptDebugCircle2D(float x, float y, float width, float height, CRGBA color);
    static CRunningScript* StartNewScript(uint8* startIP);
    static CRunningScript* StartNewScript(uint8* startIP, uint16 index);
    static void StartTestScript();
    static void UndoBuildingSwaps();
    static void UndoEntityInvisibilitySettings();
    static void UseSwitchJumpTable(int32* pSwitchLabelAddress);
    static void WipeLocalVariableMemoryForMissionScript();

    static bool HasCarModelBeenSuppressed(eModelID carModelId);
    static bool HasVehicleModelBeenBlockedByScript(eModelID carModelId);

    // DEBUG
    static void ScriptDebugLine3D(const CVector& start, const CVector& end, uint32 color1, uint32 color2);
    static void RenderTheScriptDebugLines();

    static void PrintListSizes();

    static void DrawScriptSpheres();
    static void HighlightImportantArea(uint32 markerId, float fromX, float fromY, float toX, float toY, float height);
    static void HighlightImportantArea(uint32 markerId, const CRect& area, float height) { HighlightImportantArea(markerId, area.left, area.top, area.right, area.bottom, height); } // NOTSA
    static void HighlightImportantAngledArea(uint32 markerId, float fromX, float fromY, float toX, float toY, float angledToX, float angledToY, float angledFromX, float angledFromY, float height);
    static void DrawDebugSquare(float, float, float, float);
    static void DrawDebugSquare(const CRect& area) { DrawDebugSquare(area.left, area.top, area.right, area.bottom); }
    static void DrawDebugAngledSquare(const CVector2D& inf, const CVector2D& sup, const CVector2D& rotSup, const CVector2D& rotInf);
    static void DrawDebugCube(const CVector& inf, const CVector& sup);
    static void DrawDebugAngledCube(const CVector& inf, const CVector& sup, const CVector2D& rotSup, const CVector2D& rotInf);
    static void DrawScriptSpritesAndRectangles(bool bDrawBeforeFade);

    static int32* GetPointerToScriptVariable(uint32 offset) {
        // TODO: find out how this method changed between re3 and GTA:SA
        assert(offset >= 8 && offset < GetSizeOfVariableSpace());
        return (int32*)&ScriptSpace[offset];
    }

    static int8 Read1ByteFromScript(uint8*& ip) {
        int8 retval = *reinterpret_cast<int8*>(ip);
        ip += 1;
        return retval;
    }

    static int16 Read2BytesFromScript(uint8*& ip) {
        int16 retval = *reinterpret_cast<int16*>(ip);
        ip += 2;
        return retval;
    }

    static int32 Read4BytesFromScript(uint8*& ip) {
        int32 retval = *reinterpret_cast<int32*>(ip); // big-endian unfriendly/unaligned mem access
        ip += 4;
        return retval;
    }

    static float ReadFloatFromScript(uint8*& ip) {
        int32 retval = Read4BytesFromScript(ip);
        return *reinterpret_cast<float*>(&retval);
    }

#define KEY_LENGTH_IN_SCRIPT (8)

    static void ReadTextLabelFromScript(uint8*& ip, char* buf) {
        strncpy_s(buf, KEY_LENGTH_IN_SCRIPT, (const char*) & ScriptSpace[*ip], KEY_LENGTH_IN_SCRIPT);
    }

    static auto * GetTextByKeyFromScript(uint8*& ip) {
        auto* text = TheText.Get((const char*)&ScriptSpace[*ip]);
        ip += KEY_LENGTH_IN_SCRIPT;
        return text;
    }

    static CVector2D& ReadCVector2DFromScript(uint8 offset) {
        return *reinterpret_cast<CVector2D*>(&ScriptParams[offset]);
    }

    static CVector& ReadCVectorFromScript(uint8 offset) {
        return *reinterpret_cast<CVector*>(&ScriptParams[offset]);
    }

    static uint32 GetSizeOfVariableSpace() {
        uint8* tmp = &MainSCMBlock[3];
        return Read4BytesFromScript(tmp);
    }
};
