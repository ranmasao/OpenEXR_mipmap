#include "OpenEXRReader.h"
#include <stdio.h>

#include <OpenEXR/ImfBoxAttribute.h>
#include <OpenEXR/ImfChannelList.h>
#include <OpenEXR/ImfHeader.h>

using namespace Imath;


deOpenEXRReader::~deOpenEXRReader( void ) {
  if ( Data != nullptr ) {
    switch ( pixType ) {
      case UINT: {
        Array2D<deUintRGB> * array = ( Array2D<deUintRGB> * )Data;
        delete array;
        break;
      }
      case HALF: {
        Array2D<deHalfRGB> * array = ( Array2D<deHalfRGB> * )Data;
        delete array;
        break;
      }
      case FLOAT: {
        Array2D<deFloatRGB> * array = ( Array2D<deFloatRGB> * )Data;
        delete array;
        break;
      }
      default:
        ;// is it possible to get here?
    }
  }
}


bool deOpenEXRReader::Load( char * filename ) {
  Loaded = false;
  InputFile * file;
  try {
    // open file
    file = new InputFile( filename );
    
    // check channels presence and type
    const ChannelList & channels = file->header().channels();
    const Channel * ChannelR = channels.findChannel( "R" );
    const Channel * ChannelG = channels.findChannel( "G" );
    const Channel * ChannelB = channels.findChannel( "B" );
    if ( ( ChannelR == nullptr ) || ( ChannelG == nullptr ) || 
         ( ChannelB == nullptr ) )
      throw Iex::InputExc( "Only RGB images are supported now." );
    if ( ( ChannelR->type != ChannelG->type ) || 
         ( ChannelR->type != ChannelB->type ) )
      throw Iex::InputExc( "Ultra-rare: different RGB component types." );
    pixType = ChannelR->type;
    
    // get line order
    switch ( file->header().lineOrder() ) {
      case INCREASING_Y: {
        UpsideDown = true;
        break;
      }
      case DECREASING_Y: {
        UpsideDown = false;
        break;
      }
      default:
        throw Iex::InputExc( "Strange lines order." );
    }
    
    // get image sizes
    Box2i dw = file->header().dataWindow();
    Width = dw.max.x - dw.min.x + 1;
    Height = dw.max.y - dw.min.y + 1;
    dx = dw.min.x;
    dy = dw.min.y;
    
    // allocate buffer and set interleaving
    switch ( pixType ) {
      case UINT: {
        Array2D<deUintRGB> * array = new Array2D<deUintRGB>( Width, Height );
        Data = array;
        DataStart = array[ 0 ][ 0 ];
        xStride = sizeof( deUintRGB );
        CompSize = sizeof( uint32_t );
        break;
      }
      case HALF: {
        Array2D<deHalfRGB> * array = new Array2D<deHalfRGB>( Width, Height );
        Data = array;
        DataStart = array[ 0 ][ 0 ];
        xStride = sizeof( deHalfRGB );
        CompSize = sizeof( half );
        break;
      }
      case FLOAT: {
        Array2D<deFloatRGB> * array = new Array2D<deFloatRGB>( Width, Height );
        Data = array;
        DataStart = array[ 0 ][ 0 ];
        xStride = sizeof( deFloatRGB );
        CompSize = sizeof( float );
        break;
      }
      default: 
        throw Iex::InputExc( "Unsupported pixel type." );
    }
    yStride = xStride * Width;
    MainOffset = ( uintptr_t )DataStart - ( dy * yStride + dx * xStride );
    FrameBuffer frameBuffer;
    frameBuffer.insert( "R", 
                        Slice( pixType, ( char * )GetDataOffset( 0 ),
                               xStride, yStride ) );
    frameBuffer.insert( "G", 
                        Slice( pixType, ( char * )GetDataOffset( 1 ),
                               xStride, yStride ) );
    frameBuffer.insert( "B", 
                        Slice( pixType, ( char * )GetDataOffset( 2 ),
                               xStride, yStride ) );
    file->setFrameBuffer( frameBuffer );
    
    // read data
    file->readPixels( dw.min.y, dw.max.y );
    Loaded = true;
  }
  catch ( Iex::BaseExc & e ) {
    printf( "Caught exception: %s\n", e.what() );
    printf( "Can't load file '%s' correctly!\n", filename );
  }
  if ( file != nullptr )
    delete file;
  if ( Loaded ) { // if everything was OK
    // nothing to do here yet
  }
  return Loaded;
}

int deOpenEXRReader::GetWidth( void ) const {
  if ( Loaded )
    return Width;
  else
    return 0;
}

int deOpenEXRReader::GetHeight( void ) const {
  if ( Loaded )
    return Height;
  else
    return 0;
}

bool deOpenEXRReader::IsUpsideDown( void ) const {
  return UpsideDown;
}

PixelType deOpenEXRReader::GetPixelType( void ) const {
  if ( Loaded )
    return pixType;
  else
    return NUM_PIXELTYPES;
}

bool deOpenEXRReader::FlipImage( void ) {
  if ( !Loaded )
    return false;
  uint64_t dataSize = Height * yStride;
  void * row1 = DataStart;
  void * row2 = ( void * )( ( uintptr_t )DataStart + dataSize - yStride );
  void * tmpRow = malloc( yStride );
  while ( row1 < row2 ) {
    memcpy( tmpRow, row1,   yStride );
    memcpy( row1,   row2,   yStride );
    memcpy( row2,   tmpRow, yStride );
    row1 = ( void * )( ( uintptr_t )row1 + yStride );
    row2 = ( void * )( ( uintptr_t )row2 - yStride );
  }
  free( tmpRow );
  UpsideDown = !UpsideDown;
  return true;
}

const char * deOpenEXRReader::GetPixelTypeStr() const {
  return PixelTypeString( pixType );
}

const char * deOpenEXRReader::PixelTypeString(PixelType pType){
  switch ( pType ) {
  case UINT:
    return ptUINT;
  case HALF:
    return ptHALF;
  case FLOAT:
    return ptFLOAT;
  default:
    return ptUNK;
  }
}

void * deOpenEXRReader::GetDataOffset( int channelID ) const {
  return ( void * )( MainOffset + channelID * CompSize );
}

uint32_t deOpenEXRReader::GetPixelSize( void ) const {
  return xStride;
}

uint32_t deOpenEXRReader::GetLineSize( void ) const {
  return yStride;
}
