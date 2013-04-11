/*
**  Galapix - an image viewer for large image collections
**  Copyright (C) 2012 Ingo Ruhnke <grumbel@gmx.de>
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

#ifndef HEADER_GALAPIX_UTIL_FAILABLE_HPP
#define HEADER_GALAPIX_UTIL_FAILABLE_HPP

#include <assert.h>

template<typename T>
class Failable
{
private:
  union {
    char m_dummy[sizeof(T)];
    T m_obj;
  };

  bool m_initialized;
  
  std::exception_ptr m_eptr;

public:
  Failable() :
    m_initialized(),
    m_eptr()
  {
  }

  Failable(const T& obj) :
  m_obj(obj),
  m_initialized(true),
  m_eptr()
  {
  }

  Failable(const Failable& rhs) :
  m_eptr(),
  m_initialized(false)
  {
    if (rhs.is_initialized())
    {
      construct(rhs.m_data);
    }
  }

  Failable& operator=(const Failable& rhs)
  {
    if (this != &rhs)
    {
      if (rhs.is_initialized())
      {
        assign(rhs.m_data);
      }
      else
      {
        reset();
      }
    }
    
    return *this;
  }

  ~Failable()
  {
    destroy();
  }

  bool is_initialized() const { return m_initialized ; }

  void set_exception(std::exception_ptr eptr)
  {
    m_eptr = eptr;
  }

  void reset()
  {
    destroy();
  }

  void reset(const T& data)
  {
    assign(data);
  }

  const T&  get() const 
  { 
    if (m_eptr != std::exception_ptr()) 
    {
      std::rethrow_exception(m_eptr);
    }
    else
    {
      return m_obj; 
    }
  }

  T& get()
  { 
    if (m_eptr != std::exception_ptr()) 
    {
      std::rethrow_exception(m_eptr);
    }
    else
    {
      return m_obj; 
    }
  }

private:
  void construct(const T& rhs)
  {
    assert(m_initialized == false);

    new (&m_obj) T(rhs);
    m_initialized = true;
  }

  void assign(const T& rhs)
  {
    if (is_initialized())
    {
      m_obj = rhs;
    }
    else
    {
      construct(rhs);
    }
  }

  void destroy()
  {
    if (is_initialized())
    {
      m_obj.~T();
    }
  }
};

#endif

/* EOF */