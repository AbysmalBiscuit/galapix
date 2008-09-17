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

#include <boost/bind.hpp>
#include <algorithm>
#include <sstream>
#include <stdexcept>
#include <iostream>
#include <string>
#include <vector>
#include <sqlite3.h>

#include "jpeg.hpp"
#include "png.hpp"
#include "xcf.hpp"
#include "imagemagick.hpp"
#include "surface.hpp"
#include "framebuffer.hpp"
#include "math/size.hpp"
#include "math/rect.hpp"
#include "math/vector2i.hpp"
#include "sqlite.hpp"
#include "software_surface.hpp"
#include "jpeg_decoder_thread.hpp"
#include "file_database.hpp"
#include "tile_database.hpp"
#include "database_thread.hpp"
#include "filesystem.hpp"
#include "tile_generator.hpp"
#include "tile_generator_thread.hpp"
#include "workspace.hpp"
#include "viewer_thread.hpp"
#include "viewer.hpp"
#include "galapix.hpp"

Galapix::Galapix()
  : fullscreen(false),
    geometry(800, 600)
{
  Filesystem::init();
}

Galapix::~Galapix()
{
  Filesystem::deinit();
}

void
Galapix::test(const std::vector<URL>& url)
{
  for(std::vector<URL>::const_iterator i = url.begin(); i != url.end(); ++i)
    {
      std::vector<std::string> layer =  XCF::get_layer(*i);
      std::cout << *i << ":" << std::endl;
      for(std::vector<std::string>::iterator j = layer.begin(); j != layer.end(); ++j)
        {
          std::cout << "  '" << *j << "'" << std::endl;
        }
    }
}

void
Galapix::info(const std::vector<URL>& url)
{
  for(std::vector<URL>::const_iterator i = url.begin(); i != url.end(); ++i)
    {
      Size size;

      if (SoftwareSurface::get_size(*i, size))
        std::cout << *i << " " << size.width << "x" << size.height << std::endl;
      else
        std::cout << "Error reading " << *i << std::endl;
    }
}

void
Galapix::downscale(const std::vector<URL>& url)
{
  int num = 0;
  for(std::vector<URL>::const_iterator i = url.begin(); i != url.end(); ++i, ++num)
    {
      std::cout << *i << std::endl;
      SoftwareSurface surface = JPEG::load_from_file(i->get_stdio_name(), 8);

      std::ostringstream out;
      out << "/tmp/out-" << num << ".jpg";
      Blob blob = JPEG::save(surface, 75);
      blob.write_to_file(out.str());

      std::cout << "Wrote: " << out.str() << std::endl;
    }  
}

void
Galapix::cleanup(const std::string& database)
{
  SQLiteConnection db(database); 
  std::cout << "Running database cleanup routines, this process can take multiple minutes." << std::endl;
  std::cout << "You can interrupt it via Ctrl-c, which won't do harm, but will throw away all the cleanup work done till that point" << std::endl;
  db.vacuum();
  std::cout << "Running database cleanup routines done" << std::endl;
}

void
Galapix::list(const std::string& database)
{
  SQLiteConnection db(database);

  FileDatabase file_db(&db);

  std::vector<FileEntry> entries;
  file_db.get_file_entries(entries);

  for(std::vector<FileEntry>::iterator i = entries.begin(); i != entries.end(); ++i)
    {
      std::cout << i->get_url() << std::endl;
    }  
}

void
Galapix::check(const std::string& database)
{
  SQLiteConnection db(database);

  FileDatabase file_db(&db);
  TileDatabase tile_db(&db);

  file_db.check();
  tile_db.check();
}

void
Galapix::filegen(const std::string& database, 
                 const std::vector<URL>& url)
{
  SQLiteConnection db(database);
  FileDatabase file_db(&db);  

  for(std::vector<URL>::size_type i = 0; i < url.size(); ++i)
    {
      FileEntry entry = file_db.get_file_entry(url[i]);
      if (!entry)
        {
          std::cout << "Couldn't get entry for " << url[i] << std::endl;
        }
      else
        {
          std::cout << "Got: " << entry.get_url() << " " << entry.get_width() << "x" << entry.get_height() << std::endl;
        }
    }
}

