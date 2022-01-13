#include "pch.h"

using namespace std;
using namespace boost::json;

auto main() -> int {

  std::string data{R"json({
  "pi": 3.141,
  "happy": true,
  "name": "Boost",
  "nothing": null,
  "answer": {
    "everything": 42
  },
  "list": [1, 0, 2],
  "object": {
    "currency": "USD",
    "value": 42.99
  }
})json"};

  boost::system::error_code ec;
  value d = parse(data, ec);
  if (ec) {
    cerr << "Error" << endl;
    return 0;
  } else {
    cout << d.as_object().at("answer").at("everything") << endl;
  }

  object obj;
  obj["pi"] = 3.1415;
  obj["happy"] = true;

  cout << serialize(obj) << endl;

  return 0;
}
