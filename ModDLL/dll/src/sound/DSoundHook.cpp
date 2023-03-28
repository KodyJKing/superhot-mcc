#include "headers/DSoundHook.h"
#include "headers/DSoundMethodOffsets.h"
#include "../utils/headers/Hook.h"

using Hook::VirtualTableHook;
using std::make_unique;

namespace DSoundHook {

    static HRESULT( *CreateSoundBuffer ) (
        IDirectSound* pDirectSound,
        LPCDSBUFFERDESC lpcDSBufferDesc,
        LPLPDIRECTSOUNDBUFFER lplpDirectSoundBuffer,
        IUnknown FAR* pUnkOuter
        );
    HRESULT onCreateSoundBuffer(
        IDirectSound* pDirectSound,
        LPCDSBUFFERDESC lpcDSBufferDesc,
        LPLPDIRECTSOUNDBUFFER lplpDirectSoundBuffer,
        IUnknown FAR* pUnkOuter
    ) {
        std::cout << "Creating sound buffer!\n";
        return CreateSoundBuffer( pDirectSound, lpcDSBufferDesc, lplpDirectSoundBuffer, pUnkOuter );
    }

    static std::vector<VHookPointer> hooks;

    void hook() {
        LPDIRECTSOUND8 dsoundDummy = nullptr;
        HRESULT err = DirectSoundCreate8( NULL, &dsoundDummy, NULL );

        void** vtable = *(void***) ( dsoundDummy );
        std::cout << "IDirectSound8 vtable at: " << std::uppercase << std::hex << (uint64_t) vtable << "\n";

        hooks.clear();

        hooks.emplace_back( make_unique<VirtualTableHook>(
            "CreateSoundBuffer", vtable, MO_IDirectSound8::CreateSoundBuffer, onCreateSoundBuffer, (void**) &CreateSoundBuffer ) );

        dsoundDummy->Release();
    }

    void cleanup() {
        hooks.clear();
    }

}