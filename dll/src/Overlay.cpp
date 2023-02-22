#include "./headers/Overlay.h"
#include "headers/HaloMCC.h"
#include "headers/Halo1.h"
#include "utils/headers/common.h"
#include "utils/headers/Vec.h"
#include "utils/headers/MathUtils.h"
#include "graphics/headers/Renderer.h"
#include "graphics/headers/DX11Utils.h"
#include "graphics/headers/Colors.h"

using namespace DirectX;
using namespace MathUtils;
using namespace Halo1;

namespace Overlay {

    bool drawEntityOverlay( EntityRecord* rec );
    void drawPlayerTransformHUD( ID3D11DeviceContext* pCtx );
    bool trySelectEntity( EntityRecord* rec );

    Renderer* renderer;
    Vec2 screenDimensions;
    bool printSelectedEntity;
    EntityRecord* selectedEntity;
    const float selectionThreshold = 0.995f;
    const float drawDistance = 50.0f;
    bool overlayEnabled = true;

    void cleanup() {
        if ( renderer )
            delete renderer;
    }

    void onDllThreadUpdate() {
        toggleOption( "Overlay", overlayEnabled, VK_NUMPAD1 );
    }

    void render( ID3D11DeviceContext* pCtx, ID3D11Device* pDevice, IDXGISwapChain* pSwapChain ) {

        if ( !overlayEnabled || !isCameraLoaded() )
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

        printSelectedEntity = keypressed( 'P' );
        foreachEntityRecord( drawEntityOverlay );

        renderer->flush();

        drawPlayerTransformHUD( pCtx );

        renderer->end();

    }

    void draw3DTextCentered( Vec3 point, Vec2 offset, LPCWSTR text, Vec4 color, float fontSize, bool bordered ) {
        static uint32_t _flags = FW1_CENTER | FW1_TOP;

        Vec3 p = Halo1::projectPoint( screenDimensions.x, screenDimensions.y, point );
        if ( p.z > 0.0f ) {

            Vec2 pos = { p.x + offset.x, p.y + offset.y };

            if ( bordered ) {
                Vec4 colorBlack = { 0.0f, 0.0f, 0.0f, color.w };
                float r = 2.0f;
                float dx[4] = { 1.0f, 0.0f, -1.0f, 0.0f };
                float dy[4] = { 0.0f, 1.0f, 0.0f, -1.0f };
                for ( int i = 0; i < 4; i++ )
                    renderer->drawText( { pos.x + dx[i] * r, pos.y + dy[i] * r }, text, colorBlack, _flags, fontSize, nullptr );
            }

            renderer->drawText( pos, text, color, _flags, fontSize, nullptr );

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
            color = { 0.0f, 1.0f, 1.0f, 1.0f };
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
            draw3DTextCentered( pos, { 0, fontSize * ( lineNum++ ) }, buf, color, fontSize, isSelected ); \
            if ( printThisEntity ) \
                std::wcout << buf << "\n";\
        }

        if ( printThisEntity )
            std::cout << "\n";

        if ( !type.unknown )
            LINE( type.name );

        LINE( L"Type %04X", rec->typeId );
        LINE( L"%" PRIX64, (uint64_t) pEntity );

        if ( pEntity->entityCategory == EntityCategory_Projectile ) {
            LINE( L"Age %.4f", pEntity->projectileAge );
        }

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

    void drawPlayerTransformHUD( ID3D11DeviceContext* pCtx ) {
        static Vec3 vecZero = { 0.0f, 0.0f, 0.0f };
        static Vec3 vecX = { 1.0f, 0.0f, 0.0f };
        static Vec3 vecY = { 0.0f, 1.0f, 0.0f };
        static Vec3 vecZ = { 0.0f, 0.0f, 1.0f };

        D3D11_VIEWPORT viewport;
        float w = screenDimensions.y * 0.1f;
        float pad = 20.0f;
        viewport.Width = w;
        viewport.Height = w;
        viewport.TopLeftX = screenDimensions.x - w - pad;
        viewport.TopLeftY = screenDimensions.y - w - pad;
        viewport.MaxDepth = 1.0f;
        viewport.MinDepth = 0.0f;

        auto pCam = Halo1::getPlayerCameraPointer();
        auto pos = pCam->pos;

        char text[100];
        sprintf_s( text, "%.2f, %.2f, %.2f", pos.x, pos.y, pos.z );
        Vec2 textPos = { viewport.TopLeftX + 0.5f * w, screenDimensions.y };
        renderer->drawText( textPos, text, Colors::white, FW1_CENTER | FW1_BOTTOM, 10.0f, nullptr );
        renderer->flush();

        pCtx->RSSetViewports( 1, &viewport );

        XMMATRIX scaling = XMMatrixScaling( 1.0f, 1.0f, 0.5f );
        XMMATRIX translate = XMMatrixTranslation( 0.0f, 0.0f, 1.0f );
        XMMATRIX view = XMMatrixLookToRH( XMLoadFloat3( &vecZero ), XMLoadFloat3( &pCam->fwd ), { 0.0f, 0.0f, 1.0f, 0.0f } );
        XMMATRIX transform = view * translate * scaling;
        renderer->setTransform( &transform );

        renderer->setPrimitiveTopology( D3D11_PRIMITIVE_TOPOLOGY_LINELIST );
        Vertex vertices[] = {
            { vecZero, Colors::red }, { vecX, Colors::red },
            { vecZero, Colors::green }, { vecY, Colors::green },
            { vecZero, Colors::blue }, { vecZ, Colors::blue },
        };
        renderer->pushVerticies( ARRAYSIZE( vertices ), vertices );
        renderer->flush();

    }

}