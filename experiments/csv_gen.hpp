/*
* Copyright 2022 SCRAP
*
* This file is part of Scrappy Tablebase Generator.
* 
* Scrappy Tablebase Generator is free software: you can redistribute it and/or modify it under the terms 
* of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, 
* or (at your option) any later version.
* 
* Scrappy Tablebase Generator is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of 
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License along with Scrappy Tablebase Generator. If not, see <https://www.gnu.org/licenses/>.
*/


#ifndef CSV_GEN_HPP_
#define CSV_GEN_HPP_

#include <fstream>
#include <cstdarg>

class csv_gen
{
  std::ofstream m_ofs;
public:
  csv_gen(const std::stream& fname, 
      const std::vector<std::string>& labels)
  {
    // check file existence
    bool exists = false;
    {
      std::ifstream ifs(fname.c_str());
      exists = ifs.good();
    }
    m_ofs(fname, std::ios_base::app);
    
    // if the file does not previously exist add the column labels.
    if (!exists)
    {
      std::string line{};
      for (const auto& s : labels)
      {
        if (s != labels.back())
          line += s + ",";
        else
          line += s + "\n";
      }
    }
  }

  template<typename Types...>
  void add_row(const Types&... args)
  {
    int count = 0;
    for (const auto& e : { args... })
    {
      if (++count != sizeof...(Types))
        ofs << e << ",";
      else
        ofs << e << ";";
    }
  }
};

#endif
