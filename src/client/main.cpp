#include <cstring>
#include <filesystem>
#include <iostream>
#include <string>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <vector>
constexpr char EOF_MARK = 0x04;

bool send_pkt(int fd, const std::string &s) {
    std::string pkt = s;
    pkt.push_back(EOF_MARK);
    ssize_t ret = write(fd, pkt.data(), pkt.size());
    if (ret != (ssize_t)pkt.size()) {
        perror("write");
        return false;
    }
    return true;
}

bool extract(std::vector<char> &buf, std::string &out) {
    for (size_t i = 0; i < buf.size(); i++) {
        if (buf[i] == EOF_MARK) {
            out.assign(buf.begin(), buf.begin() + i);
            buf.erase(buf.begin(), buf.begin() + i + 1);
            return true;
        }
    }
    return false;
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        std::cerr << "usage: " << argv[0]
                  << " <cmd> <service> [env] [workdir]\n";
        return 1;
    }
    std::string socket_path = std::filesystem::path("/run/user") /
                              std::to_string(getuid()) / "userd.socket";
    int sfd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (sfd == -1) {
        perror("socket");
        return 1;
    }
    struct sockaddr_un addr{};
    addr.sun_family = AF_UNIX;
    std::strncpy(addr.sun_path, socket_path.c_str(), sizeof(addr.sun_path) - 1);
    if (connect(sfd, (struct sockaddr *)&addr, sizeof(addr)) == -1) {
        perror("connect");
        close(sfd);
        return 1;
    }
    std::string cmd = argv[1];
    std::string service = argc >= 3 ? argv[2] : "";
    std::string env = argc >= 4 ? argv[3] : "";
    std::string wd = argc >= 5 ? argv[4] : "";
    if (!send_pkt(sfd, "o" + cmd + " " + service)) {
        close(sfd);
        return 1;
    }
    std::vector<char> buf;
    char recv_buf[1024];
    std::string pkt;
    while (true) {
        ssize_t n = read(sfd, recv_buf, sizeof(recv_buf));
        if (n <= 0)
            break;
        buf.insert(buf.end(), recv_buf, recv_buf + n);
        while (extract(buf, pkt)) {
            if (pkt.empty())
                continue;
            char head = pkt[0];
            std::string body = pkt.substr(1);
            if (head == 'c')
                goto clean;
            if (head == 'e')
                std::cerr << "\033[31m[ERROR]: " << body << "\033[0m\n";
            if (head == 'o')
                std::cout << body;
            if (head == 'v' && pkt.size() >= 2) {
                char sub = pkt[1];
                if (sub == 'e')
                    send_pkt(sfd, "a" + env);
                if (sub == 'w')
                    send_pkt(sfd, "a" + wd);
            }
        }
    }
clean:
    close(sfd);
    return 0;
}