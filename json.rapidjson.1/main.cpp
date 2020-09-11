#include <iostream>
#include <rapidjson/document.h>

using namespace std;
using namespace rapidjson;

constexpr auto json_file = R"json(
{
    "hello": "world",
    "t": true ,
    "f": false,
    "n": null,
    "i": 123,
    "pi": 3.1416,
    "a": [1, 2, 3, 4]
}
)json";

auto main() -> int {

  Document doc;
  doc.Parse(json_file);

  if (!doc.IsObject()) {
    cerr << "It's not an object" << endl;
    return 1;
  }

  cout << "t=" << (doc["t"].GetBool() ? "true" : "false") << endl;


  auto const& a = doc["a"];
  for (size_t i = 0; i < a.Size(); i++)
    cout << a[i].GetInt() << endl;

  return 0;
}

