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

#ifndef HEADER_GALAPIX_UTIL_SEVEN_ZIP_ARCHIVE_LOADER_HPP
#define HEADER_GALAPIX_UTIL_SEVEN_ZIP_ARCHIVE_LOADER_HPP

#include "util/archive_loader.hpp"

class SevenZipArchiveLoader : public ArchiveLoader
{
private:
public:
  SevenZipArchiveLoader();

  void register_loader(ArchiveManager& manager);

  std::vector<std::string> get_filenames(const std::string& zip_filename) const;
  BlobPtr get_file(const std::string& zip_filename, const std::string& filename) const;

  std::string str() const { return "7z"; }

private:
  SevenZipArchiveLoader(const SevenZipArchiveLoader&);
  SevenZipArchiveLoader& operator=(const SevenZipArchiveLoader&);
};

#endif

/* EOF */
