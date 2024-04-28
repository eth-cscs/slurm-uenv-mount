#include "datastore.hpp"
#include "config.hpp"
#include "sqlite/sqlite.hpp"
#include "src/util/expected.hpp"
#include "util/helper.hpp"
#include <set>
#include <sstream>

/*
 return dictionary{"name", "version", "tag", "sha" } from a uenv description
 string

 prgenv_gnu              ->("prgenv_gnu", None, None, None)
 prgenv_gnu/23.11        ->("prgenv_gnu", "23.11", None, None)
 prgenv_gnu/23.11:latest ->("prgenv_gnu", "23.11", "latest", None)
 prgenv_gnu:v2           ->("prgenv_gnu", None, "v2", None)
 3313739553fe6553        ->(None, None, None, "3313739553fe6553")
 */
uenv_desc parse_uenv_string(const std::string &str) {
  uenv_desc res;

  if (is_sha(str)) {
    res.sha = str;
  }

  auto splits = split(str, '/');
  if (splits.size() > 1) {
    res.name = splits.at(0);
    splits = split(splits.at(1), ':', 1);
    res.version = splits.at(0);
    if (splits.size() > 1) {
      res.tag = splits[1];
    }
  } else {
    splits = split(str, ':', 1);
    res.name = splits.at(0);
    if (splits.size() > 1) {
      res.tag = splits[1];
    }
  }

  return res;
}

uenv_desc to_desc(SQLiteStatement &stmt) {
  uenv_desc desc;
  desc.name = stmt.getColumn(stmt.getColumnIndex("name"));
  desc.sha = stmt.getColumn(stmt.getColumnIndex("sha256"));
  desc.tag = stmt.getColumn(stmt.getColumnIndex("tag"));
  desc.version = stmt.getColumn(stmt.getColumnIndex("version"));
  return desc;
}

struct cmp {
  bool operator()(const uenv_desc &d1, const uenv_desc &d2) const {
    return d1.sha < d2.sha;
  }
};

util::expected<std::string, std::string>
find_repo_image(const uenv_desc &desc, const std::string &repo_path) {
  std::string dbpath = repo_path + "/index.db";
  SQLiteDB db(dbpath, sqlite_open::readonly);

  // get all results
  std::set<uenv_desc, cmp> shas;
  if (desc.sha) {
    if (desc.sha.value().size() < 64) {
      SQLiteStatement query(db, "SELECT * FROM records WHERE id = " +
                                    desc.sha.value());
    } else {
      SQLiteStatement query(db, "SELECT * FROM records WHERE sha256 = " +
                                    desc.sha.value());
    }
  } else {
    std::string query_str = "SELECT * FROM records WHERE ";
    std::vector<std::pair<std::string, std::string>> filter;
    // only search for current uarch
    filter.push_back(std::make_pair("uarch", UENV_UARCH));

    if (desc.name) {
      filter.push_back(std::make_pair("name", desc.name.value()));
    }
    if (desc.version) {
      filter.push_back(std::make_pair("version", desc.version.value()));
    }
    if (desc.tag) {
      filter.push_back(std::make_pair("tag", desc.tag.value()));
    }
    for (size_t i = 0; i < filter.size(); ++i) {
      if (i > 0) {
        query_str += " AND ";
      }
      query_str += filter[i].first + " = " + filter[i].second;
    }
    SQLiteStatement query(db, query_str);

    while (query.execute()) {
      shas.insert(to_desc(query));
    }
    if (shas.size() > 1) {
      std::stringstream ss;
      ss << "[error]  more than one uenv matches.\n";
      for (auto &d : shas) {
        ss << d.name.value() << "/" << d.version.value() << ":" << d.tag.value()
           << "\t" << d.sha.value() << "\n";
      }
      return util::unexpected(ss.str());
    }
  }

  return repo_path + "/" + shas.begin()->sha.value() + "/store.squashfs";
}
