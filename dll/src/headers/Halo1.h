#include "../../pch.h"

#define ENTITY_TYPE_MAP std::map<uint16_t, EntityType*>

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

    class EntityRecord {
        public:
        uint16_t id;
        uint16_t unknown_1;
        uint16_t unknown_2;
        uint16_t typeId;
        int32_t entityArrayOffset;
    };

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
        char pad_0000[24]; //0x0000
        Vec3 pos; //0x0018
        Vec3 vel; //0x0024
        char pad_0030[40]; //0x0030
        Vec3 eyePos; //0x0058
        char pad_0064[8]; //0x0064
        int32_t N0000018A; //0x006C
        uint16_t entityCategory; //0x0070
        char pad_0072[10]; //0x0072
        uint32_t N0000018C; //0x007C
        uint32_t controllerHandle; //0x0080
        char pad_0084[24]; //0x0084
        float health; //0x009C
        float shield; //0x00A0
        char pad_00A4[48]; //0x00A4
        uint32_t N00000197; //0x00D4
        uint32_t parentHandle; //0x00D8
        char pad_00DC[288]; //0x00DC
        uint32_t N000001BC; //0x01FC
        uint32_t projectileParentHandle; //0x0200
        char pad_0204[248]; //0x0204
        uint8_t frags; //0x02FC
        uint8_t plasmas; //0x02FD
        uint16_t N000001D8; //0x02FE
        uint32_t N000001FB; //0x0300
        uint32_t vehicleRiderHandle; //0x0304
    }; //Size: 0x0308

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
        TypeID_Projectile = 0x0290
    };

    struct EntityType {
        const wchar_t* name;
        bool living;
        bool hostile;
        bool transport;
        bool unknown;
    };

    EntityType getEntityType( uint16_t typeId );
    EntityType getEntityType( TypeID type );

    void init( UINT_PTR _dllBase );

    DeviceContainer* getDeviceContainerPointer();
    EntityList* getEntityListPointer();
    Camera* getPlayerCameraPointer();

    typedef bool( __stdcall* EntityListEntryCallback ) ( EntityRecord* );
    void foreachEntityRecord( EntityListEntryCallback cb );

    Entity* getEntityPointer( EntityRecord* pRecord );
    EntityRecord* getEntityRecord( uint32_t entityHandle );
    EntityRecord* getEntityRecord( EntityList* pEntityList, uint32_t entityHandle );

    bool isPlayerHandle( uint32_t entityHandle );
    bool isPlayerControlled( uint32_t entityHandle );

    bool printEntity( EntityRecord* pRecord );
    void printEntities();

    HRESULT getCameraMatrix( float w, float h, XMMATRIX& result );
    Vec3 projectPoint( float w, float h, const Vec3 point );
    bool isCameraLoaded();

}