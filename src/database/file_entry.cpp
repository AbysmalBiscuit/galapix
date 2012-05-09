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

#include "database/file_entry.hpp"

#include <iostream>

#include "sqlite/reader.hpp"

FileEntry 
FileEntry::from_reader(SQLiteReader& reader)
{
  return FileEntry(RowId(reader.get_int(0)),  // fileid
                   URL::from_string(reader.get_text(1)), // url
                   SHA1(reader.get_blob(2)),
                   reader.get_int(3),  // size
                   reader.get_int(4),  // mtime
                   static_cast<FileEntry::Handler>(reader.get_int(4))); // format
}

std::ostream& operator<<(std::ostream& os, const FileEntry& entry)
{
  return os << "FileEntry(" 
            << entry.get_fileid() << ", " 
            << entry.get_url()    << ", "
            << entry.get_sha1()   << ", "
            << entry.get_size()   << ", "
            << entry.get_mtime()
    //<< entry.get_format();
  << ")";    
}

/* EOF */
