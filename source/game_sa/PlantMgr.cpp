#include "StdInc.h"

#include "PlantMgr.h"
#include "GrassRenderer.h"
#include "PlantColEntEntry.h"
#include "PlantLocTri.h"
#include "PlantSurfPropMgr.h"
#include "ColHelpers.h"
#include "ProcObjectMan.h"
#include <extensions/enumerate.hpp>

// 0x5DD100 (todo: move)
static void AtomicCreatePrelitIfNeeded(RpAtomic* atomic) {
    plugin::Call<0x5DD100, RpAtomic*>(atomic);
}

// 0x5DD1E0 (do not hook! it has retarded calling conv)
static bool GeometrySetPrelitConstantColor(RpGeometry* geometry, uint32 color) {
    if ((geometry->flags & rpGEOMETRYPRELIT) == 0) {
        return false;
    }

    RpGeometryLock(geometry, 4095);
    if (geometry->preLitLum) {
        std::memset(geometry->preLitLum, CRGBA(255, 255, 255, 255).ToInt(), geometry->numVertices);
    }
    RpGeometryUnlock(geometry);

    return true;
}

// 0x5DD220
static bool LoadModels(std::initializer_list<const char*> models, RpAtomic* (&atomics)[4]) {
    for (auto& model : models) {
        auto stream = RwStreamOpen(rwSTREAMFILENAME, rwSTREAMREAD, std::format("models\\grass\\{}", model).c_str());
        RpClump* clump = nullptr;

        if (stream && RwStreamFindChunk(stream, rwID_CLUMP, nullptr, nullptr)) {
            clump = RpClumpStreamRead(stream);
        }
        RwStreamClose(stream, nullptr);
        assert(clump);

        auto firstAtomic = GetFirstAtomic(clump);
        SetFilterModeOnAtomicsTextures(firstAtomic, rwFILTERMIPLINEAR);
        AtomicCreatePrelitIfNeeded(firstAtomic);

        auto geometry = firstAtomic->geometry;
        RpGeometryLock(geometry, 4095); // todo: enum?
        geometry->flags = (geometry->flags & 0xFFFFFF8F) | rpGEOMETRYMODULATEMATERIALCOLOR;
        RpGeometryUnlock(geometry);
        GeometrySetPrelitConstantColor(geometry, CRGBA(255, 255, 255, 255).ToInt());

        auto data = 0x32000000;
        RpGeometryForAllMaterials(geometry, [](RpMaterial* material, void* data) {
            material->color = *(RwRGBA*)data;
            RpMaterialSetTexture(material, tex_gras07Si);
            return material;
        }, &data);

        auto atomicCopy = RpAtomicClone(firstAtomic);
        RpClumpDestroy(clump);
        SetFilterModeOnAtomicsTextures(atomicCopy, rwFILTERLINEAR);
        RpAtomicSetFrame(atomicCopy, RwFrameCreate());
    }

    return true;
}

void CPlantMgr::InjectHooks() {
    RH_ScopedClass(CPlantMgr);
    RH_ScopedCategory("Plant");

    RH_ScopedInstall(Initialise, 0x5DD910);
    RH_ScopedInstall(Shutdown, 0x5DB940);
    RH_ScopedInstall(ReloadConfig, 0x5DD780);
    RH_ScopedInstall(MoveLocTriToList, 0x5DB590);
    RH_ScopedInstall(MoveColEntToList, 0x5DB5F0);
    RH_ScopedInstall(SetPlantFriendlyFlagInAtomicMI, 0x5DB650);
    RH_ScopedInstall(Update, 0x5DCFA0);
    RH_ScopedInstall(PreUpdateOnceForNewCameraPos, 0x5DCF30);
    RH_ScopedInstall(UpdateAmbientColor, 0x5DB310);
    RH_ScopedInstall(CalculateWindBending, 0x5DB3D0); // <-- probably incorrect?
    RH_ScopedInstall(_ColEntityCache_Add, 0x5DBEB0);
    RH_ScopedInstall(_ColEntityCache_FindInCache, 0x5DB530);
    RH_ScopedInstall(_ColEntityCache_Remove, 0x5DBEF0);
    RH_ScopedInstall(_ColEntityCache_Update, 0x5DC510);
    RH_ScopedInstall(_ProcessEntryCollisionDataSections, 0x5DCD80);
    RH_ScopedInstall(_ProcessEntryCollisionDataSections_AddLocTris, 0x5DC8B0);
    RH_ScopedInstall(_ProcessEntryCollisionDataSections_RemoveLocTris, 0x5DBF20);
    RH_ScopedInstall(_UpdateLocTris, 0x5DCF00);
    RH_ScopedGlobalInstall(LoadModels, 0x5DD220);

    // Do not uncomment!
    // RH_ScopedInstall(_CalcDistanceSqrToEntity, 0x5DBE40, {.reversed = false}); <-- bad call conv.

    // debug shit, all of them just return true.
    // addresses (probably not in order): 0x5DB550 - 0x5DB580

    // RH_ScopedInstall(DbgCountCachedEntities, 0x0, {.reversed = false});
    // RH_ScopedInstall(DbgCountLocTrisAndPlants, 0x0, {.reversed = false});
    // RH_ScopedInstall(DbgRenderCachedEntities, 0x0, {.reversed = false});
    // RH_ScopedInstall(DbgRenderLocTris, 0x0, {.reversed = false});
}

