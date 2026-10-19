#pragma once
#include <ios>
#include <locale>

namespace algorithm_assets::mesh::detail {
struct StreamFormat {
  std::ios& stream;
  std::ios::fmtflags flags;
  std::streamsize precision;
  std::locale locale;
  explicit StreamFormat(std::ios& s):stream(s),flags(s.flags()),precision(s.precision()),locale(s.getloc()) {
    s.imbue(std::locale::classic());
    s.setf(std::ios::fmtflags(0),std::ios::floatfield);
    s.precision(17);
  }
  ~StreamFormat(){try{stream.flags(flags);stream.precision(precision);stream.imbue(locale);}catch(...){}}
};
} // namespace algorithm_assets::mesh::detail
