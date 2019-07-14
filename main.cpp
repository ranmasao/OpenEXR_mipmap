#include <stdio.h>

#include "OpenEXRReader.h"
#include "OpenEXRWriter.h"

const char placeholder = '#';
const char * helpMessage = R"msg(OpenEXR mipmap packer v0.1 (July 2019)
Usage: <executable> filename

Filename is a valid filename with placeholder symbol '#' for mip levels
index sequence, i.e.: for the file sequence mip0.exr, mip1.exr, mip2.exr, etc.,
you should put mip#.exr.

Resulting image will be saved under the new filename with placeholder symbol
changed to 'm', overwriting previosly existing file.

Restrictions:
* mip levels must be consequential from 0 (strictly) up to 9;
* all images sizes must be power of two;
* every image must be 50% smaller than previous one;
* all images must have same pixel types.

Source code is available at https://github.com/ranmasao/OpenEXR_mipmap
)msg";

int main( int argc, char ** argv)
{
  int placePos = -1;
  switch ( argc ) {
    case 1: {
      printf( "No input.\n" );
      break;
    }
    case 2: {
      char * pos = strchr( argv[ 1 ], placeholder );
      if ( pos == nullptr ) {
        printf( "Error: placeholder symbol not found.\n" );
        break;
      }
      char * pos2 = strchr( pos + 1, placeholder );
      if ( pos2 != nullptr ) {
        printf( "Error: more than one placeholder symbol.\n" );
        break;
      }
      placePos = pos - argv[ 1 ];
      break;
    }
    default:
      printf( "Error: wrong parameters.\n" );
  }
  if ( placePos < 0 ) {
    printf( helpMessage );
    return EXIT_FAILURE;
  }
  
  char fullname[ 1024 ] = { 0 };
  deOpenEXRReader * image[ MAX_IMAGES ];
  int imageCount = 0;
  for ( int i = 0; i < MAX_IMAGES; i++ ) {
    strcpy( fullname, argv[ 1 ] );
    fullname[ placePos ] = '0' + i;
    printf( "Opening %s... ", fullname );
    image[ i ] = new deOpenEXRReader();
    if ( image[ i ]->Load( fullname ) ) {
      printf( "success.\n" );
      imageCount++;
    } else {
      printf( "failed.\n" );
      delete image[ i ];
      break;
    }
  }
  
  bool imagesOk = true;
  while ( true ) {
    if ( imageCount < 2 ) {
      printf( "Error: not enough images to process.\n" );
      imagesOk = false;
      break;
    }

    int tmpWidth = image[ 0 ]->GetWidth();
    int tmpHeight = image[ 0 ]->GetHeight();
    if ( ( tmpWidth & ( tmpWidth - 1 ) ) != 0 ) {
      printf( "Error: first image width is %d - not power of two.\n",
              tmpWidth );
      imagesOk = false;
      break;
    }
    if ( ( tmpHeight & ( tmpHeight - 1 ) ) != 0 ) {
      printf( "Error: first image width is %d - not power of two.\n",
              tmpHeight );
      imagesOk = false;
      break;
    }

    for ( int i = 1; i < imageCount; i++ ) {
      if ( image[ i ]->GetPixelType() != image[ 0 ]->GetPixelType() ) {
        printf( "Error: image #%d has different pixel type - '%s'.\n", i,
                image[ i ]->GetPixelTypeStr() );
        printf( "First image has pixel type '%s'.\n", 
                image[ 0 ]->GetPixelTypeStr() );
        imagesOk = false;
        break;
      }
      tmpWidth >>= 1;
      tmpHeight >>= 1;
      if ( image[ i ]->GetWidth() != tmpWidth ) {
        printf( "Error: image #%d has wrong width %d. Must be %d.\n", i, 
                image[ i ]->GetWidth(), tmpWidth );
        imagesOk = false;
        break;
      }
      if ( image[ i ]->GetHeight() != tmpHeight ) {
        printf( "Error: image #%d has wrong height %d. Must be %d.\n", i,
                image[ i ]->GetHeight(), tmpHeight );
        imagesOk = false;
        break;
      }
    }
    break;
  }
  
  bool imagesProcessed = false;
  char outFile[ 1024 ] = { 0 };
  deOpenEXRWriter * writer = new deOpenEXRWriter();
  if ( imagesOk ) {
    for ( int i = 0; i < imageCount; i++ ) {
      if ( !image[ i ]->IsUpsideDown() )
        image[ i ]->FlipImage();
      imagesOk = writer->AddImage( image[ i ] );
      if ( !imagesOk ) {
        printf( "Problems with adding image #%d.\n", i );
        break;
      }
    }
    if ( imagesOk ) {
      strcpy( outFile, argv[ 1 ] );
      outFile[ placePos ] = 'm';
      imagesProcessed = writer->Write( outFile );
    }
  }
  delete writer;
  for ( int i = 0; i < imageCount; i++ )
    delete image[ i ];
  if ( imagesProcessed ) {
    printf( "OK, '%s' written.\n", outFile );
    return EXIT_SUCCESS;
  } else {
    printf( "Writing '%s' failed.\n", outFile );
    return EXIT_FAILURE;
  }
  /*
  printf( "Image size: %dx%d\n", image.GetWidth(), image.GetHeight() );
  printf( "Pixel type: " );
  switch ( image.GetPixelType() ) {
    case UINT:
      printf( "UINT" );
      break;
    case HALF:
      printf( "HALF" );
      break;
    case FLOAT:
      printf( "FLOAT" );
      break;
    default:
      printf( "unknown" );
  }
  printf( "\n" );
  printf( "Line order: %s\n", image.IsUpsideDown() ? "flipped" : "normal" );
  */
  return 0;
}