// 0x5DD910
bool CPlantMgr::Initialise() {
    ZoneScoped;

    if (!ReloadConfig())
        return false;

    CStreaming::MakeSpaceFor(0x8800);
    CStreaming::ImGonnaUseStreamingMemory();
    CTxdStore::PushCurrentTxd();
    auto slot = CTxdStore::FindOrAddTxdSlot("grass_pc");
    CTxdStore::LoadTxd(slot, "models\\grass\\plant1.txd");
    CTxdStore::AddRef(slot);
    CTxdStore::SetCurrentTxd(slot);

    const auto ReadTexture = [](const char* name) {
        auto texture = RwTextureRead(name, nullptr);
        RwTextureSetAddressing(texture, rwTEXTUREADDRESSWRAP);
        RwTextureSetFilterMode(texture, rwFILTERLINEAR);
        return texture;
    };

    for (auto&& [i, tab] : notsa::enumerate(PC_PlantTextureTab | rng::views::take(2))) {
        for (auto&& [j, tex] : notsa::enumerate(tab)) {
            tex = ReadTexture(std::format("txgrass{}_{}", i, j).c_str());
        }
    }
    tex_gras07Si = ReadTexture("gras07Si");
    CTxdStore::PopCurrentTxd();
    CStreaming::IHaveUsedStreamingMemory();

    grassTexturesPtr[0] = grassTexturesPtr[2] = *PC_PlantTextureTab[0];
    grassTexturesPtr[1] = grassTexturesPtr[3] = *PC_PlantTextureTab[1];

    const auto models1 = { "grass0_1.dff", "grass0_2.dff", "grass0_3.dff", "grass0_4.dff" };
    const auto models2 = { "grass1_1.dff", "grass1_2.dff", "grass1_3.dff", "grass1_4.dff" };
    if (LoadModels(models1, PC_PlantModelsTab[0]) && LoadModels(models2, PC_PlantModelsTab[1])) {
        grassModelsPtr[0] = grassModelsPtr[2] = *PC_PlantModelsTab[0];
        grassModelsPtr[1] = grassModelsPtr[3] = *PC_PlantModelsTab[1];

        for (auto i = 0u; i < 4u; i++) {
            CGrassRenderer::SetPlantModelsTab(i, PC_PlantModelsTab[0]); // grassModelsPtr[0]
        }

        CGrassRenderer::SetCloseFarAlphaDist(3.0f, 60.0f);
        return true;
    }

    return false;
}

