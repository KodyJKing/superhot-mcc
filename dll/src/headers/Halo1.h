#include "../../pch.h"

namespace Halo1 {

    // Created with ReClass.NET 1.2 by KN4CK3R

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
        char pad_0064[56]; //0x0064
        float health; //0x009C
        float shield; //0x00A0
        char pad_00A4[600]; //0x00A4
        uint8_t frags; //0x02FC
        uint8_t plasmas; //0x02FD
    }; //Size: 0x02FE

    void init( UINT_PTR _dllBase );

    DeviceContainer* getDeviceContainerPointer();
    EntityList* getEntityListPointer();
    Camera* getPlayerCameraPointer();

    typedef bool( __stdcall* EntityListEntryCallback ) ( EntityRecord* );
    void foreachEntityRecord( EntityListEntryCallback cb );

    Entity* getEntityPointer( EntityRecord* pRecord );
    EntityRecord* getEntityRecord( uint32_t i );
    EntityRecord* getEntityRecord( EntityList* pEntityList, uint32_t i );

}