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

#ifndef HEADER_GALAPIX_UTIL_SEXPR_FILE_WRITER_HPP
#define HEADER_GALAPIX_UTIL_SEXPR_FILE_WRITER_HPP

#include <ostream>

#include "util/file_writer.hpp"
#include "math/rgba.hpp"
#include "math/vector3f.hpp"
#include "math/size.hpp"

class SExprFileWriter : public FileWriter
{
private:
  /** A reference to the output stream */
  std::ostream* out;
  int level;
  std::string indent();

public:
  SExprFileWriter(std::ostream& out_);
  virtual ~SExprFileWriter();

  virtual void begin_section(const char* name);
  virtual void end_section();

  virtual void write_int(const char* name, int);
  virtual void write_float(const char* name, float);
  virtual void write_rgba(const char* name, const RGBA&);
  virtual void write_bool(const char* name, bool);
  virtual void write_string(const char* name, const std::string&);
  virtual void write_vector(const char* name, const Vector3f&);
  virtual void write_size(const char* name, const Size&);

  template<class E, class F>
  void write_enum(const char* name, E value, F enum2string)
  {
    (*out) << "\n" << indent() << "(" << name << " \"" << enum2string(value) << "\")";
  }

private:
  SExprFileWriter(const SExprFileWriter&);
  SExprFileWriter& operator= (const SExprFileWriter&);
};

#endif

/* EOF */
