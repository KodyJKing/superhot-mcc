namespace MO_IDirectSound8 {
    // IUnknown methods
    inline const int QueryInterface = 0;
    inline const int AddRef = 1;
    inline const int Release = 2;

    // IDirectSound methods
    inline const int CreateSoundBuffer = 3;
    inline const int GetCaps = 4;
    inline const int DuplicateSoundBuffer = 5;
    inline const int SetCooperativeLevel = 6;
    inline const int Compact = 7;
    inline const int GetSpeakerConfig = 8;
    inline const int SetSpeakerConfig = 9;
    inline const int Initialize = 10;

    // IDirectSound8 methods
    inline const int VerifyCertification = 11;
}