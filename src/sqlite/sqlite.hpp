#include <string>

struct sqlite3_stmt;
struct sqlite3;

enum class sqlite_open : int { readonly };

class SQLiteError;

class SQLiteStatement;

class SQLiteDB {
public:
  SQLiteDB(const std::string &fname, sqlite_open flag);
  SQLiteDB(const SQLiteDB &) = delete;
  SQLiteDB operator=(const SQLiteDB &) = delete;
  virtual ~SQLiteDB();

private:
  sqlite3 *db{nullptr};

protected:
  sqlite3 *get() { return db; }

  friend SQLiteStatement;
};

class SQLiteColumn;

class SQLiteStatement {
public:
  SQLiteStatement(SQLiteDB &db, const std::string &query);
  SQLiteStatement(const SQLiteStatement &) = delete;
  SQLiteStatement operator=(const SQLiteStatement &) = delete;
  std::string getColumnType(int i);
  SQLiteColumn getColumn(int i);
  int getColumnIndex(const std::string& name);
  bool execute();

  virtual ~SQLiteStatement();

private:
  void checkIndex(int i) const;

private:
  SQLiteDB &db;
  sqlite3_stmt *stmt;
  int column_count{-1};
  int rc;
  friend SQLiteColumn;
};

class SQLiteColumn {
public:
  SQLiteColumn(SQLiteStatement &statement, int index);
  std::string getColumnName() const;
  operator int() const;
  operator std::string () const;

private:
  SQLiteStatement &statement;
  int index;
};
