#include <filesystem>
#include <fstream>
#include <iostream>
#include <map>
#include <message.hpp>
#include <set>
#include <message_queue.hpp>
#include <mutex>
#include <service.hpp>
#include <stdexcept>
#include <stop_token>
#include <string>
#include <string_view>
#include <sys/socket.h>
#include <sys/un.h>
#include <thread>

std::map<std::string, std::string> parse_config(const std::filesystem::path& path) {
    std::map<std::string, std::string> config;
    std::ifstream is(path);
    std::string line;
    while (std::getline(is, line)) {
        if (line.empty() || line[0] == '#') continue;
        size_t pos = line.find('=');
        if (pos != std::string::npos) {
            config[line.substr(0, pos)] = line.substr(pos + 1);
        }
    }
    return config;
}

void waiter(std::stop_token st, std::map<std::string, Service>& services, std::mutex& mtx) {
    sigset_t set;
    sigemptyset(&set);
    sigaddset(&set, SIGCHLD);
    pthread_sigmask(SIG_BLOCK, &set, nullptr);

    while (!st.stop_requested()) {
        int status;
        pid_t pid = waitpid(-1, &status, WNOHANG);
        if (pid > 0) {
            std::lock_guard<std::mutex> lock(mtx);
            for (auto& [n, s] : services) {
                if (s.pid() == pid) {
                    s.handle_exit(status);
                    break;
                }
            }
        } else {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
    }
}

int main() {
    std::map<std::string, Service> services;
    std::mutex services_mtx;
    MessageQueue<Message> mq;

    auto config_path =
        std::filesystem::path(getenv("HOME")) / ".config" / "userd";
    std::filesystem::create_directories(config_path);
    for (const auto &entry :
         std::filesystem::recursive_directory_iterator(config_path)) {
        if (entry.is_regular_file() && entry.path().extension() == ".service") {
            auto config = parse_config(entry.path());
            services.emplace(config["name"],
                             Service(config["restart_policy"], config, mq));
        }
    }
    {
        if (!std::filesystem::exists(config_path / "enable")) {
            std::ofstream enable_os(config_path / "enable");
        }
        std::ifstream enable_is(config_path / "enable");
        std::string line;
        while (std::getline(enable_is, line)) {
            if (!line.empty() && line[0] != '#') {
                mq.send(Message{"start", line});
            }
        }
    }

    std::stop_source ss;
    std::jthread waiter_thread(waiter, ss.get_token(), std::ref(services), std::ref(services_mtx));

    std::jthread t([&mq]() {
        auto socket_path = std::filesystem::path("/run/user") /
                           std::to_string(getuid()) / "userd.socket";
        int sfd = socket(AF_UNIX, SOCK_STREAM, 0);
        sockaddr_un addr{};
        addr.sun_family = AF_UNIX;
        strcpy(addr.sun_path, socket_path.c_str());
        unlink(socket_path.c_str());
        bind(sfd, (sockaddr *)&addr, sizeof(addr));
        listen(sfd, 1);
        while (true) {
            int cfd = accept(sfd, nullptr, nullptr);
            char buf[1024];
            size_t len = read(cfd, buf, sizeof(buf));
            if (buf[0] != 'o') {
                write(cfd, "eInvalid message format\0", 25);
                continue;
            }
            std::string_view str(buf + 1, len - 1);
            size_t pos = str.find('\04');
            if (pos != std::string::npos) {
                str = str.substr(0, pos);
            }
            pos = str.find(' ');
            if (pos != std::string::npos) {
                mq.send(Message{cfd, std::string(str.substr(0, pos)),
                                std::string(str.substr(pos + 1))});
            } else {
                write(cfd, "eInvalid message format\0", 25);
            }
        }
    });
    while (true) {
        auto msg = mq.receive();
        if (msg.opt == "exit") {
            msg << "Exiting...";
            break;
        }
        if (msg.opt == "reload") {
            std::lock_guard<std::mutex> lock(services_mtx);
            std::set<std::string> seen;
            for (const auto &entry :
                 std::filesystem::recursive_directory_iterator(config_path)) {
                if (!entry.is_regular_file() || entry.path().extension() != ".service") continue;
                auto config = parse_config(entry.path());
                auto name = config["name"];
                seen.insert(name);
                auto si = services.find(name);
                if (si != services.end()) {
                    if (si->second.status() == Status::Running) continue;
                    si->second = Service(config["restart_policy"], config, mq);
                } else {
                    services.emplace(name, Service(config["restart_policy"], config, mq));
                }
            }
            for (auto si = services.begin(); si != services.end(); ) {
                if (seen.find(si->first) == seen.end()) {
                    si->second.stop();
                    si = services.erase(si);
                } else {
                    ++si;
                }
            }
            msg << "oReloaded";
            continue;
        }
        if (msg.opt == "status") {
            if (msg.service.empty()) {
                std::string report;
                for (auto& [n, s] : services) {
                    report += n + ": " + to_string(s.status());
                    if (s.status() == Status::Running) {
                        report += " (pid " + std::to_string(s.pid()) + ")";
                    }
                    report += "\n";
                }
                if (report.empty()) report = "No services";
                msg << ("o" + report);
            } else {
                auto si = services.find(msg.service);
                if (si != services.end()) {
                    std::string report = si->first + ": " + to_string(si->second.status());
                    if (si->second.status() == Status::Running) {
                        report += " (pid " + std::to_string(si->second.pid()) + ")";
                    }
                    msg << ("o" + report);
                } else {
                    msg << "eService not found";
                }
            }
            continue;
        }
        auto it = services.find(msg.service);
        if (it != services.end()) {
            if (msg.opt == "start") {
                if (it->second.env_from_client) {
                    msg << "ve";
                    std::string env;
                    msg >> env;
                    if (env[0] != 'a') {
                        msg << "eInvalid environment format";
                        continue;
                    }
                    it->second.setenv(env.substr(1));
                }
                if (it->second.workdir_from_client) {
                    msg << "vw";
                    std::string workdir;
                    msg >> workdir;
                    if (workdir[0] != 'a') {
                        msg << "eInvalid workdir format";
                        continue;
                    }
                    it->second.setworkdir(workdir.substr(1));
                }
                msg << "oStarting";
                it->second.start();
            } else if (msg.opt == "stop") {
                msg << "oStopping";
                it->second.stop();
            } else if (msg.opt == "restart") {
                msg << "oRestarting";
                it->second.restart();
            } else {
                msg << "eInvalid option.";
            }
        } else {
            msg << "eService not found";
        }
    }
    ss.request_stop();
    return 0;
}
