#include "./headers/XAudio2Hook.h"
#include "headers/XAudio2Methods.h"
#include "../utils/headers/Hook.h"

#include <xaudio2.h>

using std::make_unique;
using Hook::VirtualTableHook;

WAVEFORMATEX getDefaultFormat() {
    WAVEFORMATEX wfx{ 0 };
    wfx.wFormatTag = WAVE_FORMAT_PCM;
    wfx.nChannels = 1;
    wfx.nSamplesPerSec = 44100;
    wfx.wBitsPerSample = 8;
    wfx.nAvgBytesPerSec = wfx.wBitsPerSample * wfx.nSamplesPerSec / 8;
    wfx.nBlockAlign = ( wfx.nChannels * wfx.wBitsPerSample ) / 8;
    return wfx;
}

namespace XAudio2Hook {

    HRESULT( *CreateSourceVoice )(
        IXAudio2SourceVoice** ppSourceVoice,
        const WAVEFORMATEX* pSourceFormat,
        UINT32                     Flags,
        float                      MaxFrequencyRatio,
        IXAudio2VoiceCallback* pCallback,
        const XAUDIO2_VOICE_SENDS* pSendList,
        const XAUDIO2_EFFECT_CHAIN* pEffectChain
        );
    HRESULT onCreateSourceVoice(
        IXAudio2SourceVoice** ppSourceVoice,
        const WAVEFORMATEX* pSourceFormat,
        UINT32                     Flags,
        float                      MaxFrequencyRatio,
        IXAudio2VoiceCallback* pCallback,
        const XAUDIO2_VOICE_SENDS* pSendList,
        const XAUDIO2_EFFECT_CHAIN* pEffectChain
    ) {
        std::cout << "Creating source voice.\n";
        return CreateSourceVoice( ppSourceVoice, pSourceFormat, Flags, MaxFrequencyRatio, pCallback, pSendList, pEffectChain );
    }

    HRESULT( *SetVolume )(
        void* voice,
        float  Volume,
        UINT32 OperationSet
        );
    HRESULT onSetVolume(
        void* voice,
        float  Volume,
        UINT32 OperationSet
    ) {
        std::cout << "Volume was set to " << Volume << "\n";
        return SetVolume( voice, 0.0f, OperationSet );
    }

    std::vector<VHookPointer> hooks;

    HRESULT hook() {
        hooks.clear();

        IXAudio2* dummyInterface = nullptr;
        auto hr = XAudio2Create( &dummyInterface, 0, Processor1 );
        if ( FAILED( hr ) ) {
            std::cout << "Couldn't create dummy xaudio2 interface for hooking.\n";
            return hr;
        }

        void** vtable = *(void***) dummyInterface;

        std::cout << "Dummy: " << std::uppercase << std::hex << (uint64_t) dummyInterface << "\n";
        std::cout << "Dummy vtable: " << std::uppercase << std::hex << (uint64_t) vtable << "\n";


        hooks.emplace_back( make_unique<VirtualTableHook>(
            "CreateSourceVoice", vtable, (size_t) Methods_IXAudio2::CreateSourceVoice, (void*) onCreateSourceVoice, (void**) &CreateSourceVoice ) );

        IXAudio2MasteringVoice* dummyMasteringVoice = nullptr;
        hr = dummyInterface->CreateMasteringVoice( &dummyMasteringVoice, 4, 44100, 0, NULL, NULL );
        if ( !FAILED( hr ) && dummyMasteringVoice ) {
            void** dummyMasteringVoice_vtable = *(void***) dummyMasteringVoice;
            std::cout << "Dummy mastering voice vtable: " << std::uppercase << std::hex << (uint64_t) dummyMasteringVoice_vtable << "\n";

            // hooks.emplace_back( make_unique<VirtualTableHook>(
            //     "IXAudio2MasteringVoice::SetVolume", dummyMasteringVoice_vtable, (size_t) Methods_IXAudio2Voice::SetVolume, onSetVolume, (void**) &SetVolume ) );

            WAVEFORMATEX wfx = getDefaultFormat();
            IXAudio2SourceVoice* dummySourceVoice = nullptr;
            hr = dummyInterface->CreateSourceVoice( &dummySourceVoice, &wfx, 0, 2.0f, NULL, NULL, NULL );
            if ( !FAILED( hr ) && dummySourceVoice ) {
                void** dummySourceVoice_vtable = *(void***) dummySourceVoice;
                std::cout << "Dummy source voice vtable: " << std::uppercase << std::hex << (uint64_t) dummySourceVoice_vtable << "\n";

                hooks.emplace_back( make_unique<VirtualTableHook>(
                    "IXAudio2SourceVoice::SetVolume", dummySourceVoice_vtable, (size_t) Methods_IXAudio2Voice::SetVolume, onSetVolume, (void**) &SetVolume ) );

                dummySourceVoice->DestroyVoice();
            } else {
                std::cout << "Couldn't create dummy source voice.\n";
            }

            dummyMasteringVoice->DestroyVoice();
        } else {
            std::cout << "Couldn't create dummy mastering voice.\n";
        }

        dummyInterface->Release();
        return hr;
    }

    void cleanup() {
        hooks.clear();
    }

}