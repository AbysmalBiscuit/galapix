/*
**  Galapix - an image viewer for large image collections
**  Copyright (C) 2009 Ingo Ruhnke <grumbel@gmx.de>
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

#include "util/filesystem.hpp"

int main(int argc, char** argv)
{
  for(int i = 1; i < argc; ++i)
  {
    std::string extension = Filesystem::get_extension(argv[i]);
    std::cout << "'" << extension << "' - " << argv[i] << std::endl;
  }
  return 0;
}

/* EOF */
