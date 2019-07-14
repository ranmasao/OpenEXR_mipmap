#ifndef DE_OPENEXR_WRITER_H
#define DE_OPENEXR_WRITER_H

#include "OpenEXRReader.h"

const int MAX_IMAGES = 10;

class deOpenEXRWriter {
public:
  bool  AddImage( deOpenEXRReader * image );
  bool  Write( char * filename );
private:
  int                ImagesCount = 0;
  deOpenEXRReader *  Images[ MAX_IMAGES ];
};


#endif // DE_OPENEXR_WRITER_H
