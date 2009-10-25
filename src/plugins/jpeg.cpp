/*
**  Galapix - an image viewer for large image collections
**  Copyright (C) 2008 Ingo Ruhnke <grumbel@gmx.de>
**
**  This program is free software: you can redistribute it and/or modify
**  it under the terms of the GNU General Public License as published by
**  the Free Software Foundation, either version 3 of the License, or
**  (at your option) any later version.
**
**  This program is distributed in the hope that it will be useful,
**  but WITHOUT ANY WARRANTY; without even the implied warranty of
**  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
**  GNU General Public License for more details.
**
**  You should have received a copy of the GNU General Public License
**  along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <iostream>
#include <sstream>
#include <boost/bind.hpp>
#include <setjmp.h>

#include "math/size.hpp"
#include "plugins/jpeg.hpp"
#include "plugins/file_jpeg_compressor.hpp"
#include "plugins/mem_jpeg_compressor.hpp"
#include "plugins/file_jpeg_decompressor.hpp"
#include "plugins/mem_jpeg_decompressor.hpp"
#include "plugins/exif.hpp"

Size
JPEG::get_size(const std::string& filename)
{
  FileJPEGDecompressor loader(filename);
  loader.read_header();
  Size size = loader.get_size();

  SoftwareSurface::Modifier modifier = EXIF::get_orientation(filename);
  switch(modifier)
  {
    case SoftwareSurface::kRot90:
    case SoftwareSurface::kRot90Flip:
    case SoftwareSurface::kRot270:
    case SoftwareSurface::kRot270Flip:
      return Size(size.height, size.width);

    case SoftwareSurface::kRot0:
    case SoftwareSurface::kRot0Flip:
    case SoftwareSurface::kRot180:
    case SoftwareSurface::kRot180Flip:
    default:
      return size;
  }
}

SoftwareSurfacePtr
JPEG::load_from_file(const std::string& filename, int scale)
{
  FileJPEGDecompressor loader(filename);
  SoftwareSurfacePtr surface = loader.read_image(scale);

  SoftwareSurface::Modifier modifier = EXIF::get_orientation(filename);

  if (modifier == SoftwareSurface::kRot0)
  {
    return surface;
  }
  else
  {
    return surface->transform(modifier);
  }
}

SoftwareSurfacePtr
JPEG::load_from_mem(uint8_t* mem, int len, int scale)
{
  MemJPEGDecompressor loader(mem, len);
  SoftwareSurfacePtr surface = loader.read_image(scale);

  return surface;
}

void
JPEG::save(const SoftwareSurfacePtr& surface, int quality, const std::string& filename)
{
  FileJPEGCompressor compressor(filename);
  compressor.save(surface, quality);
}

BlobPtr
JPEG::save(const SoftwareSurfacePtr& surface, int quality)
{
  std::vector<uint8_t> data;
  MemJPEGCompressor compressor(data);
  compressor.save(surface, quality);
  // FIXME: Unneeded copy of data
  return Blob::copy(data);
}
  
/* EOF */
