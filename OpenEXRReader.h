#ifndef DE_OPENEXR_READER_H
#define DE_OPENEXR_READER_H

#include <cstdint>
#include <OpenEXR/ImfArray.h>
#include <OpenEXR/ImfInputFile.h>
#include <OpenEXR/half.h>

using namespace Imf;

#pragma pack(push)
#pragma pack(1)

struct deUintRGB {
  uint32_t r;
  uint32_t g;
  uint32_t b;
};

struct deHalfRGB {
  half r;
  half g;
  half b;
};

struct deFloatRGB {
  float r;
  float g;
  float b;
};

#pragma pack(pop)

const char ptUINT[]  = "UINT";
const char ptHALF[]  = "HALF";
const char ptFLOAT[] = "FLOAT";
const char ptUNK[]   = "unknown";


class deOpenEXRReader {
public:
               ~deOpenEXRReader( void );
  bool          Load( char * filename );
  int           GetWidth( void ) const;
  int           GetHeight( void ) const;
  bool          IsUpsideDown( void ) const;
  PixelType     GetPixelType( void ) const;
  const char *  GetPixelTypeStr( void ) const;
  void *        GetDataOffset( int channelID ) const;
  uint32_t      GetPixelSize( void ) const;
  uint32_t      GetLineSize( void ) const;
  bool          FlipImage( void );
  static const char * PixelTypeString( PixelType pType );
private:
  void *     Data = nullptr;
  void *     DataStart = nullptr;
  uintptr_t  MainOffset = 0;
  int        CompSize = 0;
  bool       UpsideDown = false;
  bool       Loaded = false;
  PixelType  pixType;
  int        Width = 0;
  int        Height = 0;
  int        dx = 0;
  int        dy = 0;
  uint32_t   xStride = 0;
  uint32_t   yStride = 0;
};

#endif // DE_OPENEXR_READER_H
