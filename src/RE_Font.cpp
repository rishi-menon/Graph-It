#include "RE_Font.h"
#include <ft2build.h>
#include FT_FREETYPE_H

static FT_Library s_ftLib;
int Font::FontInst = 0;

Font g_fontDefault;

void Font::Init() {
    if (FontInst == 0) {
        if (FT_Init_FreeType(&s_ftLib)) {
            LogError("Error: Could not load free type");
        }
    }
    FontInst++;
}
void Font::Cleanup() {    
    FontInst--;
    if (FontInst == 0)
        FT_Done_FreeType(s_ftLib);

}

Font::Font() {
    Init();
    myFontFace = 0;
}
Font::Font(const char* path, uint32 height) {
    Init();
    bool b = Load(path, height);
    Assert(b);
}
Font::~Font() {
    if (myFontFace) {
        FT_Face* f = (FT_Face*)(myFontFace);
        FT_Done_Face(*f);

        delete f;
        myFontFace = 0;
    }
    for (int i = 0; i < CharCount; i++) {
        if (myMap[i]) {
            delete myMap[i];
            myMap[i] = nullptr;
        }
    }
    Cleanup();
}
bool Font::Load(const char* path, uint32 height) {
    if (myFontFace) {
        LogWarn("Font has already been loaded");
        return false;
    }

    FT_Face* pface = new FT_Face();
    myFontFace = pface;
    FT_Error err = FT_New_Face (s_ftLib, path, 0, pface);
    if (err) {
        LogError ("Font Error: FT_ERROR %d: Could not load font: %s", err, path);
        return false;
    }

    FT_Set_Pixel_Sizes(*pface, 0, height);
    return true;
}

bool Font::LoadSingleChar(int32 c) {
    Assert(c < CharCount);
    if (c >= CharCount || c < (int)(' ') || c == '\n') {
        return false;
    }
    if (myMap[c]) {
        LogWarn("Character %c (%d) has aldready been loaded", c, (int)(c));
        return false;
    }

    FT_Face& face = *(FT_Face*)myFontFace;
    if (FT_Load_Char (face, c, FT_LOAD_RENDER)) {
        LogError("Font Error: Could not load font character %c (%d)", c, (int)(c));
        Assert(false);
        return false;
    }

    size_t width = face->glyph->bitmap.width;
    size_t height = face->glyph->bitmap.rows;

    FontChar* fc = new FontChar;
    fc->Advance = face->glyph->advance.x >> 6; //advance is in 1/64th of a pixel
    fc->Character = c;
    fc->Size = glm::ivec2(width, height);
    fc->Bearing.x = face->glyph->bitmap_left;
    fc->Bearing.y = face->glyph->bitmap_top;
    
    //Load the texture and prepare the buffer
    if (c != ' ')
    {
        uint8* pixOut = 0;
        constexpr bool bCreateSingleChannel = false;
        if (bCreateSingleChannel) {
            //Todo: Flip the image vertically before creating the texture

            uint8* pTexData = face->glyph->bitmap.buffer;
            bool b = fc->Tex.Load(width, height, 1, pTexData, TextureParam(GL_REPEAT,GL_REPEAT, GL_LINEAR, GL_LINEAR, false));
            Assert(b);
            pixOut = pTexData;
        }
        else {
            uint8* pTexData = face->glyph->bitmap.buffer;
            int numPixels = width * height; 
            int size = numPixels * 4;
            uint8* buf = (uint8*) malloc(size);
            int k = 0;
            
            for (int y = height-1; y >= 0; y--) {
                for (int x = 0; x < width; x++) {
                    Assert(k+3 < size);
                    
                    int pixel = y*width+x;
                    buf[k++] = pTexData[pixel];
                    buf[k++] = 0;
                    buf[k++] = 0;
                    buf[k++] = 0;
                }
            }

            bool b = fc->Tex.Load(width, height, 4, buf, TextureParam(GL_REPEAT,GL_REPEAT, GL_LINEAR, GL_LINEAR, false));
            Assert(b);
            free(buf);

            pixOut = buf;
        }
        
        //Create a bmp for testing
        // char path[100];
        // sprintf(path, "temp/char_%c.bmp", (char)(c));
        // CreateBMP(path, width, height, fc->Tex.channel(), pixOut);
        
    }
    myMap[c] = fc;
    // FT_Done_Face(face);

    return true;
}

FontChar* Font::Get(int32 c) {
    Assert(c < CharCount);
    if (!myFontFace || c >= CharCount) {
        return nullptr;
    }
    //Load character if it hasn't already been loaded
    if (!myMap[c])
        LoadSingleChar(c);
    
    return myMap[c];
}
