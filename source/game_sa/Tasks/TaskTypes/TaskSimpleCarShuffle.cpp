#include "StdInc.h"

#include "TaskSimpleCarShuffle.h"
#include "TaskUtilityLineUpPedWithCar.h"

void CTaskSimpleCarShuffle::InjectHooks() {
    RH_ScopedVirtualClass(CTaskSimpleCarShuffle, 0x86ed74, 9);
    RH_ScopedCategory("Tasks/TaskTypes");

    RH_ScopedInstall(Constructor, 0x646810);
    RH_ScopedInstall(Destructor, 0x646890);

    RH_ScopedInstall(FinishAnimCarShuffleCB, 0x646970);
    RH_ScopedInstall(StartAnim, 0x64B3E0);

    RH_ScopedVMTInstall(Clone, 0x649C40);
    RH_ScopedVMTInstall(GetTaskType, 0x646880);
    RH_ScopedVMTInstall(MakeAbortable, 0x646900);
    RH_ScopedVMTInstall(ProcessPed, 0x64DC40);
    RH_ScopedVMTInstall(SetPedPosition, 0x646920);
}

// 0x646810
CTaskSimpleCarShuffle::CTaskSimpleCarShuffle(CVehicle* car, int32 targetDoor, CTaskUtilityLineUpPedWithCar* lineUpTask) :
    m_Car{ car },
    m_TargetDoor{ targetDoor },
    m_LineUpUtility{ lineUpTask }
{
    CEntity::SafeRegisterRef(m_Car);
}

// 0x649C40
CTaskSimpleCarShuffle::CTaskSimpleCarShuffle(const CTaskSimpleCarShuffle&) :
    CTaskSimpleCarShuffle{ m_Car, m_TargetDoor, m_LineUpUtility }
{
}

// 0x646890
CTaskSimpleCarShuffle::~CTaskSimpleCarShuffle() {
    CEntity::SafeCleanUpRef(m_Car);
    if (m_Anim) {
        m_Anim->SetDefaultFinishCallback();
    }
}

// 0x646970
void CTaskSimpleCarShuffle::FinishAnimCarShuffleCB(CAnimBlendAssociation* assoc, void* data) {
    const auto self = static_cast<CTaskSimpleCarShuffle*>(data);

    assert(self->m_Anim);

    self->m_Anim->SetBlendDelta(-1000.f);
    self->m_Anim = nullptr;

    self->m_bFinished = true;
}

// 0x64B3E0
void CTaskSimpleCarShuffle::StartAnim(const CPed* a2) {
    assert(!m_Anim);
    m_Anim = CAnimManager::BlendAnimation(
        m_Car->m_pRwClump,
        m_Car->GetAnimGroup().GetGroup(ANIM_ID_CAR_SHUFFLE_RHS_1),
        ANIM_ID_CAR_SHUFFLE_RHS_1,
        1000.f
    );
    m_Anim->SetFinishCallback(FinishAnimCarShuffleCB, this);
}

// 0x646900
bool CTaskSimpleCarShuffle::MakeAbortable(CPed* ped, eAbortPriority priority, const CEvent* event) {
    if (priority != ABORT_PRIORITY_IMMEDIATE) {
        return false;
    }
    if (m_Anim) {
        m_Anim->SetBlendDelta(-1000.f);
    }
    return true;
}

// 0x64DC40
bool CTaskSimpleCarShuffle::ProcessPed(CPed* ped) {
    if (m_bFinished || !m_Car) {
        return true;
    }
    if (!m_Anim) {
        StartAnim(ped);
    }
    return false;
}

// 0x646920
bool CTaskSimpleCarShuffle::SetPedPosition(CPed* ped) {
    if (!ped->bInVehicle || m_Anim && m_Anim->GetBlendAmount() >= 0.9f) {
        m_LineUpUtility->ProcessPed(ped, m_Car, m_Anim);
    } else {
        ped->SetPedPositionInCar();
    }
    return true;
}
