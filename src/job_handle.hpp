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

#ifndef HEADER_JOB_HANDLE_HPP
#define HEADER_JOB_HANDLE_HPP

#include <boost/shared_ptr.hpp>

class JobHandleImpl;

/** A JobHandle should be returend whenever one thread makes a request
    to another thread, the JobHandle allows the calling thread to
    cancel the job and the called thread to inform the calling one
    that the Job is finished. (FIXME: Do we need that last thing for something?) */
class JobHandle
{
public:
  JobHandle();
  ~JobHandle();

  void abort();
  bool is_aborted() const;
  
  void finish();
  bool is_finished() const;

private:
  boost::shared_ptr<JobHandleImpl> impl;
};

#endif

/* EOF */