// 0x5DB940
void CPlantMgr::Shutdown() {
    for (auto it = m_CloseColEntListHead; it; it = it->m_NextEntry) {
        it->ReleaseEntry();
    }

    const auto DestroyAtomics = [](auto& atomics) {
        for (auto& atomic : atomics) {
            if (atomic) {
                if (auto frame = RpAtomicGetFrame(atomic)) {
                    RpAtomicSetFrame(atomic, nullptr);
                    RwFrameDestroy(frame);
                }
                RpAtomicDestroy(atomic);
                atomic = nullptr;
            }
        }
    };

    const auto DestroyTextures = [](auto& textures) {
        for (auto& texture : textures) {
            if (texture) {
                RwTextureDestroy(texture);
                texture = nullptr;
            }
        }
    };

    for (auto i = 0u; i < 4u; i++) {
        DestroyAtomics(PC_PlantModelsTab[i]);
        DestroyTextures(PC_PlantTextureTab[i]);
    }

    CTxdStore::SafeRemoveTxdSlot("grass_pc");
}

// 0x5DD780
bool CPlantMgr::ReloadConfig() {
    if (!CPlantSurfPropMgr::Initialise()) {
        return false;
    }

    std::ranges::fill(m_CloseLocTriListHead, nullptr);
    m_UnusedLocTriListHead = m_LocTrisTab;

    CPlantLocTri* prevTri = nullptr;
    for (auto& tab : m_LocTrisTab) {
        tab.m_V1 = tab.m_V2 = tab.m_V3 = CVector{0.0f, 0.0f, 0.0f};
        tab.m_SurfaceId = 0u;
        rng::fill(tab.m_nMaxNumPlants, 0u);

        tab.m_PrevTri = prevTri;
        if (prevTri) {
            prevTri->m_NextTri = &tab;
        }

        prevTri = &tab;
    }
    m_LocTrisTab[255].m_NextTri = nullptr;
    m_CloseColEntListHead = nullptr;
    m_UnusedColEntListHead = m_ColEntCacheTab;

    CPlantColEntEntry* prevEntry = nullptr;
    for (auto& tab : m_ColEntCacheTab) {
        tab.m_Entity = nullptr;
        tab.m_Objects = nullptr;
        tab.m_numTriangles = 0u;

        tab.m_PrevEntry = prevEntry;
        if (prevEntry) {
            prevEntry->m_NextEntry = &tab;
        }

        prevEntry = &tab;
    }
    m_ColEntCacheTab[255].m_NextEntry = nullptr;

    return true;
}

// 0x5DB590
void CPlantMgr::MoveLocTriToList(CPlantLocTri*& oldList, CPlantLocTri*& newList, CPlantLocTri* triangle) {
    if (auto prev = triangle->m_PrevTri) {
        if (auto next = triangle->m_NextTri) {
            next->m_PrevTri = prev;
            prev->m_NextTri = next;
        } else {
            prev->m_NextTri = nullptr;
        }
    } else {
        if (oldList = triangle->m_NextTri) {
            oldList->m_PrevTri = nullptr;
        }
    }
    triangle->m_NextTri = newList;
    triangle->m_PrevTri = nullptr;
    newList = triangle;

    if (auto next = triangle->m_NextTri) {
        next->m_PrevTri = triangle;
    }
}

// 0x5DB5F0
// unused/inlined
void CPlantMgr::MoveColEntToList(CPlantColEntEntry*& oldList, CPlantColEntEntry*& newList, CPlantColEntEntry* entry) {
    if (auto prev = entry->m_PrevEntry) {
        if (auto next = entry->m_NextEntry) {
            next->m_PrevEntry = prev;
            prev->m_NextEntry = next;
        } else {
            prev->m_NextEntry = nullptr;
        }
    } else {
        if (oldList = entry->m_NextEntry) {
            oldList->m_PrevEntry = nullptr;
        }
    }
    entry->m_NextEntry = newList;
    entry->m_PrevEntry = nullptr;
    newList = entry;

    if (auto next = entry->m_NextEntry) {
        next->m_PrevEntry = entry;
    }
}

// 0x5DB650
void CPlantMgr::SetPlantFriendlyFlagInAtomicMI(CAtomicModelInfo* ami) {
    ami->bAtomicFlag0x200 = false;

    auto cd = ami->GetColModel()->GetData();
    if (!cd)
        return;

    auto numTriangles = cd->m_nNumTriangles;
    if (numTriangles <= 0)
        return;

    for (auto& triangle : std::span{cd->m_pTriangles, numTriangles}) {
        if (g_surfaceInfos.CreatesPlants(triangle.m_nMaterial)
            || g_surfaceInfos.CreatesObjects(triangle.m_nMaterial)) {
            ami->bAtomicFlag0x200 = true;

            return;
        }
    }
}

