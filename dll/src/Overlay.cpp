#include "./headers/Overlay.h"
#include "headers/HaloMCC.h"
#include "headers/Halo1.h"
#include "utils/headers/common.h"
#include "utils/headers/Vec.h"
#include "utils/headers/MathUtils.h"
#include "graphics/headers/Renderer.h"
#include "graphics/headers/DX11Utils.h"
#include "graphics/headers/Colors.h"

using namespace MathUtils;
using namespace Halo1;

namespace Overlay {

    bool drawEntityOverlay( EntityRecord* rec );
    bool trySelectEntity( EntityRecord* rec );

    Renderer* renderer;
    Vec2 screenDimensions;
    bool printSelectedEntity;
    EntityRecord* selectedEntity;
    const float selectionThreshold = 0.995f;
    const float drawDistance = 50.0f;

    void cleanup() {
        if ( renderer )
            delete renderer;
    }

    void render( ID3D11DeviceContext* pCtx, ID3D11Device* pDevice, IDXGISwapChain* pSwapChain ) {

        if ( !isCameraLoaded() )
            return;

        if ( !renderer ) {
            std::cout << "Constructing new Renderer.\n";
            renderer = new Renderer( pDevice, 4096 );
        }

        XMMATRIX transform;
        screenDimensions = HaloMCC::getWindowSize();
        if ( FAILED( Halo1::getCameraMatrix( screenDimensions.x, screenDimensions.y, transform ) ) )
            return;

        renderer->setTransform( &transform );

        fitViewportToWindow( pCtx, HaloMCC::getWindow() );
        renderer->setPrimitiveTopology( D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST );
        renderer->begin();

        selectedEntity = nullptr;
        foreachEntityRecord( trySelectEntity );
        printSelectedEntity = keypressed( VK_F4 );
        foreachEntityRecord( drawEntityOverlay );

        // Vec4 color = { 1.0f, 1.0f, 1.0f, 0.25f };
        // renderer->drawText( { 500, 100 }, L"Hello Text!", color, NULL, 100.0f, nullptr );

        renderer->flush();
        renderer->end();

    }

    void draw3DText( Vec3 point, Vec2 offset, LPCWSTR text, Vec4 color, uint32_t flags, float fontSize, LPCWSTR fontFamily ) {
        Vec3 p = Halo1::projectPoint( screenDimensions.x, screenDimensions.y, point );
        if ( p.z > 0.0f )
            renderer->drawText( { p.x + offset.x, p.y + offset.y }, text, color, NULL, fontSize, nullptr );
    }

    void draw3DText( Vec3 point, LPCWSTR text, Vec4 color, uint32_t flags, float fontSize, LPCWSTR fontFamily ) {
        draw3DText( point, { 0.0f, 0.0f }, text, color, flags, fontSize, fontFamily );
    }

    void draw3DTextCentered( Vec3 point, Vec2 offset, LPCWSTR text, Vec4 color, uint32_t flags, float fontSize, LPCWSTR fontFamily ) {
        Vec3 p = Halo1::projectPoint( screenDimensions.x, screenDimensions.y, point );
        if ( p.z > 0.0f ) {
            auto dims = renderer->measureText( text, fontSize, fontFamily );
            renderer->drawText( { p.x + offset.x - dims.x * 0.5f, p.y + offset.y }, text, color, NULL, fontSize, nullptr );
        }
    }

    bool shouldDisplay( EntityRecord* rec ) {
        auto pEntity = getEntityPointer( rec );

        auto pCam = getPlayerCameraPointer();
        auto diff = Vec::sub( pEntity->pos, pCam->pos );
        if ( Vec::length( diff ) > drawDistance )
            return false;

        switch ( pEntity->entityCategory ) {
            case EntityCategory_Biped:
            case EntityCategory_Vehicle:
            case EntityCategory_Weapon:
            case EntityCategory_Projectile:
                return true;
            default:
                return false;
        }

        // if ( pEntity->health <= 0 )
        //     return false;
    }

    bool drawEntityOverlay( EntityRecord* rec ) {
        if ( !shouldDisplay( rec ) )
            return true;

        auto pEntity = getEntityPointer( rec );
        auto type = getEntityType( rec->typeId );
        auto pos = pEntity->pos;

        bool isSelected = rec == selectedEntity;
        bool printThisEntity = isSelected && printSelectedEntity;

        Vec4 color;
        float fontSize;
        if ( isSelected ) {
            color = { 0.0f, 1.0f, 1.0f, 0.5f };
            fontSize = 20.0f;
        } else {
            if ( pEntity->health > 0 )
                color = { 0.25f, 1.0f, 0.25f, 0.25f };
            else
                color = { 1.0f, 1.0f, 1.0f, 0.25f };
            fontSize = 8.0f;
        }

        wchar_t buf[100];
        int lineNum = 0;
        #define LINE(format, ...) { \
            swprintf_s( buf, format, __VA_ARGS__ ); \
            draw3DTextCentered( pos, { 0, fontSize * ( lineNum++ ) }, buf, color, NULL, fontSize, nullptr ); \
            if ( printThisEntity ) \
                std::wcout << buf << "\n";\
        }

        if ( printThisEntity )
            std::cout << "\n";

        // LINE( L"%.2f, %.2f, %.2f", pos.x, pos.y, pos.z );
        // LINE( L"HP %.2f SP %0.2f", pEntity->health, pEntity->shield );
        LINE( L"%" PRIX64, (uint64_t) pEntity );
        LINE( L"Type %04X", rec->typeId );
        if ( !type.unknown )
            LINE( type.name );

        #undef LINE

        return true;
    }


    float getSelectionScore( EntityRecord* rec ) {
        auto pEntity = getEntityPointer( rec );
        auto pCam = getPlayerCameraPointer();
        return Vec::dot(
            Vec::unit( Vec::sub( pEntity->pos, pCam->pos ) ),
            pCam->fwd
        );
    }

    bool trySelectEntity( EntityRecord* rec ) {
        if ( !shouldDisplay( rec ) )
            return true;

        float score = getSelectionScore( rec );
        if ( score < selectionThreshold )
            return true;

        if ( !selectedEntity ) {
            selectedEntity = rec;
            return true;
        }

        float oldScore = getSelectionScore( selectedEntity );
        if ( score > oldScore )
            selectedEntity = rec;

        return true;
    }

}