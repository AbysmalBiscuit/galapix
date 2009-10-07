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

#include "image.hpp"

#include "math/rgb.hpp"
#include "display/framebuffer.hpp"
#include "database/file_entry.hpp"
#include "database_thread.hpp"
#include "galapix/viewer.hpp"
#include "galapix/image_tile_cache.hpp"

class ImageImpl
{
public:
  FileEntry file_entry;

  /** The smallest scale that is stored permanently */
  int min_keep_scale; 

  /** The maximum scale for which tiles exist */
  int max_scale;

  float alpha;

  /** Position refers to the center of the image */
  Vector2f pos;
  Vector2f last_pos;
  Vector2f target_pos;

  /** Scale of the image */
  float scale;
  float last_scale;
  float target_scale;

  /** Rotation angle */
  float angle;

  ImageTileCache cache;

  ImageImpl(const FileEntry& file_entry_) :
    file_entry(file_entry_),
    min_keep_scale(), 
    max_scale(),
    alpha(),
    pos(),
    last_pos(),
    target_pos(),
    scale(),
    last_scale(),
    target_scale(),
    angle(),
    cache(file_entry_)
  {
  }

  ~ImageImpl()
  {
  }
};

Image::Image()
  : impl()
{
}

Image::Image(const FileEntry& file_entry)
  : impl(new ImageImpl(file_entry))
{
  impl->angle        = 0.0f; 
  impl->scale        = 1.0f;
  impl->last_scale   = 1.0f;
  impl->target_scale = 1.0f;

  impl->max_scale = file_entry.get_thumbnail_scale();
  impl->min_keep_scale = impl->max_scale - 2;

  assert(impl->max_scale >= 0);
}

Vector2f
Image::get_top_left_pos() const
{
  return impl->pos - Vector2f(get_scaled_width()/2, get_scaled_height()/2);
}

void
Image::set_pos(const Vector2f& pos)
{
  impl->pos        = pos;
  impl->last_pos   = pos;
  impl->target_pos = pos;
}

Vector2f
Image::get_pos() const
{
  return impl->pos;
}

void
Image::set_target_pos(const Vector2f& target_pos)
{
  impl->last_pos   = impl->pos;
  impl->target_pos = target_pos;
}

void
Image::set_target_scale(float target_scale)
{
  impl->last_scale   = impl->scale;
  impl->target_scale = target_scale;  
}

void
Image::update_pos(float progress)
{
  assert(progress >= 0.0f &&
         progress <= 1.0f);

  if (progress == 1.0f)
    {
      set_pos(impl->target_pos);
      set_scale(impl->target_scale);
    }
  else
    {
      impl->pos   = (impl->last_pos   * (1.0f - progress)) + (impl->target_pos   * progress);
      impl->scale = (impl->last_scale * (1.0f - progress)) + (impl->target_scale * progress);
    }
}

void
Image::set_angle(float a)
{
  impl->angle = a;
}

float
Image::get_angle() const
{
  return impl->angle;
}

void
Image::set_scale(float f)
{
  impl->scale        = f;
  impl->last_scale   = f;
  impl->target_scale = f;
}

float
Image::get_scale() const
{
  return impl->scale;
}

float
Image::get_scaled_width() const
{
  return static_cast<float>(impl->file_entry.get_width()) * impl->scale;
}

float
Image::get_scaled_height() const
{
  return static_cast<float>(impl->file_entry.get_height()) * impl->scale;
}

int
Image::get_original_width() const
{
  return impl->file_entry.get_width();
}

int
Image::get_original_height() const
{
  return impl->file_entry.get_height();
}

void
Image::draw_tile(int x, int y, int scale, 
                 const Vector2f& pos, float fscale)
{
  Surface surface = impl->cache.get_tile(x, y, scale);
  if (surface)
    {
      // FIXME: surface.get_size() * scale does not give the correct
      // size of a tile due to rounding errors
      surface.draw(Rectf(pos, surface.get_size() * fscale));
      //surface.draw(Rectf(pos, get_tile_size(x, y, scale)));
      //Framebuffer::draw_rect(Rectf(pos, surface.get_size() * scale), RGB(100, 100, 100));
    }
  else
    {
      // Look for the next smaller tile
      // FIXME: Rewrite this to work all smaller tiles, not just the next     
      int downscale;
      surface = impl->cache.find_smaller_tile(x, y, scale, downscale);

      // Calculate the actual size of the tile (i.e. border tiles might be smaller then 256x256)
      Size tile_size(Math::min(256, (impl->file_entry.get_width()  / Math::pow2(scale)) - 256 * x),
                     Math::min(256, (impl->file_entry.get_height() / Math::pow2(scale)) - 256 * y));


      if (surface)
        { // Must only draw relevant section!
          Size s((x%downscale) ? (surface.get_width()  - 256/downscale * (x%downscale)) : 256/downscale,
                 (y%downscale) ? (surface.get_height() - 256/downscale * (y%downscale)) : 256/downscale);

          s.width  = Math::min(surface.get_width(),  s.width);
          s.height = Math::min(surface.get_height(), s.height);
          
          surface.draw(Rectf(Vector2f(static_cast<float>(x % downscale), 
                                      static_cast<float>(y % downscale)) * static_cast<float>(256/downscale), 
                             s),
                       //Rectf(pos, tile_size * scale)); kind of works, but leads to discontuinity and jumps
                       Rectf(pos, s * fscale * static_cast<float>(downscale)));
        }
      else // draw replacement rect when no tile could be loaded
        {         
          Framebuffer::fill_rect(Rectf(pos, tile_size * fscale), RGB(155, 0, 155)); // impl->file_entry.color);
        }

      //Framebuffer::draw_rect(Rectf(pos, s*scale), RGB(255, 255, 255)); // impl->file_entry.color);
    }
}

