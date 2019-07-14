#include "OpenEXRWriter.h"

#include "OpenEXR/ImfChannelList.h"
using namespace Imath;

bool deOpenEXRWriter::AddImage( deOpenEXRReader * image ) {
  if ( ImagesCount == MAX_IMAGES )
    return false;
  Images[ ImagesCount ] = image;
  ImagesCount++;
  return true;
}

bool deOpenEXRWriter::Write( char * filename ) {
  if ( ImagesCount <= 1 )
    return false;
  bool Result = false;
  const int tileWidth = 64;
  const int tileHeight = 64;
  try {
    Header header( Images[ 0 ]->GetWidth(), Images[ 0 ]->GetHeight() );
    header.lineOrder() = Images[ 0 ]->IsUpsideDown() ? 
                         INCREASING_Y : DECREASING_Y;
    header.compression() = ZIP_COMPRESSION;
    header.setType( "tiledimage" );
    header.channels().insert( "R", Images[ 0 ]->GetPixelType() );
    header.channels().insert( "G", Images[ 0 ]->GetPixelType() );
    header.channels().insert( "B", Images[ 0 ]->GetPixelType() );
    header.setTileDescription( TileDescription( tileWidth, tileHeight, 
                                                MIPMAP_LEVELS, ROUND_DOWN ) );
    TiledOutputFile file( filename, header );
    for ( int level = 0; level < ImagesCount; level++ ) {
      FrameBuffer frameBuffer;
      frameBuffer.insert( "R",
                          Slice( Images[ level ]->GetPixelType(),
                          ( char * )Images[ level ]->GetDataOffset( 0 ),
                          Images[ level ]->GetPixelSize(),
                          Images[ level ]->GetLineSize() ) );
      frameBuffer.insert( "G",
                          Slice( Images[ level ]->GetPixelType(),
                          ( char * )Images[ level ]->GetDataOffset( 1 ),
                          Images[ level ]->GetPixelSize(),
                          Images[ level ]->GetLineSize() ) );
      frameBuffer.insert( "B",
                          Slice( Images[ level ]->GetPixelType(),
                          ( char * )Images[ level ]->GetDataOffset( 2 ),
                          Images[ level ]->GetPixelSize(),
                          Images[ level ]->GetLineSize() ) );
      
      file.setFrameBuffer( frameBuffer );
      for ( int tileY = 0; tileY < file.numYTiles( level ); tileY++ ) {
        for ( int tileX = 0; tileX < file.numXTiles( level ); tileX++ ) {
          //Box2i range = file.dataWindowForTile( tileX, tileY, level );
          //file.writeTile( tileX, tileY, level );
          
          file.writeTile( tileX, tileY, level );
        }
      }
    }
    
    Result = true;
  }
  catch ( Iex::BaseExc & e ) {
    printf( "Caught exception: %s\n", e.what() );
    printf( "Can't write file '%s' correctly!\n", filename );
  }
  return Result;
}
