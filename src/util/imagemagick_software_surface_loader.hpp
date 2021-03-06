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

#ifndef HEADER_GALAPIX_UTIL_IMAGEMAGICK_SOFTWARE_SURFACE_LOADER_HPP
#define HEADER_GALAPIX_UTIL_IMAGEMAGICK_SOFTWARE_SURFACE_LOADER_HPP

#include "util/software_surface_loader.hpp"
#include "util/software_surface_factory.hpp"
#include "plugins/imagemagick.hpp"

class ImagemagickSoftwareSurfaceLoader : public SoftwareSurfaceLoader
{
public:
  ImagemagickSoftwareSurfaceLoader()
  {}

  std::string get_name() const
  {
    return "imagemagick";
  }

  void register_loader(SoftwareSurfaceFactory& factory) const
  {
    std::vector<std::string> lst = Imagemagick::get_supported_extensions();
    for(std::vector<std::string>::const_iterator i = lst.begin();
        i != lst.end(); ++i)
    {
      factory.register_by_extension(this, *i);
    }
  }


  bool supports_from_file() const { return true; }
  bool supports_from_mem()  const { return true; }

  SoftwareSurfacePtr from_file(const std::string& filename) const
  {
    return Imagemagick::load_from_file(filename);
  }

  SoftwareSurfacePtr from_mem(uint8_t* data, int len) const
  {
    return Imagemagick::load_from_mem(data, len);
  }

private:
  ImagemagickSoftwareSurfaceLoader(const ImagemagickSoftwareSurfaceLoader&);
  ImagemagickSoftwareSurfaceLoader& operator=(const ImagemagickSoftwareSurfaceLoader&);
};

#endif

/* EOF */
