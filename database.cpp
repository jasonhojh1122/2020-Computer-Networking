#include "database.h"

Database::Database(std::string name) : name(name) {
    int err;
    err = sqlite3_open(name.c_str(), &db);
    if (err) {
        std::runtime_error("Failed to open database.");
        exit(EXIT_FAILURE);
    }
}

Database::~Database() {
    sqlite3_close(db);
}

void Database::select(sqlite3_stmt *statement, select_result *res) {

    while (sqlite3_step(statement) == SQLITE_ROW) {
        
    }
    // sqlite3_exec(db, sql.c_str(), Database::selectCallback, (void*)res, NULL);
}

int Database::selectCallback(void *data, int argc, char **argv, char **colName) {
    select_result *res = static_cast<select_result*> (data);
    int row = res->value.size();
    res->value.push_back(select_row());
    for (int i = 0; i < argc; ++i) {
        if (res->col_name_set == false) {
            res->col_name.push_back(colName[i]);
        }
        res->value[row].push_back(argv[i]);
    }
    res->col_name_set = true;
}

template <typename... Args>
void Database::insert(std::string& sql, Args&... args) {
    const char *sql = "INSERT INTO COMMENTS (COMMENT) VALUES (?)";
    if (sqlite3_prepare_v2(database, sql, -1, &statement, NULL) != SQLITE_OK) {
        NSLog(@"Prepare failure: %s", sqlite3_errmsg(database));
        return;
    }
    if (sqlite3_bind_text(statement, 1, [commentString UTF8String], -1, SQLITE_TRANSIENT) != SQLITE_OK) {
        NSLog(@"Bind 1 failure: %s", sqlite3_errmsg(database));
        sqlite3_finalize(statement);
        return;
    }
    if (sqlite3_step(statement) != SQLITE_DONE) {
        NSLog(@"Step failure: %s", sqlite3_errmsg(database));
    }
    sqlite3_finalize(statement);

}