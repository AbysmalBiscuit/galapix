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

#ifndef HEADER_GALAPIX_DATABASE_STATEMENTS_FILE_ENTRY_GET_ALL_HPP
#define HEADER_GALAPIX_DATABASE_STATEMENTS_FILE_ENTRY_GET_ALL_HPP

#include "sqlite/connection.hpp"
#include "sqlite/statement.hpp"
#include "database/entries/file_entry.hpp"

class FileEntryGetAll
{
private:
  SQLiteStatement m_stmt;

public:
  FileEntryGetAll(SQLiteConnection& db) :
    m_stmt(db,
           "SELECT\n"
           "  file.id, file.path, file.mtime, file.blob_id\n"
           "FROM\n"
           "  file;")
  {}

  void operator()(std::vector<FileEntry>& entries_out)
  {
    SQLiteReader reader = m_stmt.execute_query();

    while (reader.next())
    {
      entries_out.push_back(FileEntry::from_reader(reader));
    }
  }

private:
  FileEntryGetAll(const FileEntryGetAll&);
  FileEntryGetAll& operator=(const FileEntryGetAll&);
};

#endif

/* EOF */