void 
Image::draw_tiles(const Rect& rect, int scale, 
                  const Vector2f& pos, float fscale)
{
  float tilesize = 256.0f * fscale;

  for(int y = rect.top; y < rect.bottom; ++y)
    for(int x = rect.left; x < rect.right; ++x)
      {
        draw_tile(x, y, scale, 
                  get_top_left_pos() + Vector2f(static_cast<float>(x), static_cast<float>(y)) * tilesize,
                  fscale);
      }
}

void
Image::clear_cache()
{
  impl->cache.clear();
}

void
Image::cache_cleanup()
{
  impl->cache.cleanup();
}

void
Image::refresh(bool force)
{
  if (force || impl->file_entry.get_url().get_mtime() != impl->file_entry.get_mtime())
    {
      clear_cache();
      DatabaseThread::current()->delete_file_entry(impl->file_entry.get_fileid());

      // FIXME: Add this point the FileEntry is invalid and points to
      // something that no longer exists, newly generated Tiles point
      // to that obsolete fileid number
      // do stuff with receive_file_entry() to fix this
    }
}

void
Image::print_info() const
{
  std::cout << "  Image: " << impl.get() << std::endl;
  //std::cout << "    Cache Size: " << impl->cache.size() << std::endl;
  //std::cout << "    Job Size:   " << impl->jobs.size() << std::endl;
}

bool
Image::overlaps(const Vector2f& pos) const
{
  return get_image_rect().contains(pos);
}

bool
Image::overlaps(const Rectf& cliprect) const
{
  return cliprect.is_overlapped(get_image_rect());
}

void
Image::draw(const Rectf& cliprect, float fscale)
{
  if (impl->file_entry)
  {
    impl->cache.process_queue();
  
    Rectf image_rect = get_image_rect();
    Vector2f top_left(image_rect.left, image_rect.top);

    if (!cliprect.is_overlapped(image_rect))
    {
      cache_cleanup();
    }
    else
    {
      // scale factor for requesting the tile from the TileDatabase
      // FIXME: Can likely be done without float
      
      //std::cout << 0 << " " 
      //          << static_cast<int>(log(1.0f / (fscale*impl->scale)) / log(2)) << " "
      //          << impl->max_scale << std::endl;
  
      assert(impl->max_scale >= 0);
      int tiledb_scale = Math::clamp(0, static_cast<int>(log(1.0f / (fscale*impl->scale)) /
                                                         log(2)), impl->max_scale);
      int scale_factor = Math::pow2(tiledb_scale);

      int scaled_width  = impl->file_entry.get_width()  / scale_factor;
      int scaled_height = impl->file_entry.get_height() / scale_factor;

      if (scaled_width  < 256 && scaled_height < 256)
      { // So small that only one tile is to be drawn
        draw_tile(0, 0, tiledb_scale, 
                  top_left,
                  static_cast<float>(scale_factor) * impl->scale);
      }
      else
      {
        Rectf image_region = image_rect.clip_to(cliprect); // visible part of the image

        image_region.left   = (image_region.left   - top_left.x) / impl->scale;
        image_region.right  = (image_region.right  - top_left.x) / impl->scale;
        image_region.top    = (image_region.top    - top_left.y) / impl->scale;
        image_region.bottom = (image_region.bottom - top_left.y) / impl->scale;

        int   itilesize = 256 * scale_factor;
          
        int start_x = static_cast<int>(image_region.left / static_cast<float>(itilesize));
        int end_x   = Math::ceil_div(static_cast<int>(image_region.right), itilesize);

        int start_y = static_cast<int>(image_region.top / static_cast<float>(itilesize));
        int end_y   = Math::ceil_div(static_cast<int>(image_region.bottom), itilesize);

        draw_tiles(Rect(start_x, start_y, end_x, end_y), 
                   tiledb_scale, 
                   top_left,
                   static_cast<float>(scale_factor) * impl->scale);
      }
    }
  }
}

URL
Image::get_url() const
{
  return impl->file_entry.get_url();
}

void
Image::draw_mark()
{
  Framebuffer::draw_rect(get_image_rect(), RGB(255, 255, 255));
}

Rectf
Image::get_image_rect() const
{
  if (!impl->file_entry)
    {
      return Rectf(impl->pos, Size(0, 0));
    }
  else
    {
      if (impl->file_entry.get_image_size() == Size(0,0))
        {
          return Rectf(impl->pos, impl->file_entry.get_image_size());
        }
      else
        {
          Sizef image_size(impl->file_entry.get_image_size() * impl->scale);
          return Rectf(impl->pos - Vector2f(image_size.width/2, image_size.height/2), image_size); // in world coordinates
        }
    }
}

void
Image::receive_file_entry(const FileEntry& file_entry)
{
  assert(!impl->file_entry);

  impl->file_entry = file_entry;

  impl->max_scale      = impl->file_entry.get_thumbnail_scale();
  impl->min_keep_scale = impl->max_scale - 2;
}

/* EOF */