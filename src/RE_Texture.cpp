#include "RE_Texture.h"
#include <stb_image.h>

#include "Maths.h"
#include "RE_Buffers.h"
#include <fstream>


static void CreateBMP_BGR (const char* path, int width, int height, uint8* data);
static uint8* createBitmapFileHeader (int height, int stride);
static uint8* createBitmapInfoHeader (int height, int width);

TextureParam::TextureParam() :
    wrapS(GL_REPEAT),
    wrapT(GL_REPEAT), 
    filterMin(GL_LINEAR), 
    filterMax(GL_LINEAR), 
    MipMaps(false)
{    
}

TextureParam::TextureParam(uint32 wrapS, uint32 wrapT, uint32 filterMin, uint32 filterMax, bool MipMaps) :
    wrapS(wrapS), 
    wrapT(wrapT), 
    filterMin(filterMin), 
    filterMax(filterMax), 
    MipMaps(MipMaps)
{
}



Texture::Texture(const char* path, TextureParam param) {
    bool b = Load(path, param);
    Assert(b);
}
Texture::Texture(int32 width, int32 height, int32 channel, uint8* data, TextureParam param) {
    bool b = Load(width, height, channel, data);
    Assert(b);
}

Texture::~Texture() {
    glDeleteTextures(1, &m_id);
}
void Texture::Bind(uint32 slot) const {
    Assert(slot < 32);
    glActiveTexture(GL_TEXTURE0+slot);
    glBindTexture(GL_TEXTURE_2D, m_id);
}

bool Texture::Load(const char* path, TextureParam param) {
    stbi_set_flip_vertically_on_load(true);

    int32 w, h, ch=0;
    uint8 *data = stbi_load(path, &w, &h, &ch, 0);

    LogWarn("Tex: %dx%d channels: %d", w, h, ch);
    if (data)
    {
        bool bStatus = Load(w, h, ch, data, param);
        stbi_image_free(data);
        return bStatus;
    }
    else
    {
        LogError("Failed to load texture with path: %s", path);
        Assert(false);
        return false;
    }
}
bool Texture::Load(int32 width, int32 height, int32 channel, uint8* data, TextureParam param) {
    m_width = width;
    m_height = height;
    m_channel = channel;

    glGenTextures(1, &m_id);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, m_id);
    glCheckError();

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, param.wrapS); 
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, param.wrapT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, param.filterMin);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, param.filterMax);
    glCheckError();

    GLint internalFormat = GL_RGBA;
    switch(m_channel) {
        case 1: internalFormat = GL_R8;    break;
        case 2: internalFormat = GL_RG8;   break;
        case 3: internalFormat = GL_RGB8;  break;
        case 4: internalFormat = GL_RGBA8; break;
        default: Assert(! "Invalid number of channels"); break;
    }
    internalFormat = GL_RGBA8;
    
    GLint format = GL_RGBA;
    switch(m_channel) {
        case 1: format = GL_RED;    break;
        case 2: format = GL_RG;   break;
        case 3: format = GL_RGB;  break;
        case 4: format = GL_RGBA; break;
        default: Assert(! "Invalid number of channels"); break;
    }

    glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, m_width, m_height, 0, format, GL_UNSIGNED_BYTE, data);
    glCheckError();

    if (param.MipMaps) {
        glGenerateMipmap(GL_TEXTURE_2D);
    }
    glCheckError();
    return true;
}

// bool IsLittleEndian() {
//     int32_t a = 0x120034;
//     char b[4];
//     memcpy(b, &a, 4);
//     printf("%x", (int)b[0]);
//     return b[0] == 0x34;
// }

// int32 swap(int32 a) {
//     return __builtin_bswap32(a);
// }
// int64 swap(int64 a) {
//     return __builtin_bswap64(a);
// }
// int16 swap(int16 a) {
//     return __builtin_bswap16(a);
// }

// int32 swapLE(int32 a) {
//     return IsLittleEndian() ? a : swap(a);
// }

// bool Texture::Save(const char* path) {
    
// }


/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////

