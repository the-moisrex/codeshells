#include "pch.h"

namespace overmath {
  using namespace std;

  struct parameter {
    string name;
    string type;
  };

  struct assignment_statement {
    string assignie;
    string value;
  };

  struct function {
    string name;
    vector<parameter> params;
    vector<assignment_statement> assignments;
  };
}

BOOST_FUSION_ADAPT_STRUCT(
    overmath::function,
    (std::string, name)
    (std::vector<overmath::parameter>, params)
    (std::vector<overmath::assignment_statement>, assignments)
)

BOOST_FUSION_ADAPT_STRUCT(
                          overmath::parameter,
                          (std::string, name)
                          (std::string, type)
)

BOOST_FUSION_ADAPT_STRUCT(
                          overmath::assignment_statement,
                          (std::string, assignie)
                          (std::string, value)
)


namespace overmath {

  using namespace boost::spirit::unicode;
  namespace qi = boost::spirit::qi;

  template <typename Iter>
  struct function_parser : qi::grammer<Iter, function(), space_type> {

    qi::rule<Iter, function(), space_type> start;

    function_parser () : function_parser::base_type(start) {}

  };

}
