#pragma once
#include <GL/glew.h>
#include "DebugFinal.h"

void CreateBMP (const char* path, int width, int height, int channel, const uint8* data);

struct TextureParam {
    TextureParam();
    TextureParam(uint32 wrapS, uint32 wrapT, uint32 filterMin, uint32 filterMax, bool MipMaps);

    unsigned int wrapS;
    unsigned int wrapT;
    unsigned int filterMin;
    unsigned int filterMax;
    bool MipMaps;
};

class Texture {
public:
    ~Texture();
    Texture() = default;
    Texture(const Texture&) = delete;
    const Texture& operator= (const Texture&) = delete;
    
    Texture(const char* path, TextureParam param = TextureParam());
    Texture(int32 width, int32 height, int32 channel, uint8* data, TextureParam param = TextureParam());

    bool Load(const char* path, TextureParam param = TextureParam());
    bool Load(int32 width, int32 height, int32 channel, uint8* data, TextureParam param = TextureParam());

    void Bind(uint32 slot = 0) const;
    unsigned int id() const { return m_id; }

    int32 width() const { return m_width; }
    int32 height() const { return m_height; }
    int32 channel() const { return m_channel; }

    // bool Save(const char* path);

private:
    unsigned int m_id;
    int32 m_width, m_height, m_channel;

};

