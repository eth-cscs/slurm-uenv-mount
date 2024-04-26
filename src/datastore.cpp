#include "datastore.hpp"
#include "util/helper.hpp"
#include "util/sqlite.hpp"
#include <set>
#include <stdexcept>

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

std::string find_repo_image(const uenv_desc &desc) {
  std::string dbpath = "/scratch/index.db";
  SQLiteDB db(dbpath, sqlite_open::readonly);

  if (desc.sha) {
    if (desc.sha.value().size() < 64) {
      SQLiteStatement query(db, "SELECT * FROM records WHERE id = " + desc.sha.value());
      // query.bind(1, desc.sha.value());
    } else {
      SQLiteStatement query(db, "SELECT * FROM records WHERE sha256 = " + desc.sha.value());
    }
  } else {
    std::string query_str = "SELECT * FROM records WHERE ";
    std::vector<std::pair<std::string, std::string>> filter;
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

    // get all results
    std::set<std::string> shas;
    while(query.execute()) {
      int aIndex = query.getColumnIndex("sha256");
      shas.insert(query.getColumn(aIndex));
    }
    if(shas.size() > 1) {
      throw std::runtime_error("nope");
    }
  }

  return "TODO";
}
