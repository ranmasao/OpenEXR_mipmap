# OpenEXR_mipmap
Simple command line mipmap packer for OpenEXR format.

Successfully compiled using gcc 9.1.0 in MSYS, Windows 10 x64, but it should be possible to compile for Linux and x86.

**Usage**: `<executable>` filename

**Filename** is a valid filename with placeholder symbol '#' for mip levels index sequence, i.e.: for the file sequence mip0.exr, mip1.exr, mip2.exr, etc., you should put mip#.exr.

Resulting image will be saved under the new filename with placeholder symbol changed to 'm', overwriting previosly existing file.

**Restrictions**:
* mip levels must be consequential from 0 (strictly) up to 9;
* all images sizes must be power of two;
* every image must be 50% smaller than previous one;
* all images must have same pixel types.

**Dependencies** (other from standard libs):
* OpenEXR library (http://www.openexr.org)