#pragma once

#include "../../pch.h"

#define NULL_HANDLE 0xFFFFFFFF

namespace Halo1 {

    // === Classes created with ReClass.NET 1.2 by KN4CK3R ===

    class DeviceContainer {
        public:
        uint64_t vftable; //0x0000
        char gpuName[256]; //0x0008
        char pad_0108[992]; //0x0108
        ID3D11Device* pDevice; //0x04E8
        char pad_04F0[88]; //0x04F0
        uint32_t resolutionX; //0x0548
        uint32_t resolutionY; //0x054C
        char pad_0550[984]; //0x0550
    }; //Size: 0x0928

    class Camera {
        public:
        Vec3 pos; //0x0000
        char pad_000C[16]; //0x000C
        uint32_t N00000163; //0x001C
        Vec3 fwd; //0x0020
        Vec3 up; //0x002C
        float fov; //0x0038
    }; //Size: 0x003C

    // Thanks to Kavawuvi for documentation on the map format and Tag structure.
    class Tag {
        public:
        uint32_t fourCC_A; //0x0000
        uint32_t fourCC_B; //0x0004
        uint32_t fourCC_C; //0x0008
        uint32_t tagID; //0x000C
        uint32_t resourcePathAddress; //0x0010
        uint32_t dataAddress; //0x0014
        char pad_0018[8]; //0x0018

        char* getResourcePath();
    }; //Size: 0x0020

    class EntityList {
        public:
        char pad_0000[32]; //0x0000
        uint16_t capacity; //0x0020
        char pad_0022[14]; //0x0022
        uint16_t count; //0x0030
        int32_t entityListOffset; //0x0032
    }; //Size: 0x0036

    class Entity {
        public:
        uint32_t tagID; //0x0000
        char pad_0004[16]; //0x0004
        uint32_t ageMilis; //0x0014
        Vec3 pos; //0x0018
        Vec3 vel; //0x0024
        Vec3 fwd; //0x0030
        Vec3 up; //0x003C
        Vec3 angularVelocity; //0x0048
        char pad_0054[8]; //0x0054
        Vec3 eyePos; //0x005C
        char pad_0068[8]; //0x0068
        uint16_t entityCategory; //0x0070
        char pad_0072[14]; //0x0072
        uint32_t controllerHandle; //0x0080
        char pad_0084[8]; //0x0084
        uint16_t animId; //0x008C
        uint16_t animFrame; //0x008E
        char pad_0090[12]; //0x0090
        float health; //0x009C
        float shield; //0x00A0
        char pad_00A4[44]; //0x00A4
        uint32_t vehicleHandle; //0x00D0
        uint32_t childHandle; //0x00D4
        uint32_t parentHandle; //0x00D8
        char pad_00DC[292]; //0x00DC
        uint32_t projectileParentHandle; //0x0200
        float heat; //0x0204
        float plasmaUsed; //0x0208
        float fuse; //0x020C
        char pad_0210[12]; //0x0210
        float projectileAge; //0x021C
        char pad_0220[32]; //0x0220
        float plasmaCharge; //0x0240
        char pad_0244[61]; //0x0244
        uint8_t weaponIndex; //0x0281
        char pad_0282[1]; //0x0282
        uint8_t grenadeAnim; //0x0283
        uint8_t weaponAnim; //0x0284
        char pad_0285[1]; //0x0285
        uint16_t ammo; //0x0286
        char pad_0288[2]; //0x0288
        uint16_t clipAmmo; //0x028A
        char pad_028C[112]; //0x028C
        uint8_t frags; //0x02FC
        uint8_t plasmas; //0x02FD
        char pad_02FE[6]; //0x02FE
        uint32_t vehicleRiderHandle; //0x0304

        Tag* tag();
        char* getTagResourcePath();
        bool fromResourcePath( const char* str );
    }; //Size: 0x0308

    class EntityRecord {
        public:
        uint16_t id;
        uint16_t unknown_1;
        uint16_t unknown_2;
        uint16_t typeId;
        int32_t entityArrayOffset;

        Entity* entity();
    };

    // =======================================================

    enum EntityCategory {
        EntityCategory_Biped,
        EntityCategory_Vehicle,
        EntityCategory_Weapon,
        EntityCategory_Equipment,
        EntityCategory_Garbage,
        EntityCategory_Projectile,
        EntityCategory_Scenery,
        EntityCategory_Machine,
        EntityCategory_Control,
        EntityCategory_LightFixture,
        EntityCategory_Placeholder,
        EntityCategory_SoundScenery,
    };

    enum TypeID {
        TypeID_Player = 0x0DE4,
        TypeID_Marine = 0x0E58,
        TypeID_Jackal = 0x1184,
        TypeID_Grunt = 0x0CFC,
        TypeID_Elite = 0x1110,
        TypeID_VehicleA = 0x0AF4,
        TypeID_VehicleB = 0x06E0,
        TypeID_Projectile = 0x0290,
    };

    // =======================================================

    void init( UINT_PTR _dllBase );

    DeviceContainer* getDeviceContainerPointer();
    EntityList* getEntityListPointer();
    Camera* getPlayerCameraPointer();
    uint32_t getPlayerHandle();

    Tag* getTag( uint32_t tagID );

    uint64_t translateMapAddress( uint32_t address );

    typedef bool( __stdcall* EntityListEntryCallback ) ( EntityRecord* );
    void foreachEntityRecord( EntityListEntryCallback cb );

    Entity* getEntityPointer( EntityRecord* pRecord );
    EntityRecord* getEntityRecord( uint32_t entityHandle );
    EntityRecord* getEntityRecord( EntityList* pEntityList, uint32_t entityHandle );

    bool isPlayerHandle( uint32_t entityHandle );
    bool isPlayerControlled( EntityRecord* rec );

    bool isReloading( Entity* entity );
    bool isDoingMelee( Entity* entity );

    bool isTransport( Entity* entity );

    bool printEntity( EntityRecord* pRecord );
    void printEntities();

    extern float fovScale;
    extern float clippingNear;
    extern float clippingFar;
    HRESULT getCameraMatrix( float w, float h, XMMATRIX& result );
    Vec3 projectPoint( float w, float h, const Vec3 point );
    bool isCameraLoaded();

}