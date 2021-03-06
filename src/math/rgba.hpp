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

#ifndef HEADER_GALAPIX_MATH_RGBA_HPP
#define HEADER_GALAPIX_MATH_RGBA_HPP

#include <stdint.h>

class RGBA
{
public:
  uint8_t r;
  uint8_t g; 
  uint8_t b;
  uint8_t a;

  RGBA()
    : r(0), g(0), b(0), a(0)
  {}

  RGBA(uint8_t r_, uint8_t g_, uint8_t b_, uint8_t a_ = 255)
    : r(r_), g(g_), b(b_), a(a_)
  {}

  RGBA(uint32_t c)
    : r(static_cast<uint8_t>((c>> 0) & 0xFF)), 
      g(static_cast<uint8_t>((c>> 8) & 0xFF)),
      b(static_cast<uint8_t>((c>>16) & 0xFF)),
      a(static_cast<uint8_t>((c>>24) & 0xFF))
  {}

  uint32_t get_uint32() const {
    return ((r << 0) | (g << 8) | (b << 16) | (a << 24));
  }
};

#endif

/* EOF */
