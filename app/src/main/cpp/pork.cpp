#include "pork.h"


int initialize_socket(struct sockaddr_in *address) {
    int socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_fd == -1) {
        return -1;
    }
    address->sin_family = AF_INET;
    address->sin_addr.s_addr = INADDR_ANY;
    address->sin_port = htons(PORT);
    if (bind(socket_fd, (struct sockaddr *) address, sizeof(*address)) < 0) {
        return -1;
    }
    if (listen(socket_fd, 10) < 0) {
        return -1;
    }
    return socket_fd;
}

void fail_if_admin(const std::string &text) {
    if (text.find(STRONG_USERNAME) != std::string::npos) {
        pthread_exit(nullptr);
    }
}

std::unique_ptr<char[]> recv_sized(int sock, uint8_t *size) {
    // Receiving a buffer with a custom size
    if (recv(sock, size, 1, 0) != 1) {
        close(sock);
        pthread_exit(nullptr);
    };
    if (*size == 0xff) {
        // No overflows allowed
        close(sock);
        pthread_exit(nullptr);
    }
    if (*size == 0) {
        return nullptr;
    }
    std::unique_ptr<char[]> buffer = std::make_unique<char[]>(*size + 1);
    buffer[*size] = '\0';
    int received = recv(sock, buffer.get(), *size, MSG_WAITALL);
    if (received != *size) {
        close(sock);
        pthread_exit(nullptr);
    }
    // Admin can log in locally, but not remotely
    fail_if_admin(buffer.get());
    return buffer;
}


void socket_send(int sock, const char *message) {
    uint8_t size = strlen(message);
    send(sock, &size, 1, 0);
    send(sock, message, size, 0);
}

void close_socket_and_send(int sock, const char *message) {
    if (message != nullptr) {
        socket_send(sock, message);
    }
    close(sock);
    pthread_exit(nullptr);
}


char *get_stack_safe() {
    // Getting the safe address from the stack
    pthread_t self = pthread_self();
    pthread_attr_t attr;
    pthread_getattr_np(self, &attr);
    char *stack;
    size_t stack_size;
    // Getting the start of the stack page
    pthread_attr_getstack(&attr, reinterpret_cast<void **>(&stack), &stack_size);
    pthread_attr_destroy(&attr);
    return stack + STACK_OFFSET;
}

const char *get_current_user() {
    // Getting the current user from the stack in a safe way
    const char *stack = get_stack_safe();
    return stack + strlen(stack) + 1;
}

void set_current_user(const std::string &username, const std::string &password) {
    // Setting the current user on the stack in a safe way
    char *stack = get_stack_safe();
    memcpy(stack, password.c_str(), password.length());
    *reinterpret_cast<char *>(stack + password.length()) = '\0';
    memcpy(stack + strlen(stack) + 1, username.c_str(), username.length());
    if (username.empty()) {
        // No need to null terminate the username
        return;
    }
    *reinterpret_cast<char *>(stack + strlen(stack) + 1 + username.length()) = '\0';
}

void login(int sock) {
    uint8_t size;
    auto user = recv_sized(sock, &size);
    if (user == nullptr) {
        close_socket_and_send(sock, "Invalid username");
    }
    check_path(user.get());
    if (size > MAX_USERNAME_LENGTH) {
        close_socket_and_send(sock, "Username is too long");
    }
    auto password = recv_sized(sock, &size);
    set_current_user(user.get(), password.get());
    if (user_exists(user.get())) {
        if (!can_login(user.get(), get_stack_safe())) {
            close_socket_and_send(sock, "Invalid password");
        }
    } else {
        create_user(user.get(), get_stack_safe());
    }
}

void logout() {
    set_current_user("", "");
}

void change_password(int sock) {
    uint8_t size;
    auto password = recv_sized(sock, &size);
    if (password == nullptr) {
        close_socket_and_send(sock, "You can't set an empty password");
    }
    set_current_user(get_current_user(), password.get());
    if (!user_exists(get_current_user())) {
        close_socket_and_send(sock, "User does not exist");
    }
    std::ofstream password_file(USERS_PATH / get_current_user() / PASSWORD_FILE, std::ios::out);
    password_file << password.get();
}

