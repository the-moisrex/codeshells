#include <iostream>
#include <mysql++/mysql++.h>

using namespace mysqlpp;
using namespace std;

auto main() -> int {

  Connection con("main", "localhost", "root", "toor", 3306);

  Query query = con.query();
  query << "select item from list;";
  auto res = query.store();

  cout << "We have: " << endl;
  if (res) {
    Row row;
    Row::size_type i;
    if (!res.size()) {
      cout << "nothing to print" << endl;
      return 0;
    }
    for (i = 0; i < res.size(); ++i) {
      row = res.at(i);
      cout << '\t' << row.at(0) << endl;
    }
  } else {
    cerr << "Failed to get the items: " << query.error() << endl;
    return 1;
  }

  return 0;
}

