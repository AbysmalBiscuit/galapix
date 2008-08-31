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

#ifndef HEADER_VIEWER_THREAD_HPP
#define HEADER_VIEWER_THREAD_HPP

#include "math/size.hpp"
#include "thread.hpp"
#include "thread_message_queue.hpp"

#include "image.hpp"
#include "job_handle.hpp"
#include "tile_entry.hpp"

class FileEntry;
class Image;
class TileEntry;

class ViewerThread
{
private:
  static ViewerThread* current_;
public:
  static ViewerThread* current() { return current_; }
  
private:
  Size geometry;
  bool fullscreen;

  ThreadMessageQueue<FileEntry>   file_queue;

public:
  ViewerThread(const Size& geometry, bool fullscreen);
  virtual ~ViewerThread();

  int run();

  void receive_file(const FileEntry& entry);

private:
  ViewerThread (const ViewerThread&);
  ViewerThread& operator= (const ViewerThread&);
};

#endif

/* EOF */
