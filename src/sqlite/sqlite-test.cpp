#include "sqlite.hpp"
#include <iostream>

int main() {
  SQLiteDB db("index.db", sqlite_open::readonly);

  SQLiteStatement stmt(db, "SELECT * FROM records");

  while (stmt.execute()) {
    int ai = stmt.getColumnIndex("sha256");
    std::cout << ai << "\n";
    std::cout << static_cast<std::string>(stmt.getColumn(ai)) << "\n";
  }
  return 0;
}
