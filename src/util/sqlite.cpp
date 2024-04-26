#include "sqlite.hpp"
#include <exception>
#include <map>
#include <sqlite3.h>

std::map<sqlite_open, int> sqlite_oflag = {
    {sqlite_open::readonly, SQLITE_OPEN_READONLY}};

class SQLiteError : public std::exception {
public:
  SQLiteError(const std::string &msg) : msg(msg) {}

  const char *what() const noexcept override { return msg.c_str(); }

private:
  std::string msg;
};

SQLiteDB::SQLiteDB(const std::string &fname, sqlite_open flag) {
  int rc =
      sqlite3_open_v2(fname.c_str(), &this->db, sqlite_oflag.at(flag), NULL);
  if (rc != SQLITE_OK) {
    throw SQLiteError("Couldn't open database");
  }
};

SQLiteDB::~SQLiteDB() { sqlite3_close(this->db); }

/// SQLiteColumn
SQLiteColumn::SQLiteColumn(SQLiteStatement &statement, int index)
    : statement(statement), index(index) {}

std::string SQLiteColumn::getColumnName() const {
  return sqlite3_column_name(this->statement.stmt, this->index);
}

SQLiteColumn::operator int() const {
  auto res = sqlite3_column_type(this->statement.stmt, this->index);
  if (res != SQLITE_INTEGER) {
    throw SQLiteError("Wrong column type requested");
  }
  return sqlite3_column_int(this->statement.stmt, this->index);
}

SQLiteColumn::operator std::string() const {
  const unsigned char *txt =
      sqlite3_column_text(this->statement.stmt, this->index);
  return reinterpret_cast<const char *>(txt);
}

/// SQLiteStatement
SQLiteStatement::SQLiteStatement(SQLiteDB &db, const std::string &query)
    : db(db) {
  const char *tail;
  int rc = sqlite3_prepare_v2(db.get(), query.c_str(), -1, &this->stmt, &tail);
  if (rc != SQLITE_OK) {
    throw SQLiteError(sqlite3_errmsg(db.get()));
  }
  column_count = sqlite3_column_count(this->stmt);
}

int SQLiteStatement::getColumnIndex(const std::string &name) {
  for (int i = 0; i < this->column_count; ++i) {
    if (this->getColumn(i).getColumnName() == name)
      return i;
  }
  return -1;
}

void SQLiteStatement::checkIndex(int i) const {
  if (i >= this->column_count) {
    throw SQLiteError("Column out of range");
  }
}

std::string SQLiteStatement::getColumnType(int i) {
  checkIndex(i);
  const char *result = sqlite3_column_decltype(this->stmt, i);
  if (!result) {
    throw SQLiteError("Could not determine declared column type.");
  } else {
    return result;
  }
}

SQLiteColumn SQLiteStatement::getColumn(int i) {
  checkIndex(i);
  if (this->rc != SQLITE_ROW) {
    throw SQLiteError("Statement invalid");
  }
  return SQLiteColumn(*this, i);
}

bool SQLiteStatement::execute() {
  if ((this->rc = sqlite3_step(this->stmt)) == SQLITE_ROW) {
    return true;
  }
  return false;
}

SQLiteStatement::~SQLiteStatement() { sqlite3_finalize(this->stmt); }
