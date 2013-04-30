/*
**  Galapix - an image viewer for large image collections
**  Copyright (C) 2013 Ingo Ruhnke <grumbel@gmx.de>
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

#include "resource/blob_manager.hpp"

#include "network/download_manager.hpp"
#include "resource/resource_locator.hpp"
#include "util/format.hpp"

BlobManager::BlobManager(DownloadManager& download_mgr,
                         ArchiveManager&  archive_mgr) :
  m_download_mgr(download_mgr),
  m_archive_mgr(archive_mgr),
  m_pool(4),
  m_mutex(),
  m_requests()
{
}

BlobManager::~BlobManager()
{
  // FIXME: keep track of all async requests and only close down when all of them are done
}

void
BlobManager::request_blob(const ResourceLocator& locator,
                          const std::function<void (Failable<BlobPtr>)>& callback)
{
  // register_request()

  if (!locator.get_handler().empty())
  {
    //callback(Failable<BlobPtr>::from_exception(std::runtime_error("not a valid blob locator: " + locator.str())));
    callback(Failable<BlobPtr>::from_exception(std::runtime_error("handler are not supported")));
  }
  else
  {
    ResourceURL url = locator.get_url();
    if (url.get_scheme() == "file")
    {
      m_pool.schedule
        ([url, callback]
         {
           try 
           {
             BlobPtr blob = Blob::from_file(url.get_path());

             // magick handler stuff

             callback(blob);
           }
           catch(const std::exception& err)
           {
             callback(Failable<BlobPtr>(std::current_exception()));
           }
         });
    }
    else if (url.get_scheme() == "http"  ||
             url.get_scheme() == "https" ||
             url.get_scheme() == "ftp")
    {
      m_download_mgr.request_get
        (url.str(), 
         [=](const DownloadResult& result)
         {
           if (result.success())
           {
             callback(Failable<BlobPtr>(result.get_blob()));
           }
           else
           {
             callback(Failable<BlobPtr>::from_exception(std::runtime_error(format("%s: error: invalid response code: %d", 
                                                                                  url.str(), result.get_response_code()))));
           }
         });
    }
    else
    {
      Failable<BlobPtr> failable;
      failable.set_exception(std::make_exception_ptr(std::runtime_error(format("%s: error: unsupported URL scheme: %s", 
                                                                               locator.str(), url.get_scheme()))));
      callback(failable);
    }
  }
}

/* EOF */
