CREATE TABLE user (
    id INTEGER PRIMARY KEY,
    account TEXT NOT NULL,
    password TEXT NOT NULL,
    id_cookie TEXT NOT NULL
);

CREATE TABLE bulletin (
    id INTEGER PRIMARY KEY,
    message TEXT,
    user_id INTEGER,
    FOREIGN KEY(user_id) REFERENCES user(id)
);