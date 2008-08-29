/*  $Id$
**   __      __ __             ___        __   __ __   __
**  /  \    /  \__| ____    __| _/_______/  |_|__|  | |  |   ____
**  \   \/\/   /  |/    \  / __ |/  ___/\   __\  |  | |  | _/ __ \
**   \        /|  |   |  \/ /_/ |\___ \  |  | |  |  |_|  |_\  ___/
**    \__/\  / |__|___|  /\____ /____  > |__| |__|____/____/\___  >
**         \/          \/      \/    \/                         \/
**  Copyright (C) 2007 Ingo Ruhnke <grumbel@gmx.de>
**
**  This program is free software; you can redistribute it and/or
**  modify it under the terms of the GNU General Public License
**  as published by the Free Software Foundation; either version 2
**  of the License, or (at your option) any later version.
**
**  This program is distributed in the hope that it will be useful,
**  but WITHOUT ANY WARRANTY; without even the implied warranty of
**  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
**  GNU General Public License for more details.
** 
**  You should have received a copy of the GNU General Public License
**  along with this program; if not, write to the Free Software
**  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
**  02111-1307, USA.
*/

#include <boost/bind.hpp>
#include "math/rgb.hpp"
#include "math/rect.hpp"
#include "framebuffer.hpp"
#include "surface.hpp"
#include "math.hpp"
#include "file_entry.hpp"
#include "database_thread.hpp"
#include "viewer_thread.hpp"
#include "thread_message_queue.hpp"
#include "image.hpp"

uint32_t make_cache_id(int x, int y, int tile_scale)
{
  return x | (y << 8) | (tile_scale << 16);
}

class ImageImpl
{
public:
  FileEntry file_entry;
  float scale;

  int min_keep_scale; 
  Vector2f pos;

  Image::Cache cache;
  Image::Jobs jobs;  

  ThreadMessageQueue<TileEntry> tile_queue;
  
  ImageImpl() 
  {
  }

  ~ImageImpl()
  {
  }
};

Image::Image()
{
}

Image::Image(const FileEntry& file_entry)
  : impl(new ImageImpl())
{
  impl->file_entry = file_entry;
  impl->scale      = 1.0f;

  int size  = Math::max(file_entry.size.width, file_entry.size.height);
  impl->min_keep_scale = 0;
  while(size > 32) 
    {
      size /= 2;
      impl->min_keep_scale +=1 ;
    }
  
}

void
Image::set_pos(const Vector2f& pos_)
{
  impl->pos = pos_;
}

Vector2f
Image::get_pos() const
{
  return impl->pos;
}

void
Image::set_scale(float f)
{
  impl->scale = f;
}

float
Image::get_scale() const
{
  return impl->scale;
}

float
Image::get_scaled_width() const
{
  return impl->file_entry.size.width * impl->scale;
}

float
Image::get_scaled_height() const
{
  return impl->file_entry.size.height * impl->scale;
}

int
Image::get_original_width() const
{
  return impl->file_entry.size.width;
}

int
Image::get_original_height() const
{
  return impl->file_entry.size.height;
}

Surface
Image::get_tile(int x, int y, int tile_scale)
{
  uint32_t cache_id = make_cache_id(x, y, tile_scale);
  Cache::iterator i = impl->cache.find(cache_id);

  if (i == impl->cache.end())
    {
      // Important: it must be '*this', not 'this', since the 'this'
      // pointer might disappear any time, its only the impl that
      // stays and which we can link to by making a copy of the Image
      // object via *this.
      impl->jobs.push_back(DatabaseThread::current()->request_tile(impl->file_entry, tile_scale, Vector2i(x, y), 
                                                                   boost::bind(&Image::receive_tile, *this, _1)));

      // FIXME: Something to try: Request the next smaller tile too,
      // so we get a lower quality image fast and a higher quality one
      // soon after FIXME: Its unclear if this actually improves
      // things, also the order of request gets mungled in the
      // DatabaseThread, we should request the whole group of lower
      // res tiles at once, instead of one by one, since that eats up
      // the possible speed up
      //impl->jobs.push_back(DatabaseThread::current()->request_tile(impl->file_entry, tile_scale+1, Vector2i(x, y), 
      //                                                             boost::bind(&Image::receive_tile, *this, _1)));

      SurfaceStruct s;
      
      s.surface = Surface();
      s.status  = SurfaceStruct::SURFACE_REQUESTED;

      impl->cache[cache_id] = s;

      return Surface();
    }
  else
    {
      return i->second.surface;
    }
}

Surface
Image::find_smaller_tile(int x, int y, int tiledb_scale, int& downscale)
{
  int  downscale_factor = 1;

  // FIXME: Replace this loop with a 'find_next_best_smaller_tile()' function
  while(downscale_factor < 10) // Make this 'max_scale' instead of random number
    {
      downscale = Math::pow2(downscale_factor);
      uint32_t cache_id = make_cache_id(x/downscale, y/downscale, tiledb_scale+downscale_factor);
      Cache::iterator i = impl->cache.find(cache_id);
      if (i != impl->cache.end() && i->second.surface)
        {
          return i->second.surface;
        }

      downscale_factor += 1;
    }
  return Surface();
}