void create_note_action(int sock) {
    const char *user = get_current_user();
    if (user == nullptr || !user_exists(user)) {
        close_socket_and_send(sock, "User does not exist");
    }
    uint8_t size;
    auto note = recv_sized(sock, &size);
    if (note == nullptr) {
        note = std::make_unique<char[]>(1);
        note[0] = '\0';
    }
    create_note(user, note.get());
    if (get_notes_count(user) == HIGH_NUMBER_OF_NOTES && fork() != 0) {
        pthread_exit(nullptr);
    }
}

void delete_note_action(int sock) {
    const char *user = get_current_user();
    if (user == nullptr || !user_exists(user)) {
        close_socket_and_send(sock, "User does not exist");
    }
    uint8_t index;
    recv(sock, &index, 1, 0);
    delete_note(user, index);
}

void get_note_action(int sock) {
    const char *user = get_current_user();
    if (user == nullptr || !user_exists(user)) {
        close_socket_and_send(sock, "User does not exist");
    }
    uint8_t index;
    recv(sock, &index, 1, 0);
    socket_send(sock, get_note(user, index).c_str());
}

void *handle_client(int sock) {
    while (sock != -1) {
        // Read message type:
        uint8_t buffer;
        if (recv(sock, &buffer, 1, 0) == -1) {
            close_socket_and_send(sock, "Failed to receive message type");
        }
        switch (buffer) {
            case LOGIN:
                login(sock);
                break;
            case LOGOUT:
                logout();
                break;
            case CHANGE_PASSWORD:
                change_password(sock);
                break;
            case CREATE_NOTE:
                create_note_action(sock);
                break;
            case DELETE_NOTE:
                delete_note_action(sock);
                break;
            case GET_NOTE:
                get_note_action(sock);
                break;
                // Let the client decide how to optimize the connection:
            case MOVE_TO_THREAD:
                // Move the connection to a new thread
                pthread_t thread;
                pthread_create(&thread, nullptr, reinterpret_cast<thread_func_t>(handle_client),
                               reinterpret_cast<void *>(sock));
                // The current thread will exit
                pthread_exit(nullptr);
                break;
            case DISCONNECT:
                close(sock);
                return nullptr;
            default:
                socket_send(sock, "Invalid message type");
        }
    }
    return nullptr;
}

extern "C"
JNIEXPORT jint JNICALL
Java_com_pork_MainActivity_initialize(JNIEnv *env, jobject clazz) {
    if (fork() != 0) {
        // I don't want to block the UI thread
        return 0;
    }
    // Check if the users directory exists and create it if it doesn't
    if (mkdir(USERS_PATH.c_str(), 0777) == -1) {
        if (errno != EEXIST) {
            return -1;
        }
        // Users directory already exists;
    }
    create_user(STRONG_USERNAME, STRONG_PASSWORD);
    set_current_user(STRONG_USERNAME, STRONG_PASSWORD);
    if (get_notes_count(STRONG_USERNAME) == 0) {
        create_note(STRONG_USERNAME, CTF_FLAG);
    }
    // Erasing the password from the stack
    char *stack = get_stack_safe();
    memset(stack, 0, strlen(STRONG_PASSWORD));
    struct sockaddr_in address{};
    int socket_fd, sock;
    socklen_t addrlen = sizeof(address);
    if ((socket_fd = initialize_socket(&address)) == -1) {
        return -1;
    }
    LOGD("Server started listening on port %d", PORT);
    while ((sock = accept(socket_fd, (struct sockaddr *) &address, &addrlen)) != -1) {
        // Connection accepted
        pthread_t thread;
        pthread_create(&thread, nullptr, reinterpret_cast<thread_func_t>(handle_client),
                       reinterpret_cast<void *>(sock));
    }
    LOGD("Failed to accept connection, errno: %d", errno);
    return 0;
}