void CreateBMP (const char* path, int width, int height, int channel, const uint8* data) {
    //Convert it to an bgr buffer.. alpha values are ignore since its a bmp
    if (channel < 1 || channel > 4) {
        Assert(false);
        return;
    }

    int maxSize = width*height*3;
    uint8* buf = (uint8*)malloc(maxSize);
    int index = 0;
    
    for (int i = 0; i < height; i++) {
        const uint8* row = data + i * width*channel;
        for (int j = 0; j < width; j++) {
            Assert(index+2 < maxSize);
            const uint8* pixel = row + j*channel;
            uint8 b, g, r;

            switch (channel) {
                case 4:
                case 3:
                {
                    r=pixel[0]; g=pixel[1]; b=pixel[2]; 
                    break;
                }
                case 2:
                {
                    r=pixel[0]; g=pixel[1]; b=0;
                    break;
                }
                case 1:
                {
                    r=pixel[0]; g=r; b=r;
                    break;
                }
            }
            
            buf[index++] = b;
            buf[index++] = g;
            buf[index++] = r;
        }
    }

    CreateBMP_BGR(path, width, height, buf);
    free(buf);
}


///////////////////////////////////////////////////////////////////////////////////////
//////      Code to create BMP: https://stackoverflow.com/a/47785639


static const int BYTES_PER_PIXEL = 3;   // red, green, & blue
static const int FILE_HEADER_SIZE = 14;
static const int INFO_HEADER_SIZE = 40;

static void CreateBMP_BGR (const char* path, int width, int height, uint8* data)
{
    int widthInBytes = width * BYTES_PER_PIXEL;

    uint8 padding[3] = {0, 0, 0};
    int paddingSize = (4 - (widthInBytes%4)) % 4;
    int stride = widthInBytes + paddingSize;

    FILE* imageFile = fopen(path, "wb");

    uint8* fileHeader = createBitmapFileHeader(height, stride);
    fwrite(fileHeader, 1, FILE_HEADER_SIZE, imageFile);

    uint8* infoHeader = createBitmapInfoHeader(height, width);
    fwrite(infoHeader, 1, INFO_HEADER_SIZE, imageFile);

    // char* widthBuf = malloc(widthInBytes);  
    for (int i = 0; i < height; i++) {
        uint8* row = data + i * widthInBytes;
        // int index = 0;
        // for (int j = 0; j < widthInBytes; j += 3) {
        //     widthBuf[index++] = row[j+2];  //blue
        //     widthBuf[index++] = row[j+1];  //green
        //     widthBuf[index++] = row[j];    //red
        // }
        fwrite(row, 1, widthInBytes, imageFile);
        fwrite(padding, 1, paddingSize, imageFile);
    }
    // free(widthBuf);

    fclose(imageFile);
}

static uint8* createBitmapFileHeader (int height, int stride)
{
    int fileSize = FILE_HEADER_SIZE + INFO_HEADER_SIZE + (stride * height);

    static uint8 fileHeader[] = {
        0,0,     /// signature
        0,0,0,0, /// image file size in bytes
        0,0,0,0, /// reserved
        0,0,0,0, /// start of pixel array
    };

    fileHeader[ 0] = (uint8)('B');
    fileHeader[ 1] = (uint8)('M');
    fileHeader[ 2] = (uint8)(fileSize      );
    fileHeader[ 3] = (uint8)(fileSize >>  8);
    fileHeader[ 4] = (uint8)(fileSize >> 16);
    fileHeader[ 5] = (uint8)(fileSize >> 24);
    fileHeader[10] = (uint8)(FILE_HEADER_SIZE + INFO_HEADER_SIZE);

    return fileHeader;
}

static uint8* createBitmapInfoHeader (int height, int width)
{
    static uint8 infoHeader[] = {
        0,0,0,0, /// header size
        0,0,0,0, /// image width
        0,0,0,0, /// image height
        0,0,     /// number of color planes
        0,0,     /// bits per pixel
        0,0,0,0, /// compression
        0,0,0,0, /// image size
        0,0,0,0, /// horizontal resolution
        0,0,0,0, /// vertical resolution
        0,0,0,0, /// colors in color table
        0,0,0,0, /// important color count
    };

    infoHeader[ 0] = (uint8)(INFO_HEADER_SIZE);
    infoHeader[ 4] = (uint8)(width      );
    infoHeader[ 5] = (uint8)(width >>  8);
    infoHeader[ 6] = (uint8)(width >> 16);
    infoHeader[ 7] = (uint8)(width >> 24);
    infoHeader[ 8] = (uint8)(height      );
    infoHeader[ 9] = (uint8)(height >>  8);
    infoHeader[10] = (uint8)(height >> 16);
    infoHeader[11] = (uint8)(height >> 24);
    infoHeader[12] = (uint8)(1);
    infoHeader[14] = (uint8)(BYTES_PER_PIXEL*8);

    return infoHeader;
}