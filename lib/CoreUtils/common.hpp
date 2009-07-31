#ifndef ISISCOMMON_HPP
#define ISISCOMMON_HPP

// template<class InputIterator,typename _CharT, typename _Traits> std::basic_ostream<_CharT, _Traits> &
// write_list(InputIterator start,InputIterator end,
//                 std::basic_ostream<_CharT, _Traits> &o,
//                 std::string delim=" ",
//                 std::string prefix="",std::string suffix=""){
//   o << prefix;
//   if(start!=end)o << *start; start++;
//   for(InputIterator i=start;i!=end;i++)
//     o << delim << *i;
//   o << suffix;
//   return o;
// }

#include "../common.hpp"

namespace isis{
struct CoreLog{enum {use_rel = _ENABLE_CORE_LOG};};
}
#endif //ISISCOMMON_HPP