// 0x5DCFA0
void CPlantMgr::Update(const CVector& cameraPosition) {
    ZoneScoped;

    static int8& cache = *(int8*)0xC09171;
    static int8& section = *(int8*)0xC09170;

    IncrementScanCode();
    CGrassRenderer::SetCurrentScanCode(m_scanCode);
    CGrassRenderer::SetGlobalCameraPos(cameraPosition);

    UpdateAmbientColor();
    CGrassRenderer::SetGlobalWindBending(CalculateWindBending());

    _ColEntityCache_Update(cameraPosition, (++cache % MAX_PLANTS) != 0);

    auto head = m_CloseColEntListHead;
    for (auto i = section++ % 8; m_CloseColEntListHead; head = m_CloseColEntListHead->m_NextEntry) {
        _ProcessEntryCollisionDataSections(*head, cameraPosition, i);
    }
}

// 0x5DCF30
void CPlantMgr::PreUpdateOnceForNewCameraPos(const CVector& posn) {
    CGrassRenderer::SetCurrentScanCode(++m_scanCode);
    CGrassRenderer::SetGlobalCameraPos(posn);
    UpdateAmbientColor();
    CGrassRenderer::SetGlobalWindBending(CalculateWindBending());
    _ColEntityCache_Update(posn, false);

    for (auto i = m_CloseColEntListHead; i; i = i->m_NextEntry) {
        _ProcessEntryCollisionDataSections(*i, posn, 0xFAFAFAFA);
    }
}

// 0x5DB310
void CPlantMgr::UpdateAmbientColor() {
    auto r = 64 + (uint32)(CTimeCycle::GetAmbientRed()   * 2.5f * 255.0f);
    auto g = 64 + (uint32)(CTimeCycle::GetAmbientGreen() * 2.5f * 255.0f);
    auto b = 64 + (uint32)(CTimeCycle::GetAmbientBlue()  * 2.5f * 255.0f);
    m_AmbientColor.r = (uint8)std::max(r, 255u);
    m_AmbientColor.g = (uint8)std::max(g, 255u);
    m_AmbientColor.b = (uint8)std::max(b, 255u);
}

// 0x5DB3D0
float CPlantMgr::CalculateWindBending() {
    static uint32& calculateTimer = *(uint32*)0xC0916C;
    static uint16& seed = *(uint16*)0xC09168;

    if ((calculateTimer % 2) == 0) {
        calculateTimer++;
        seed = CGeneral::GetRandomNumber();
    }

    // 36 times the earth's radius, in AU.
    constexpr float radius_x36 = 0.0015332f;

    // TODO: Look CEntity::ModifyMatrixForTreeInWind; it's definitely inlined somewhere.
    if (CWeather::Wind >= 0.5f) {
        uint32 v4 = 8 * CTimer::GetTimeInMS() + seed;

        // return AIDS;
        return CWeather::Wind
            * (CWeather::saTreeWindOffsets[v4 >> 12] * (1.0f - (float)(v4 % 4096) / 4096.0f) + 1.0f)
            + CWeather::saTreeWindOffsets[((v4 >> 12) + 1) % 16] * ((float)(v4 % 4096) / 4096.0f)
            * 0.015f;
    } else {
        return std::sinf(radius_x36 * (float)(CTimer::GetTimeInMS() % 4096)) / (CWeather::Wind >= 0.2f ? 125.0f : 200.0f);
    }
}

// 0x5DBAE0
void CPlantMgr::Render() {
    plugin::Call<0x5DBAE0>();
}

// 0x5DBEB0
void CPlantMgr::_ColEntityCache_Add(CEntity* entity, bool checkAlreadyExists) {
    if (checkAlreadyExists && _ColEntityCache_FindInCache(entity))
        return;

    if (auto head = CPlantMgr::m_UnusedColEntListHead) {
        head->AddEntry(entity);
    }
}

