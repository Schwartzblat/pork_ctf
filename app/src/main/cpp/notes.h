#ifndef PORK_NOTES_H
#define PORK_NOTES_H

#include <string>
#include <fstream>
#include <iostream>
#include <sstream>
#include <filesystem>

#endif //PORK_NOTES_H

namespace fs = std::filesystem;
const fs::path USERS_PATH = "/data/data/com.pork/users/";
const fs::path PASSWORD_FILE = "password";
#define MAX_USERNAME_LENGTH 18


void check_path(const char *user);

void create_user(const char *user, const char *password);

bool can_login(const char *user, const char *password);

void create_note(const char *user, const std::string &note);

void delete_note(const char *user, uint8_t index);

std::string get_note(const char *user, uint8_t index);

uint8_t get_notes_count(const char *user);

bool user_exists(const char *user);