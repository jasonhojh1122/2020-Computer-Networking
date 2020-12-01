#ifndef _DATABASE_H
#define _DATABASE_H
#include <string>
#include <stdexcept>
#include <functional>
#include <vector>
#include <sqlite3.h>

typedef std::vector<std::string> select_row;

struct select_result {
    std::vector<std::string> col_name;
    std::vector<select_row> value;
    bool col_name_set = false;
};

class Database {
public:
    Database(std::string names);
    ~Database();

    void select(std::string& sql, select_result *res);

    void execute(sqlite3_stmt *statement);

private:
    std::string name;
    sqlite3 *db;

    static int selectCallback(void *data, int argc, char **argv, char **colName);


};

#endif