void
Galapix::thumbgen(const std::string& database, 
                  const std::vector<URL>& url)
{
  SQLiteConnection db(database);

  FileDatabase file_db(&db);
  TileDatabase tile_db(&db);

  TileGenerator tile_generator;

  for(std::vector<std::string>::size_type i = 0; i < url.size(); ++i)
    {
      try 
        {
          FileEntry entry = file_db.get_file_entry(url[i]);
          // FIXME: Insert some checks if the tile already exist

          // Generate Image Tiles
          std::cout << "Generating tiles for " << url[i]  << std::endl;
          try {
            tile_generator.generate_quick(entry,
                                          boost::bind(&TileDatabase::store_tile, &tile_db, _1));
          } catch(std::exception& err) {
            std::cout << err.what() << std::endl;
          }
        }
      catch (std::exception& err) 
        {
          std::cout << "Couldn't find entry for " << url[i] << std::endl;
        }
    }
}

void
Galapix::generate_tiles(const std::string& database, 
                        const std::vector<URL>& urls)
{
  SQLiteConnection db(database);

  FileDatabase file_db(&db);
  TileDatabase tile_db(&db);

  TileGenerator tile_generator;

  for(std::vector<URL>::size_type i = 0; i < urls.size(); ++i)
    {
      std::cout << "Getting file entry..." << std::endl;
      FileEntry entry = file_db.get_file_entry(urls[i]);
      if (!entry)
        {
          std::cout << "Couldn't find entry for " << urls[i] << std::endl;
        }
      else
        {
          // Generate Image Tiles
          std::cout << "Generating tiles... " << urls[i]  << std::endl;
          SoftwareSurface surface = SoftwareSurface::from_url(urls[i]);
          
          tile_generator.generate_all(entry.get_fileid(), surface, 
                                      boost::bind(&TileDatabase::store_tile, &tile_db, _1));
        }
    }
}

void
Galapix::view(const std::string& database, const std::vector<URL>& urls, const std::string& pattern)
{
  if (SDL_Init(SDL_INIT_VIDEO) != 0)
    {
      std::cout << "Unable to initialize SDL: " << SDL_GetError() << std::endl;
      exit(1);
    }
  atexit(SDL_Quit); 

  JPEGDecoderThread   jpeg_thread;
  DatabaseThread      database_thread(database);
  TileGeneratorThread tile_generator_thread;
  ViewerThread        viewer_thread(geometry, fullscreen);

  jpeg_thread.start();
  database_thread.start();
  tile_generator_thread.start();

  if (urls.empty() && pattern.empty())
    {
      // When no files are given, display everything in the database
      database_thread.request_all_files(boost::bind(&ViewerThread::receive_file, &viewer_thread, _1));
    }

  for(std::vector<std::string>::size_type i = 0; i < urls.size(); ++i)
    {
      database_thread.request_file(urls[i], boost::bind(&ViewerThread::receive_file, &viewer_thread, _1));
    }

  if (!pattern.empty())
    {
      std::cout << "Using pattern: '" << pattern << "'" << std::endl;
      database_thread.request_files_by_pattern(boost::bind(&ViewerThread::receive_file, &viewer_thread, _1), pattern);
    }

  viewer_thread.run();

  tile_generator_thread.stop();
  database_thread.stop();
  jpeg_thread.stop();

  tile_generator_thread.join(); 
  database_thread.join();
  jpeg_thread.join();
}

void
Galapix::print_usage()
{
  std::cout << "Usage: galapix view     [OPTIONS]... [FILES]...\n"
            << "       galapix prepare  [OPTIONS]... [FILES]...\n"
            << "       galapix thumbgen [OPTIONS]... [FILES]...\n"
            << "       galapix filegen  [OPTIONS]... [FILES]...\n"
            << "       galapix info     [OPTIONS]... [FILES]...\n"
            << "       galapix check    [OPTIONS]...\n"
            << "       galapix list     [OPTIONS]...\n"
            << "       galapix cleanup  [OPTIONS]...\n"
            << "\n"
            << "Commands:\n"
            << "  view      Display the given files\n"
            << "  prepare   Generate all thumbnail for all given images\n"
            << "  thumbgen  Generate only small thumbnails for all given images\n"
            << "  filegen   Generate only small the file entries in the database\n"
            << "  list      Lists all files in the database\n"
            << "  check     Checks the database for consistency\n"
            << "  info      Display size of the given files\n"
            << "  cleanup   Runs garbage collection on the database\n"
            << "\n"
            << "Options:\n"
            << "  -d, --database FILE    Use FILE has database (default: none)\n"
            << "  -f, --fullscreen       Start in fullscreen mode\n"
            << "  -p, --pattern GLOB     Select files from the database via globbing pattern\n"
            << "  -g, --geometry WxH     Start with window size WxH\n"        
            << "\n"
            << "If you do not supply any files, the whole content of the given database will be displayed."
            << std::endl;
}

