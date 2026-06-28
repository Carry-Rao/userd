#pragma once

#include <cstring>
#include <fcntl.h>
#include <map>
#include <message.hpp>
#include <message_queue.hpp>
#include <signal.h>
#include <stdexcept>
#include <string>
#include <sys/wait.h>
#include <thread>
#include <unistd.h>
#include <vector>
#include <cstdlib>
#include <sys/stat.h>

enum class Status { Stopped, Running, Failed, Killed, Unknown };

struct RestartPolicy {
private:
    enum class RP { No, Always, UnlessStopped, OnFailure };
    RP _policy;
    unsigned int _max_restart_time;
    unsigned int _restart_time = 0;

public:
    RestartPolicy(std::string policy) {
        if (policy.empty()) {
            _policy = RP::No;
            _max_restart_time = 0;
            return;
        }
        size_t pos = policy.find(':');
        if (pos != std::string::npos) {
            try {
                _max_restart_time = std::stoi(policy.substr(pos + 1));
            } catch (const std::invalid_argument&) {
                throw std::invalid_argument("Invalid restart time format");
            } catch (const std::out_of_range&) {
                throw std::out_of_range("Restart time out of range");
            }
            policy = policy.substr(0, pos);
        } else {
            _max_restart_time = -1;
        }
        if (policy == "no") {
            _policy = RP::No;
        } else if (policy == "always") {
            _policy = RP::Always;
        } else if (policy == "unless-stopped") {
            _policy = RP::UnlessStopped;
        } else if (policy == "on-failure") {
            _policy = RP::OnFailure;
        } else {
            throw std::invalid_argument("Invalid restart policy");
        }
    }

    bool need(Status status) {
        if (_policy == RP::No) {
            return false;
        }
        if (_restart_time >= _max_restart_time) {
            return false;
        }
        _restart_time++;
        switch (_policy) {
        case RP::Always:
            return true;
        case RP::OnFailure:
            return status == Status::Failed;
        case RP::UnlessStopped:
            return status != Status::Stopped;
        default:
            return false;
        }
    }
};

struct Service {
private:
    std::string name;
    Status _status = Status::Stopped;
    RestartPolicy _restart_policy;
    std::jthread _monitor;

    pid_t _pid = -1;
    char** _cmd = nullptr;
    char** _env = nullptr;
    char* _workdir = nullptr;

    void freeCmd();
    void freeEnv();

public:
    bool env_from_client = false;
    bool workdir_from_client = false;

    Service(std::string, std::map<std::string, std::string>);
    Service(const Service &) = delete;
    Service &operator=(const Service &) = delete;
    Service(Service &&);
    Service &operator=(Service &&);
    ~Service();

    void start(MessageQueue<Message> &queue);
    void stop();
    void restart(MessageQueue<Message> &queue);
    void setenv(std::string);
    void setworkdir(std::string);
};

Service::Service(std::string restart_policy, std::map<std::string, std::string> options)
    : _restart_policy(restart_policy) {
    auto it = options.find("name");
    if (it != options.end()) {
        name = it->second;
    } else {
        throw std::invalid_argument("Service name not specified");
    }

    it = options.find("cmd");
    if (it == options.end()) {
        throw std::invalid_argument("Command not specified");
    }

    std::vector<char*> argv;
    std::string cmd_str = it->second;
    size_t pos = cmd_str.find(' ');
    while (pos != std::string::npos) {
        argv.push_back(strdup(cmd_str.substr(0, pos).c_str()));
        cmd_str = cmd_str.substr(pos + 1);
        pos = cmd_str.find(' ');
    }
    argv.push_back(strdup(cmd_str.c_str()));

    _cmd = new char*[argv.size() + 1];
    for (size_t i = 0; i < argv.size(); i++) {
        _cmd[i] = argv[i];
    }
    _cmd[argv.size()] = nullptr;

    _env = environ;
    it = options.find("env");
    if (it != options.end()) {
        if (it->second == "${CLIENT}") {
            env_from_client = true;
        } else if (it->second != "${SERVER}") {
            std::vector<std::string> env_vars;
            size_t start = 0;
            size_t end = it->second.find(',');
            while (end != std::string::npos) {
                env_vars.push_back(it->second.substr(start, end - start));
                start = end + 1;
                end = it->second.find(',', start);
            }
            env_vars.push_back(it->second.substr(start));
            _env = new char*[env_vars.size() + 1];
            for (size_t i = 0; i < env_vars.size(); i++) {
                _env[i] = strdup(env_vars[i].c_str());
            }
            _env[env_vars.size()] = nullptr;
        }
    }

    _workdir = new char[1024];
    getcwd(_workdir, 1024);
    it = options.find("workdir");
    if (it != options.end()) {
        if (it->second == "${CLIENT}") {
            workdir_from_client = true;
        } else if (it->second != "${SERVER}") {
            delete[] _workdir;
            _workdir = new char[it->second.size() + 1];
            strcpy(_workdir, it->second.c_str());
        }
    }
}

