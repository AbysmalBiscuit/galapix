/*
**  Galapix - an image viewer for large image collections
**  Copyright (C) 2009 Ingo Ruhnke <grumbel@gmx.de>
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

#ifndef HEADER_GALAPIX_UTIL_UFRAW_SOFTWARE_SURFACE_LOADER_HPP
#define HEADER_GALAPIX_UTIL_UFRAW_SOFTWARE_SURFACE_LOADER_HPP

#include <assert.h>

#include "plugins/ufraw.hpp"
#include "util/software_surface_factory.hpp"

class UFRawSoftwareSurfaceLoader : public SoftwareSurfaceLoader
{
private:
public:
  UFRawSoftwareSurfaceLoader()
  {}

  std::string get_name() const
  {
    return "ufraw";
  }

  void register_loader(SoftwareSurfaceFactory& factory) const
  {
    factory.register_by_extension(this, "3fr");
    factory.register_by_extension(this, "arw");
    factory.register_by_extension(this, "cr2");
    factory.register_by_extension(this, "crw");
    factory.register_by_extension(this, "dcr");
    factory.register_by_extension(this, "dcs");
    factory.register_by_extension(this, "dng");
    factory.register_by_extension(this, "kdc");
    factory.register_by_extension(this, "mef");
    factory.register_by_extension(this, "mrw");
    factory.register_by_extension(this, "nef");
    factory.register_by_extension(this, "orf");
    factory.register_by_extension(this, "pef");
    factory.register_by_extension(this, "raf");
    factory.register_by_extension(this, "raw");
    factory.register_by_extension(this, "sr2");
    factory.register_by_extension(this, "srf");
    factory.register_by_extension(this, "x3f");

    factory.register_by_mime_type(this, "image/x-adobe-dng");
    factory.register_by_mime_type(this, "image/x-canon-cr2 ");
    factory.register_by_mime_type(this, "image/x-canon-crw");
    factory.register_by_mime_type(this, "image/x-fuji-raf ");
    factory.register_by_mime_type(this, "image/x-minolta-mrw");
    factory.register_by_mime_type(this, "image/x-nikon-nef");
    factory.register_by_mime_type(this, "image/x-olympus-orf");
    factory.register_by_mime_type(this, "image/x-panasonic-raw ");
    factory.register_by_mime_type(this, "image/x-panasonic-raw");
    factory.register_by_mime_type(this, "image/x-pentax-pef");
    factory.register_by_mime_type(this, "image/x-raw");
    factory.register_by_mime_type(this, "image/x-sony-arw");
    factory.register_by_mime_type(this, "image/x-sony-sr2");
    factory.register_by_mime_type(this, "image/x-sony-srf");
  }

  bool supports_from_file() const { return true; }
  bool supports_from_mem()  const { return false; }

  SoftwareSurfacePtr from_file(const std::string& filename) const
  {
    return UFRaw::load_from_file(filename);
  }

  SoftwareSurfacePtr from_mem(uint8_t* data, int len) const
  {
    assert(!"not implemented");
  }

private:
  UFRawSoftwareSurfaceLoader(const UFRawSoftwareSurfaceLoader&);
  UFRawSoftwareSurfaceLoader& operator=(const UFRawSoftwareSurfaceLoader&);
};

#endif

/* EOF */
