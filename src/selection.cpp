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

#include "math/rect.hpp"
#include "selection.hpp"

class SelectionImpl
{
public:
  Selection::Images images;
};

Selection::Selection()
  : impl(new SelectionImpl())
{
}

void
Selection::add_image(const Image& image)
{
  impl->images.push_back(image);
}

void
Selection::add_images(const std::vector<Image>& images)
{
  for(Images::const_iterator i = images.begin(); i != images.end(); ++i)
    impl->images.push_back(*i);
}

void
Selection::remove_image(const Image& image)
{
  impl->images.push_back(image);
}

void
Selection::scale(float factor)
{
  Vector2f center = get_center();
  for(Images::iterator i = impl->images.begin(); i != impl->images.end(); ++i)
    {
      i->set_scale(i->get_scale() * factor);

      i->set_pos(center + (i->get_pos() - center) * factor);
    }
}

void
Selection::clear()
{
  impl->images.clear();
}

Selection::Images
Selection::get_images() const 
{
  return impl->images; 
}

bool
Selection::empty() const
{
  return impl->images.empty();
}

Vector2f
Selection::get_center() const
{
  if (impl->images.empty())
    {
      return Vector2f();
    }
  else
    {
      Vector2f pos;
      for(Images::const_iterator i = impl->images.begin(); i != impl->images.end(); ++i)
        {
          pos += i->get_pos();
        }
      return pos / impl->images.size();
    }
}

Selection::iterator
Selection::begin()
{
  return impl->images.begin();
}

Selection::iterator
Selection::end()
{
  return impl->images.end();
}

/* EOF */