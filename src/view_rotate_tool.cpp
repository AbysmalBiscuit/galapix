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

#include <math.h>
#include "math/vector2i.hpp"
#include "framebuffer.hpp"
#include "viewer.hpp"
#include "view_rotate_tool.hpp"

ViewRotateTool::ViewRotateTool(Viewer* viewer)
  : Tool(viewer),
    active(false),
    start_angle(0.0f)
{
}

void
ViewRotateTool::move(const Vector2i& pos, const Vector2i& /*rel*/)
{
  if (active)
    {
      Vector2i center(Framebuffer::get_width()/2,
                      Framebuffer::get_height()/2);

      float angle = atan2(pos.y - center.y,
                          pos.x - center.x);

      viewer->get_state().rotate((angle - start_angle)/M_PI * 180.0f);
      start_angle = angle;
    }
}

void
ViewRotateTool::up  (const Vector2i& /*pos*/)
{
  active = false;
}

void
ViewRotateTool::down(const Vector2i& pos)
{
  active = true;

  Vector2i center(Framebuffer::get_width()/2,
                  Framebuffer::get_height()/2);

  start_angle = atan2(pos.y - center.y,
                      pos.x - center.x);
}

/* EOF */