int
Galapix::main(int argc, char** argv)
{
  // FIXME: Function doesn't seem to be available in 3.4.2
  // if (!sqlite3_threadsafe())
  //  throw std::runtime_error("Error: SQLite must be compiled with SQLITE_THREADSAFE");

  std::string database = Filesystem::get_home() + "/.galapix/cache.sqlite";
  
  if (argc < 2)
    {
      print_usage();
    }
  else
    {
      std::string pattern;
      std::vector<std::string> argument_filenames;
      for(int i = 2; i < argc; ++i)
        {
          if (argv[i][0] == '-')
            {
              if (strcmp(argv[i], "--help") == 0 ||
                  strcmp(argv[i], "-h") == 0)
                {
                  print_usage();
                  exit(0);
                }
              else if (strcmp(argv[i], "--database") == 0 ||
                       strcmp(argv[i], "-d") == 0)
                {
                  ++i;
                  if (i < argc)
                    {
                      database = argv[i];
                    }
                  else
                    {
                      throw std::runtime_error(std::string(argv[i-1]) + " requires an argument");
                    }
                }
              else if (strcmp(argv[i], "--pattern") == 0 ||
                       strcmp(argv[i], "-p") == 0)
                {
                  i += 1;
                  if (i < argc)
                    pattern = argv[i];
                  else
                    throw std::runtime_error(std::string("Option ") + argv[i-1] + " requires an argument");
                }
              else if (strcmp(argv[i], "--geometry") == 0 ||
                       strcmp(argv[i], "-g") == 0)
                {
                  i += 1;
                  if (i < argc)
                    sscanf(argv[i], "%dx%d", &geometry.width, &geometry.height);
                  else
                    throw std::runtime_error(std::string("Option ") + argv[i-1] + " requires an argument");
                }
              else if (strcmp(argv[i], "--fullscreen") == 0 ||
                       strcmp(argv[i], "-f") == 0)
                {
                  fullscreen = true;
                }
              else
                {
                  throw std::runtime_error("Unknown option " + std::string(argv[i]));
                }
            }
          else
            {
              argument_filenames.push_back(argv[i]);
            }
        }

      std::cout << "Using database: " << (database.empty() ? "memory" : database) << std::endl;

      std::vector<URL> urls;
      if (argument_filenames.empty())
        {
          std::cout << "Displaying all files in the database" << std::endl;;
        }
      else
        {
          std::cout << "Scanning directories... " << std::flush;
          for(std::vector<std::string>::iterator i = argument_filenames.begin(); i != argument_filenames.end(); ++i)
            {
              std::cout << " --- " << *i << " " << URL::is_url(*i) << std::endl;
              if (URL::is_url(*i))
                urls.push_back(URL::from_string(*i));
              else
                Filesystem::generate_image_file_list(*i, urls);
            }
          std::sort(urls.begin(), urls.end());
          std::cout << urls.size() << " files found." << std::endl;
        }

      if (strcmp(argv[1], "view") == 0)
        {
          view(database, urls, pattern);
        }
      else if (strcmp(argv[1], "check") == 0)
        {
          check(database);
        }
      else if (strcmp(argv[1], "list") == 0)
        {
          list(database);
        }
      else if (strcmp(argv[1], "cleanup") == 0)
        {
          cleanup(database);
        }
      else if (strcmp(argv[1], "info") == 0)
        {
          info(urls);
        }
      else if (strcmp(argv[1], "test") == 0)
        {
          test(urls);
        }
      else if (strcmp(argv[1], "downscale") == 0)
        {
          downscale(urls);
        }
      else if (strcmp(argv[1], "prepare") == 0)
        {
          generate_tiles(database, urls);
        }
      else if (strcmp(argv[1], "thumbgen") == 0)
        {
          thumbgen(database, urls);
        }
      else if (strcmp(argv[1], "filegen") == 0)
        {
          filegen(database, urls);
        }
      else
        {
          print_usage();
        }
    }

  return 0;
}
  
int main(int argc, char** argv)
{
  try 
    {
      Galapix app;
      int ret = app.main(argc, argv);
      return ret;
    }
  catch(const std::exception& err) 
    {
      std::cout << "Exception: " << err.what() << std::endl;
    }
}
  
/* EOF */
