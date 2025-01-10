#include "notes.h"

void check_path(const char *user) {
    // Check if the user is valid (only alphanumeric characters):
    uint32_t i = 0;
    for (i = 0; user[i] != '\0'; i++) {
        if (!isalnum(user[i])) {
            pthread_exit(nullptr);
        }
    }
    if (i > MAX_USERNAME_LENGTH || i == 0) {
        pthread_exit(nullptr);
    }
}

bool user_exists(const char *user) {
    check_path(user);
    return fs::exists(USERS_PATH / user);
}


void create_user(const char *user, const char *password) {
    if (user_exists(user)) {
        return;
    }
    fs::create_directory(USERS_PATH / user);
    std::ofstream password_file(USERS_PATH / user / PASSWORD_FILE, std::ios::out);
    password_file << password;
}


bool can_login(const char *user, const char *password) {
    if (!user_exists(user)) {
        return false;
    }
    std::ifstream password_file(USERS_PATH / user / PASSWORD_FILE, std::ios::in);
    std::stringstream real_password;
    real_password << password_file.rdbuf();
    return real_password.str() == password;
}


uint8_t get_notes_count(const char *user) {
    if (!user_exists(user)) {
        return -1;
    }
    // returns the number of notes the user has:
    int count = -1;
    for (const auto &_: fs::directory_iterator(USERS_PATH / user)) {
        count++;
    }
    return count;
}


void create_note(const char *user, const std::string &note) {
    if (!user_exists(user)) {
        return;
    }
    const uint8_t new_note_index = get_notes_count(user);
    if (new_note_index > 200) {
        return;
    }
    fs::path note_path = USERS_PATH / user / std::to_string(new_note_index);
    std::ofstream note_file(note_path, std::ios::out);
    note_file << note;
}

std::string get_note(const char *user, uint8_t index) {
    if (!user_exists(user)) {
        return "";
    }
    if (index >= get_notes_count(user)) {
        return "";
    }
    fs::path note_path = USERS_PATH / user / std::to_string(index);
    if (!fs::exists(note_path)) {
        return "";
    }
    std::ifstream note_file(note_path, std::ios::in);
    std::stringstream note;
    note << note_file.rdbuf();
    return note.str();
}

void delete_note(const char *user, uint8_t index) {
    if (!user_exists(user)) {
        return;
    }
    fs::path note_path = USERS_PATH / user / std::to_string(index);
    fs::remove(note_path);
}