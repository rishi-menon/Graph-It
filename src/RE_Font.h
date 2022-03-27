#pragma once
#include <unordered_map>
#include "DebugFinal.h"
#include "RE_Buffers.h"
#include "RE_Texture.h"
#include "Maths.h"

class Font;
extern Font g_fontDefault;


struct FontChar
{
    Texture Tex;
    unsigned int Advance;
    int32 Character;
    glm::ivec2 Size;
    glm::ivec2 Bearing;
};
class Font {
public:
    Font();
    ~Font();

    Font(const char* path, uint32 height);
    bool Load(const char* path, uint32 height);

    FontChar* Get(int32 c);

private:
    bool LoadSingleChar(int32 c);

    static void Init();
    static void Cleanup();
    static int FontInst;
private:
    //Pointer to 
    void* myFontFace;   
    // std::unordered_map<int, FontChar*> myMap;
    constexpr static int CharCount = 256;
    FontChar* myMap[CharCount] = {};
};