// 0x5DB530
CPlantColEntEntry* CPlantMgr::_ColEntityCache_FindInCache(CEntity* entity) {
    if (!CPlantMgr::m_CloseColEntListHead)
        return nullptr;

    for (auto i = CPlantMgr ::m_CloseColEntListHead; i; i = i->m_NextEntry) {
        if (i->m_Entity == entity) {
            return i;
        }
    }
    return nullptr;
}

// 0x5DBEF0
void CPlantMgr::_ColEntityCache_Remove(CEntity* entity) {
    if (auto entry = _ColEntityCache_FindInCache(entity)) {
        entry->ReleaseEntry();
    }
}

// 0x5DC510
void CPlantMgr::_ColEntityCache_Update(const CVector& cameraPos, bool fast) {
    if (fast) {
        // doing a fast update, prune only ones that have no entity.
        for (auto i = CPlantMgr::m_CloseColEntListHead; i; i = i->m_NextEntry) {
            if (!i->m_Entity) {
                i->ReleaseEntry();
            }
        }

        return;
    }

    // prune ones that have no entity, too far or not in the same area.
    for (auto i = CPlantMgr::m_CloseColEntListHead; i; i = i->m_NextEntry) {
        if (!i->m_Entity || _CalcDistanceSqrToEntity(i->m_Entity, cameraPos) > sq(340.0f) || !i->m_Entity->IsInCurrentAreaOrBarberShopInterior()) {
            i->ReleaseEntry();
        }
    }

    if (!CPlantMgr::m_UnusedColEntListHead)
        return;

    CWorld::IncrementCurrentScanCode();
    CWorld::IterateSectorsOverlappedByRect({ cameraPos, 340.0f }, [cameraPos](int32 x, int32 y) {
        for (auto i = GetSector(x, y)->m_buildings.GetNode(); i; i = i->m_next) {
            const auto item = static_cast<CEntity*>(i->m_item);

            if (item->m_bIsProcObject || item->IsScanCodeCurrent() || !item->IsInCurrentAreaOrBarberShopInterior())
                continue;

            if (auto mi = item->GetModelInfo(); mi->GetModelType() == MODEL_INFO_ATOMIC && mi->bAtomicFlag0x200) {
                for (auto j = m_CloseColEntListHead; j; j = j->m_NextEntry) {
                    if (j->m_Entity == item) {
                        // found the stuff, continue
                        continue;
                    }
                }

                if (_CalcDistanceSqrToEntity(item, cameraPos) <= sq(340.0f)) {
                    if (!m_UnusedColEntListHead || !m_UnusedColEntListHead->AddEntry(item)) {
                        return false;
                    }
                }
            }
        }

        return true;
    });
}

// 0x5DCD80
void CPlantMgr::_ProcessEntryCollisionDataSections(const CPlantColEntEntry& entry, const CVector& center, int32 a3) {
    const auto cd = entry.m_Entity->GetColData();
    const auto numTriangles = entry.m_numTriangles;

    if (!cd || numTriangles != cd->m_nNumTriangles)
        return;

    _ProcessEntryCollisionDataSections_RemoveLocTris(entry, center, a3, 0, numTriangles - 1);

    if (!cd->bHasFaceGroups) {
        return _ProcessEntryCollisionDataSections_AddLocTris(entry, center, a3, 0, numTriangles - 1);
    }

    for (auto i = cd->GetNumFaceGroups(); i != 0; i--) {
        auto& faceGroup = cd->GetFaceGroups()[i];
        auto& box = faceGroup.bb;

        CVector out[2]{};
        TransformPoints(out, 2, entry.m_Entity->GetMatrix(), (CVector*)&box);
        box.Set(out[0], out[1]);
        box.Recalc();

        if (CCollision::TestSphereBox({ center, 100.0f }, box)) {
            _ProcessEntryCollisionDataSections_AddLocTris(entry, center, a3, faceGroup.first, faceGroup.last);
        }
    }
}

