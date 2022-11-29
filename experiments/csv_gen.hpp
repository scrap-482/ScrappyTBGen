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