Service::Service(Service &&other)
    : name(std::move(other.name)), _status(other._status),
      _restart_policy(other._restart_policy), _monitor(std::move(other._monitor)),
      _pid(other._pid), _cmd(other._cmd), _env(other._env), _workdir(other._workdir),
      env_from_client(other.env_from_client), workdir_from_client(other.workdir_from_client) {
    other._cmd = nullptr;
    other._env = nullptr;
    other._workdir = nullptr;
}

Service &Service::operator=(Service &&other) {
    if (this != &other) {
        freeCmd();
        freeEnv();
        delete[] _workdir;

        name = std::move(other.name);
        _status = other._status;
        _restart_policy = other._restart_policy;
        _monitor = std::move(other._monitor);
        _pid = other._pid;
        _cmd = other._cmd;
        _env = other._env;
        _workdir = other._workdir;
        env_from_client = other.env_from_client;
        workdir_from_client = other.workdir_from_client;

        other._cmd = nullptr;
        other._env = nullptr;
        other._workdir = nullptr;
    }
    return *this;
}

void Service::freeCmd() {
    if (!_cmd) return;
    for (int i = 0; _cmd[i]; i++) {
        free(_cmd[i]);
    }
    delete[] _cmd;
    _cmd = nullptr;
}

void Service::freeEnv() {
    if (_env == environ || !_env) return;
    for (int i = 0; _env[i]; i++) {
        free(_env[i]);
    }
    delete[] _env;
    _env = nullptr;
}

Service::~Service() {
    stop();
    freeCmd();
    freeEnv();
    delete[] _workdir;
    if (_monitor.joinable()) {
        _monitor.join();
    }
}

void Service::start(MessageQueue<Message> &queue) {
    pid_t pid = fork();
    if (pid == 0) {
        int null_fd = open("/dev/null", O_RDONLY);
        dup2(null_fd, 0);
        close(null_fd);

        const char* home = getenv("HOME");
        std::string log_dir = std::string(home) + "/.cache/userd";
        mkdir(log_dir.c_str(), 0755);
        std::string log_file = log_dir + "/" + name + ".log";

        int log_fd = open(log_file.c_str(), O_WRONLY | O_CREAT | O_APPEND, 0644);
        dup2(log_fd, 1);
        dup2(log_fd, 2);
        close(log_fd);

        chdir(_workdir);
        execve(_cmd[0], _cmd, _env);
        exit(1);
    }
    _pid = pid;
    _status = Status::Running;

    _monitor = std::jthread([&queue, this]() {
        int status;
        waitpid(_pid, &status, 0);
        if (WIFSIGNALED(status)) {
            _status = Status::Killed;
        } else if (WIFEXITED(status)) {
            _status = WEXITSTATUS(status) == 0 ? Status::Stopped : Status::Failed;
        } else {
            _status = Status::Unknown;
        }
        if (_restart_policy.need(_status)) {
            queue.send(Message{"start", name});
        }
    });
}

void Service::stop() {
    _status = Status::Stopped;
    if (_pid > 0 && kill(_pid, 0) == 0) {
        kill(_pid, SIGTERM);
    }
}

void Service::restart(MessageQueue<Message> &queue) {
    stop();
    start(queue);
}

void Service::setenv(std::string env) {
    if (env.empty()) return;
    freeEnv();
    std::vector<std::string> env_vars;
    size_t start = 0;
    size_t end = env.find(',');
    while (end != std::string::npos) {
        env_vars.push_back(env.substr(start, end - start));
        start = end + 1;
        end = env.find(',', start);
    }
    env_vars.push_back(env.substr(start));
    _env = new char*[env_vars.size() + 1];
    for (size_t i = 0; i < env_vars.size(); i++) {
        _env[i] = strdup(env_vars[i].c_str());
    }
    _env[env_vars.size()] = nullptr;
}

void Service::setworkdir(std::string workdir) {
    if (workdir.empty()) return;
    delete[] _workdir;
    _workdir = new char[workdir.size() + 1];
    strcpy(_workdir, workdir.c_str());
}