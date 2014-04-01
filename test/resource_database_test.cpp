/*
**  Galapix - an image viewer for large image collections
**  Copyright (C) 2011 Ingo Ruhnke <grumbel@gmx.de>
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

#include <iostream>

#include "sqlite/connection.hpp"
#include "database/resource_database.hpp"
#include "util/filesystem.hpp"
#include "util/sha1.hpp"

int main(int argc, char** argv)
{
  SQLiteConnection db(""); //("/tmp/resource_database_test.sqlite3");
  ResourceDatabase res_db(db);

  res_db.store_file_info(FileInfo("/tmp/hello_world.txt", 123, SHA1::from_string("hello world"), 98787));
  res_db.store_file_info(FileInfo("/tmp/foobar.txt", 456, SHA1::from_string("foobar"), 1234578));
  res_db.store_file_info(FileInfo("/tmp/barfoo.txt", 789, SHA1::from_string("barfoo"), 1234578));

  for(auto& path : { "/tmp/foobar.txt", "/tmp/barfoo.txt", "/tmp/unknown.txt", "/tmp/hello_world.txt",  })
  {
    boost::optional<FileInfo> file_info = res_db.get_file_info(path);
    if (file_info)
    {
      std::cout << file_info->get_path() << '\n'
                << "  mtime : " << file_info->get_mtime() << '\n'
                << "  sha1  : " << file_info->get_sha1().str() << '\n'
                << "  size  : " << file_info->get_size() << '\n'
                << std::endl;
    }
    else
    {
      std::cout << path << ": not found\n" << std::endl;
    }
  }


  if (false)
  {
    res_db.store_file_entry("/tmp/foo/bar.txt", 23445);
    res_db.store_file_entry("/tmp/foo/bam.txt", 2989);
    res_db.store_file_entry("/tmp/bar/foo.txt", 298998);

    std::vector<FileEntry> entries;
    res_db.get_file_entries(entries);
    std::cout << "got " << entries.size() << " entries" << std::endl;
  }
  
  return 0;
}

/* EOF */