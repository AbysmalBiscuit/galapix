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

#ifndef HEADER_GALAPIX_UTIL_SOFTWARE_SURFACE_FACTORY_HPP
#define HEADER_GALAPIX_UTIL_SOFTWARE_SURFACE_FACTORY_HPP

#include "util/software_surface.hpp"

class URL;

class SoftwareSurfaceFactory
{
public:
  // Do not change the value of these, since they are stored in the database
  enum FileFormat { 
    JPEG_FILEFORMAT = 0,
    PNG_FILEFORMAT  = 1, 
    XCF_FILEFORMAT, 
    KRA_FILEFORMAT, 
    SVG_FILEFORMAT, 
    MAGICK_FILEFORMAT, 
    UFRAW_FILEFORMAT, 
    UNKNOWN_FILEFORMAT 
  };

  static FileFormat get_fileformat(const URL& url);
  static bool       get_size(const URL& url, Size& size);

  static SoftwareSurfacePtr from_url(const URL& url);

private:
  SoftwareSurfaceFactory(const SoftwareSurfaceFactory&);
  SoftwareSurfaceFactory& operator=(const SoftwareSurfaceFactory&);
};

#endif

/* EOF */
