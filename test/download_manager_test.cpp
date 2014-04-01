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

#include <iostream>
#include <thread>
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <boost/uuid/string_generator.hpp>

#include "network/download_manager.hpp"
#include "network/download_result.hpp"
#include "util/log.hpp"

int main(int argc, char** argv)
{
  g_logger.set_log_level(Logger::kDebug);

  DownloadManager downloader;

  std::cout << "Commands: get, post, cancel, cancel_all" << std::endl;

  std::string line;
  while(std::getline(std::cin, line))
  {
    std::vector<std::string> args;
    boost::split(args, line, boost::is_any_of(" "), boost::token_compress_on);

    if (!args.empty())
    {
      if (args[0] == "get")
      {
        if (args.size() != 2)
        {
          std::cout << "'get' requires two arguments" << std::endl;
        }
        else
        {
          auto handle =
            downloader.request_get(args[1], 
                                   [=](const DownloadResult& result) {

                                     std::cout << "results for " << args[1] << std::endl;
                                     std::cout << "  response code     : " << result.get_response_code() << std::endl;
                                     std::cout << "  modification time : " << result.get_mtime() << std::endl;

                                     if (result.success())
                                     {
                                       std::cout.write(reinterpret_cast<const char*>(result.get_blob()->get_data()),
                                                       result.get_blob()->size());
                                     }
                                     else
                                     {
                                       std::cout << "failure: " << result.get_response_code() << std::endl;
                                     }
                                   },
                                   [=](double dltotal, double dlnow, double ultotal, double ulnow) -> bool {
                                     std::cout << args[1] << ": " << dlnow/1000 << " / " << dltotal/1000 << std::endl;
                                     return false;
                                   });
          std::cout << "launched " << handle << std::endl;
        }
      }
      else if (args[0] == "post")
      {
        if (args.size() != 3)
        {
          std::cout << "'post' requires three arguments" << std::endl;
        }
        else
        {
          auto handle =
            downloader.request_post(args[1], 
                                    args[2],
                                    [=](const DownloadResult& result) {
                                      std::cout << "got " << result.get_response_code() << " for " << " " << args[1] << std::endl;
                                      if (result.success())
                                      {
                                        std::cout.write(reinterpret_cast<const char*>(result.get_blob()->get_data()),
                                                        result.get_blob()->size());
                                      }
                                      else
                                      {
                                        std::cout << "failure: " << result.get_response_code() << std::endl;
                                      }
                                    },
                                    [=](double dltotal, double dlnow, double ultotal, double ulnow) -> bool {
                                      std::cout << args[1] << ": " << dlnow/1000 << " / " << dltotal/1000 << std::endl;
                                      return false;
                                    });
          std::cout << "launched " << handle << std::endl;
        }
      }
      else if (args[0] == "cancel_all")
      {
        downloader.cancel_all_transfers();
      }
      else if (args[0] == "cancel")
      {
        if (args.size() != 2)
        {
          std::cout << "'cancel' requires one arguments" << std::endl;
        }
        else
        {
          downloader.cancel_transfer(boost::lexical_cast<DownloadManager::TransferHandle>(args[1]));
        }
      }
      else
      {
        std::cout << "unknown command: " << args[0] << std::endl;
      }
    }
  }

  return 0;
}

/* EOF */