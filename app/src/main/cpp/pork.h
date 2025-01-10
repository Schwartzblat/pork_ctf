#ifndef PORK_PORK_H
#define PORK_PORK_H

#endif //PORK_PORK_H

#include <jni.h>
#include <pthread.h>
#include <cstdio>
#include <unistd.h>
#include <android/log.h>
#include <vector>
#include <unordered_map>
#include <netinet/in.h>
#include <cstdlib>
#include <cstring>
#include <sys/socket.h>
#include <sys/endian.h>
#include <sys/stat.h>
#include "notes.h"

#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG, "PORK", __VA_ARGS__)

typedef void *(*thread_func_t)(void *);
constexpr int PORT = 8888;

enum Actions {
    LOGIN = 0,
    LOGOUT = 1,
    CHANGE_PASSWORD = 2,
    CREATE_NOTE = 3,
    DELETE_NOTE = 4,
    GET_NOTE = 5,
    MOVE_TO_THREAD = 6,
    DISCONNECT = 7
};
// Jump over the stack page guard:
#define STACK_OFFSET (0x69 + PAGE_SIZE * 2)

#define STRONG_USERNAME "ADMIN" // You know the username
#define STRONG_PASSWORD "WOWWWThisIsTheStrongestPasswordEver123!@#" // You don't know the password
#define HIGH_NUMBER_OF_NOTES 0x69

const std::string CTF_FLAG = "FLAG{this_is_a_fake_flag}";