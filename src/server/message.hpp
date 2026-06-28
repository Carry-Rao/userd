#pragma once

#include <cstdio>
#include <cstring>
#include <string>
#include <unistd.h>
#include <utility>

struct Message;
Message &operator<<(Message &, const std::string &);
Message &operator<<(Message &, std::string &&);
Message &&operator<<(Message &&, const std::string &);
Message &&operator<<(Message &&, std::string &&);
Message &operator>>(Message &, std::string &);
Message &&operator>>(Message &&, std::string &);

struct Message {
  private:
    int _fd;

  public:
    friend Message &operator<<(Message &, const std::string &);
    friend Message &operator>>(Message &, std::string &);

    const std::string opt;
    const std::string service;

    Message(std::string, std::string);
    Message(int, std::string, std::string);
    Message(const Message &) = delete;
    Message &operator=(const Message &) = delete;
    Message(Message &&) noexcept;
    Message &operator=(Message &&) noexcept;
    ~Message();
};

Message::Message(std::string operation, std::string serviceName)
    : _fd(STDERR_FILENO), opt(std::move(operation)),
      service(std::move(serviceName)) {}
Message::Message(int fd, std::string operation, std::string serviceName)
    : _fd(fd), opt(std::move(operation)), service(std::move(serviceName)) {}
Message::Message(Message &&other) noexcept
    : _fd(other._fd), opt(std::move(other.opt)),
      service(std::move(other.service)) {
    other._fd = STDERR_FILENO;
}
Message &Message::operator=(Message &&other) noexcept {
    if (this != &other) {
        if (_fd != STDERR_FILENO) {
            close(_fd);
        }
        _fd = other._fd;
        other._fd = STDERR_FILENO;
    }
    return *this;
}
Message::~Message() {
    if (_fd != STDERR_FILENO) {
        write(_fd, "c\04", 2);
        close(_fd);
    }
}

Message &operator<<(Message &msg, const std::string &str) {
    if (msg._fd < 0 || msg._fd == STDERR_FILENO)
        return msg;
    ssize_t ret = write(msg._fd, str.c_str(), str.size());
    if (ret == -1) {
        perror("write failed");
    }
    write(msg._fd, "\04", 1);
    return msg;
}
Message &operator<<(Message &msg, std::string &&str) { return msg << str; }
Message &&operator<<(Message &&msg, const std::string &str) {
    msg << str;
    return std::move(msg);
}
Message &&operator<<(Message &&msg, std::string &&str) {
    msg << std::move(str);
    return std::move(msg);
}
Message &operator>>(Message &msg, std::string &str) {
    str.clear();
    if (msg._fd < 0)
        return msg;
    if (msg._fd == STDERR_FILENO) {
        str = "a";
        return msg;
    }
    char buf[1024];
    ssize_t n;
    while (true) {
        n = read(msg._fd, buf, sizeof(buf) - 1);
        if (n <= 0) {
            if (n == -1)
                throw std::runtime_error("read failed");
            break;
        }
        for (ssize_t i = 0; i < n; ++i) {
            if (buf[i] == '\04') {
#ifdef DEBUG
                std::clog << static_cast<int>(buf[i]) << " ";
#endif
                str.append(buf, i);
                return msg;
            }
        }
        str.append(buf, static_cast<size_t>(n));
    }
    return msg;
}
Message &&operator>>(Message &&msg, std::string &str) {
    msg >> str;
    return std::move(msg);
}