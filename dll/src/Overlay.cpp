#include "./headers/Overlay.h"
#include "headers/HaloMCC.h"
#include "headers/Halo1.h"
#include "utils/headers/common.h"
#include "utils/headers/Vec.h"
#include "utils/headers/MathUtils.h"
#include "utils/headers/StringUtils.h"
#include "graphics/headers/Renderer.h"
#include "graphics/headers/DX11Utils.h"
#include "graphics/headers/Colors.h"

using namespace DirectX;
using namespace MathUtils;
using namespace Halo1;

namespace Overlay {

    // Declarations
    bool drawEntityOverlay( EntityRecord* rec );
    void drawPlayerTransformHUD( ID3D11DeviceContext* pCtx );
    bool trySelectEntity( EntityRecord* rec );

    // Constants
    static const float selectionThreshold = 0.995f;
    static const float drawDistance = 50.0f;
    // State
    static Renderer* renderer;
    static Vec2 screenDimensions;
    static bool printSelectedEntity;
    static EntityRecord* selectedEntity;
    // Options
    static bool overlayEnabled = true;
    static bool onlyShowSelected = false;

    void cleanup() {
        if ( renderer )
            delete renderer;
    }

    void onDllThreadUpdate() {
        toggleOption( "Overlay", overlayEnabled, VK_NUMPAD1 );
        toggleOption( "Only show selected", onlyShowSelected, VK_NUMPAD2 );
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

        if ( HaloMCC::isInForeground() )
            printSelectedEntity = keypressed( 'P' );
        foreachEntityRecord( drawEntityOverlay );

        renderer->flush();

        drawPlayerTransformHUD( pCtx );

        renderer->end();

    }

    void draw3DTextCentered( Vec3 point, Vec2 offset, LPCSTR text, Vec4 color, float fontSize, bool bordered ) {
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
            case EntityCategory_Machine:
                return true;
            default:
                return false;
        }
    }

    bool drawEntityOverlay( EntityRecord* rec ) {
        if ( !shouldDisplay( rec ) )
            return true;

        auto pEntity = getEntityPointer( rec );
        auto pos = pEntity->pos;

        bool isSelected = rec == selectedEntity;
        bool printThisEntity = isSelected && printSelectedEntity;

        if ( onlyShowSelected && !isSelected )
            return true;

        Vec4 color;
        float fontSize;
        if ( isSelected ) {
            fontSize = 20.0f;
            color = { 1.0f, 1.0f, 0.0f, 1.0f };
        } else {
            fontSize = 8.0f;
            color = pEntity->health > 0 ?
                Vec4{ 0.25f, 1.0f, 0.25f, 0.25f } :
                Vec4{ 1.0f, 1.0f, 1.0f, 0.25f };
        }

        std::stringstream overlayText;

        auto tag = pEntity->tag();
        if ( tag ) {
            auto resourceName = tag->getResourcePath();
            if ( StringUtils::checkCStr( resourceName, 256 ) ) {
                overlayText << resourceName << "\n";
                // auto parts = StringUtils::split( resourceName, "\\" );
                // if ( parts.size() >= 3 )
                //     overlayText << parts[2] << "\n";
            }
        }

        overlayText << std::uppercase << std::hex << rec->typeId << " ";
        overlayText << std::uppercase << std::hex << (uint64_t) pEntity << "\n";

        std::string overlayStr = overlayText.str();
        draw3DTextCentered( pos, {}, overlayStr.c_str(), color, fontSize, isSelected );
        if ( printThisEntity )
            std::cout << "\n" << overlayStr.c_str();

        if ( printThisEntity ) {
            std::stringstream pointerText;
            pointerText << std::uppercase << std::hex << (uint64_t) pEntity;
            copyANSITextToClipboard( pointerText.str().c_str() );
        }

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
        renderer->drawText( textPos, text, Colors::white, FW1_CENTER | FW1_BOTTOM, 12.0f, nullptr );
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