void
Image::draw_tile(int x, int y, int tiledb_scale, 
                 const Vector2f& pos, float scale)
{
  Surface surface = get_tile(x, y, tiledb_scale);
  if (surface)
    {
      surface.draw(Rectf(pos, surface.get_size() * scale));
      //Framebuffer::draw_rect(Rectf(pos, surface.get_size() * scale), RGB(100, 100, 100));
    }
  else
    {
      // Look for the next smaller tile
      // FIXME: Rewrite this to work all smaller tiles, not just the next     
      int downscale;
      surface = find_smaller_tile(x, y, tiledb_scale, downscale);
      if (surface)
        { // Must only draw relevant section!
          Size s((x%downscale) ? (surface.get_width()  - 256/downscale * (x%downscale)) : 256/downscale,
                 (y%downscale) ? (surface.get_height() - 256/downscale * (y%downscale)) : 256/downscale);

          s.width  = Math::min(surface.get_width(),  s.width);
          s.height = Math::min(surface.get_height(), s.height);
          
          surface.draw(Rectf(Vector2f(x%downscale, y%downscale) * 256/downscale, 
                             s),
                       Rectf(pos, s * scale * downscale));
        }
      else // draw replacement rect when no tile could be loaded
        {         
          // Calculate the actual size of the tile (i.e. border tiles might be smaller then 256x256)
          Size s(Math::min(256, (impl->file_entry.size.width  / Math::pow2(tiledb_scale)) - 256 * x),
                 Math::min(256, (impl->file_entry.size.height / Math::pow2(tiledb_scale)) - 256 * y));

          Framebuffer::fill_rect(Rectf(pos, s*scale),
                                 RGB(255, 0, 255));
        }
    }
}

void 
Image::draw_tiles(const Rect& rect, int tiledb_scale, 
                  const Vector2f& pos, float scale)
{
  float tilesize = 256.0f * scale;

  for(int y = rect.top; y < rect.bottom; ++y)
    for(int x = rect.left; x < rect.right; ++x)
      {
        draw_tile(x, y, tiledb_scale, 
                  impl->pos + Vector2f(x,y) * tilesize,
                  scale);
      }
}

void
Image::process_queue()
{
  // Check the queue for newly arrived tiles
  while (!impl->tile_queue.empty())
    {
      TileEntry tile = impl->tile_queue.front();
      impl->tile_queue.pop();

      int tile_id = make_cache_id(tile.pos.x, tile.pos.y, tile.scale);
  
      SurfaceStruct s;
  
      if (tile.surface)
        {
          s.surface = Surface(tile.surface);
          s.status  = SurfaceStruct::SURFACE_OK;
        }
      else
        {
          s.surface = Surface();
          s.status  = SurfaceStruct::SURFACE_FAILED;
        }

      impl->cache[tile_id] = s;
    }
}

void
Image::cache_cleanup()
{
  // FIXME: Cache cleanup should happen based on if the Tile is
  // visible, not if the image is visible

  // Image is not visible, so cancel all jobs
  for(Jobs::iterator i = impl->jobs.begin(); i != impl->jobs.end(); ++i)
    i->abort();
  impl->jobs.clear();
        
  // FIXME: We also need to purge the cache more often, since with
  // big gigapixel images we would end up never clearing it
      
  // Erase all tiles larger then 32x32

  // FIXME: Could make this more clever and relative to the number
  // of the images we display, since with large collections 32x32
  // might be to much for memory while with small collections it
  // will lead to unnecessary loading artifacts.      
  for(Cache::iterator i = impl->cache.begin(); i != impl->cache.end(); ++i)
    {
      int tiledb_scale = (i->first >> 16); // get scale
      if (tiledb_scale > impl->min_keep_scale)
        {
          impl->cache.erase(i);
        }
    }
}

void
Image::draw(const Rectf& cliprect, float fscale)
{
  process_queue();
  
  Rectf image_rect(impl->pos, Sizef(impl->file_entry.size * impl->scale)); // in world coordinates

  if (!cliprect.is_overlapped(image_rect))
    {
      cache_cleanup();
    }
  else
    {
      // scale factor for requesting the tile from the TileDatabase
      // FIXME: Can likely be done without float
      int tiledb_scale = Math::max(0, static_cast<int>(log(1.0f / (fscale*impl->scale)) /
                                                       log(2)));
      int scale_factor = Math::pow2(tiledb_scale);

      int scaled_width  = impl->file_entry.size.width  / scale_factor;
      int scaled_height = impl->file_entry.size.height / scale_factor;

      if (scaled_width  < 256 && scaled_height < 256)
        { // So small that only one tile is to be drawn
          draw_tile(0, 0, tiledb_scale, 
                    impl->pos,
                    scale_factor * impl->scale);
        }
      else
        {
          Rectf image_region = image_rect.clip_to(cliprect); // visible part of the image

          image_region.left   = (image_region.left   - impl->pos.x) / impl->scale;
          image_region.right  = (image_region.right  - impl->pos.x) / impl->scale;
          image_region.top    = (image_region.top    - impl->pos.y) / impl->scale;
          image_region.bottom = (image_region.bottom - impl->pos.y) / impl->scale;

          int   itilesize = 256 * scale_factor;
          
          int start_x = (image_region.left)  / itilesize;
          int end_x   = (image_region.right) / itilesize + 1;

          int start_y = (image_region.top   ) / itilesize;
          int end_y   = (image_region.bottom) / itilesize + 1;

          draw_tiles(Rect(start_x, start_y, end_x, end_y), 
                     tiledb_scale, 
                     impl->pos,
                     scale_factor * impl->scale);
        }
    }
}


void
Image::receive_tile(const TileEntry& tile)
{
  assert(impl.get());
  impl->tile_queue.push(tile);
}

/* EOF */