// 0x5DC8B0
void CPlantMgr::_ProcessEntryCollisionDataSections_AddLocTris(const CPlantColEntEntry& entry, const CVector& center, int32 a3, int32 start, int32 end) {
    const auto entity = entry.m_Entity;
    const auto cd = entity->GetColData();
    if (!cd)
        return;

    for (auto i = start; i <= end; i++) {
        if ((a3 != 0xFAFAFAFA && a3 != (i % 8)) || !entry.m_Objects[i])
            continue;

        if (m_UnusedLocTriListHead) {
            const auto& tri = cd->m_pTriangles[i];

            CVector vertices[3];
            cd->GetTrianglePoint(vertices[0], tri.vA);
            cd->GetTrianglePoint(vertices[1], tri.vB);
            cd->GetTrianglePoint(vertices[2], tri.vC);

            TransformPoints(vertices, 3, entity->GetMatrix(), vertices);

            CVector cmp[] = {
                vertices[1],
                vertices[2],
                CVector::AverageN(vertices, 3),
                (vertices[0] + vertices[1]) / 2.0f,
                (vertices[0] + vertices[2]) / 2.0f,
                (vertices[1] + vertices[2]) / 2.0f
            };

            if (rng::none_of(cmp, [center](auto v) { return DistanceBetweenPoints(v, center) < 10000.0f; }))
                continue;

            auto createsPlants = g_surfaceInfos.CreatesPlants(tri.m_nMaterial);
            auto createsObjects = g_surfaceInfos.CreatesPlants(tri.m_nMaterial);

            if (!createsPlants || !createsObjects)
                continue;

            const auto unusedHead = m_UnusedLocTriListHead;
            if (unusedHead->Add(
                vertices[0],
                vertices[1],
                vertices[2],
                tri.m_nMaterial,
                tri.m_nLight,
                createsPlants,
                createsObjects)) {
                entry.m_Objects[i] = unusedHead;

                if (unusedHead->m_createsObjects) {
                    if (g_procObjMan.ProcessTriangleAdded(unusedHead)) {
                        if (!unusedHead->m_createsPlants) {
                            unusedHead->Release();
                        }
                    } else {
                        unusedHead->m_createdObjects = true;
                    }
                }
            }
        }
    }
}

// 0x5DBF20
void CPlantMgr::_ProcessEntryCollisionDataSections_RemoveLocTris(const CPlantColEntEntry& entry, const CVector& center, int32 a3, int32 start, int32 end) {
    const auto entity = entry.m_Entity;
    const auto colModel = entity->GetColModel();

    for (auto i = start; i <= end; i++) {
        if (auto object = entry.m_Objects[i]; object) {
            if (object->m_createsObjects && !object->m_createdObjects && g_procObjMan.ProcessTriangleAdded(object)) {
                object->m_createdObjects = true;
            }
        }

        if (a3 != 0xFAFAFAFA && a3 != (i % 8))
            continue;

        if (auto& object = entry.m_Objects[i]; object) {
            CVector cmp[] = {
                object->m_V1,
                object->m_V2,
                object->m_V3,
                (object->m_V1 + object->m_V2) / 2.0f,
                (object->m_V2 + object->m_V3) / 2.0f,
                (object->m_V1 + object->m_V3) / 2.0f
            };

            if (rng::none_of(cmp, [center](auto v) { return DistanceBetweenPoints(v, center) < 10000.0f; }))
                continue;

            object->Release();
            object = nullptr;
        }
    }
}

// 0x5DCF00
void CPlantMgr::_UpdateLocTris(const CVector& center, int32 a2) {
    for (auto i = m_CloseColEntListHead; i; i = i->m_NextEntry) {
        _ProcessEntryCollisionDataSections(*i, center, a2);
    }
}

// 0x5DBE40
float CPlantMgr::_CalcDistanceSqrToEntity(CEntity* entity, const CVector& posn) {
    const auto colModel = entity->GetColModel();
    CVector dst;
    entity->TransformFromObjectSpace(dst, colModel->m_boundSphere.m_vecCenter);

    auto d = DistanceBetweenPoints(dst, posn);
    if (auto r = colModel->m_boundSphere.m_fRadius; d > r) {
        d -= r;
    }

    return sq(d);
}

bool CPlantMgr::DbgCountCachedEntities(uint32*) {
    return true;
}

bool CPlantMgr::DbgCountLocTrisAndPlants(uint32, uint32*, uint32*) {
    return true;
}

bool CPlantMgr::DbgRenderCachedEntities(uint32*) {
    return true;
}

bool CPlantMgr::DbgRenderLocTris() {
    return true;
}
