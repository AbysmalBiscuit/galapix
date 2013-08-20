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

#include "galapix/galapix.hpp"

#include <algorithm>
#include <fstream>
#include <sstream>
#include <stdlib.h>
#include <stdexcept>
#include <iostream>
#include <string>
#include <vector>

#include "archive/archive_manager.hpp"
#include "database/database.hpp"
#include "display/framebuffer.hpp"
#include "display/surface.hpp"
#include "galapix/arg_parser.hpp"
#include "galapix/database_thread.hpp"
#include "galapix/options.hpp"
#include "galapix/thumbnail_generator.hpp"
#include "galapix/viewer.hpp"
#include "galapix/viewer_command.hpp"
#include "galapix/workspace.hpp"
#include "job/job_handle_group.hpp"
#include "job/job_manager.hpp"
#include "jobs/test_job.hpp"
#include "jobs/tile_generation_job.hpp"
#include "math/rect.hpp"
#include "math/size.hpp"
#include "math/vector2i.hpp"
#include "network/download_manager.hpp"
#include "plugins/imagemagick.hpp"
#include "plugins/jpeg.hpp"
#include "plugins/png.hpp"
#include "plugins/xcf.hpp"
#include "util/filesystem.hpp"
#include "util/raise_exception.hpp"
#include "util/software_surface.hpp"
#include "util/software_surface_factory.hpp"
#include "util/string_util.hpp"
#ifdef GALAPIX_SDL
#  include "sdl/sdl_viewer.hpp"
#endif
#ifdef GALAPIX_GTK
#  include "gtk/gtk_viewer.hpp"
#endif

Galapix::Galapix()
{
  Filesystem::init();
}

Galapix::~Galapix()
{
  Filesystem::deinit();
}

void
Galapix::export_images(const std::string& database, const std::vector<URL>& url)
{
#if 0
  Database db(database);
  
  int wish_size = 512;
  int image_num = 0;
  for(std::vector<URL>::const_iterator i = url.begin(); i != url.end(); ++i)
  {
    FileEntry entry = db.get_resources().get_file_entry(*i);
    if (!entry)
    {
      std::cerr << "Error: Couldn't get file entry for " << *i << std::endl;
    }
    else
    {
      Size size = entry.get_image_size();
      int scale = 0;
      while(size.width > wish_size && size.height > wish_size)
      {
        scale += 1;
        size /= 2;
      }

      SoftwareSurfacePtr target = SoftwareSurface::create(SoftwareSurface::RGB_FORMAT, size);
      for(int y = 0; y < (size.height+255)/256; ++y)
        for(int x = 0; x < (size.width+255)/256; ++x)
        {
          TileEntry tile;
          if (db.get_tiles().get_tile(entry.get_fileid(), scale, Vector2i(x, y), tile))
          {
            tile.get_surface()->blit(target, Vector2i(x, y) * 256);
          }
        }

      char filename[1024];
      sprintf(filename, "/tmp/out/%04d.jpg", image_num);
      std::cout << "Writing result to: " << filename << std::endl;
      JPEG::save(target, 85, filename);
      //PNG::save(target, filename);
      image_num += 1;
    }
  }
#endif
}

void
Galapix::cleanup(const std::string& database)
{
  Database db(database); 
  std::cout << "Running database cleanup routines, this process can take multiple minutes." << std::endl;
  std::cout << "You can interrupt it via Ctrl-c, which won't do harm, but will throw away all the cleanup work done till that point" << std::endl;
  db.cleanup();
  std::cout << "Running database cleanup routines done" << std::endl;
}

void
Galapix::list(const Options& opts)
{
  Database db(opts.database);

  std::vector<OldFileEntry> entries;
  if (opts.patterns.empty())
  {
    db.get_resources().get_old_file_entries(entries);
  }
  else
  {
    for(std::vector<std::string>::const_iterator i = opts.patterns.begin(); i != opts.patterns.end(); ++i)
    {
      db.get_resources().get_old_file_entries(*i, entries);
    }
  }

  for(std::vector<OldFileEntry>::iterator i = entries.begin(); i != entries.end(); ++i)
  {
    std::cout << i->get_url() << std::endl;
  }  
}
  
int
Galapix::main(int argc, char** argv)
{
  // FIXME: Function doesn't seem to be available in 3.4.2
  // if (!sqlite3_threadsafe())
  //  raise_runtime_error("Error: SQLite must be compiled with SQLITE_THREADSAFE");

  try 
  {
    Options opts;
    opts.threads  = 2;
    opts.database = Filesystem::get_home() + "/.galapix/cache4";
    ArgParser::parse_args(argc, argv, opts);

    DownloadManager download_manager;
    ArchiveManager archive_manager;
    SoftwareSurfaceFactory software_surface_factory;

    run(opts);

    return EXIT_SUCCESS;
  }
  catch(const std::exception& err) 
  {
    std::cout << "Exception: " << err.what() << std::endl;
    return EXIT_FAILURE;
  }
}
  
void
Galapix::run(const Options& opts)
{
  std::cout << "Using database: " << (opts.database.empty() ? "memory" : opts.database) << std::endl;

  if (opts.rest.empty())
  {
#ifdef GALAPIX_SDL
    ArgParser::print_usage();
#else
    ViewerCommand viewer(opts);
    viewer.run(std::vector<URL>());
#endif
  }
  else
  {
    std::vector<URL> urls;

    std::cout << "Scanning directories... " << std::flush;
    for(std::vector<std::string>::const_iterator i = opts.rest.begin()+1; i != opts.rest.end(); ++i)
    {
      if (URL::is_url(*i))
        urls.push_back(URL::from_string(*i));
      else
        Filesystem::generate_image_file_list(*i, urls);
    }
    std::sort(urls.begin(), urls.end(),
              [](const URL& lhs, const URL& rhs) {
                return StringUtil::numeric_less(lhs.str(), rhs.str());
              });
    std::cout << urls.size() << " files found." << std::endl;

    const std::string& command = opts.rest.front();
        
    if (command == "view")
    {
      if (urls.empty() && opts.patterns.empty())
      {
        std::cout << "Galapix::run(): Error: No URLs given" << std::endl;
      }
      else
      {
        ViewerCommand viewer(opts);
        viewer.run(urls);
      }
    }
    else if (command == "list")
    {
      list(opts);
    }
    else if (command == "cleanup")
    {
      cleanup(opts.database);
    }
    else if (command == "export")
    {
      export_images(opts.database, urls);
    }
    else if (command == "thumbgen")
    {
      ThumbnailGenerator generator(opts);
      generator.run(urls, false);
    }
    else
    {
      ArgParser::print_usage();
    }
  }
}

/* EOF */
