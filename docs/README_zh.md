# userd

一个轻量级的用户级服务管理器，用于管理用户空间的服务进程。

## 功能特性

- **服务管理**：支持启动、停止、重启服务
- **自动重启**：支持多种重启策略（no、always、unless-stopped、on-failure）
- **日志管理**：自动将服务输出重定向到日志文件
- **状态查询**：支持查询服务运行状态
- **配置热加载**：支持重新加载配置文件而无需重启守护进程
- **环境变量支持**：支持从客户端传递环境变量给服务
- **工作目录支持**：支持自定义服务工作目录

## 项目结构

```
userd/
├── LICENSE                    # MIT 许可证
├── Makefile                   # 构建脚本
├── src/
│   ├── client/               # 客户端程序
│   │   └── main.cpp         # 用户控制工具 (userctl)
│   └── server/               # 服务端守护进程
│       ├── main.cpp          # 守护进程主程序
│       ├── message.hpp       # 消息结构定义
│       ├── message_queue.hpp # 线程安全消息队列
│       └── service.hpp       # 服务管理类
└── bin/                      # 编译输出目录
```

## 构建与安装

### 依赖要求

- C++17 或更高版本
- POSIX 兼容系统（Linux、macOS）

### 构建

```bash
# 构建客户端和服务端
make

# 构建调试版本
make debug
```

### 安装

```bash
# 安装到 /usr/local/bin
sudo make install
```

## 使用方法

### 配置文件

服务配置文件位于 `~/.config/userd/` 目录下，文件扩展名为 `.service`。

配置文件格式：

```ini
name=服务名称
cmd=执行命令
restart_policy=重启策略
env=环境变量（可选）
workdir=工作目录（可选）
```

#### 重启策略

- `no`：不自动重启
- `always`：总是自动重启
- `unless-stopped`：除非手动停止，否则总是重启
- `on-failure`：仅在失败时重启

可指定最大重启次数：`always:5`

#### 环境变量

- `${SERVER}`：使用服务器环境变量（默认）
- `${CLIENT}`：从客户端接收环境变量
- `KEY1=val1,KEY2=val2`：指定环境变量

#### 工作目录

- `${SERVER}`：使用服务器当前目录（默认）
- `${CLIENT}`：从客户端接收工作目录
- `/path/to/dir`：指定工作目录

### 启用服务

在 `~/.config/userd/enable` 文件中添加要自动启动的服务名称，每行一个。

### 客户端命令

```bash
# 启动服务
userctl start <service> [env] [workdir]

# 停止服务
userctl stop <service>

# 重启服务
userctl restart <service>

# 查看服务状态
userctl status [service]

# 查看服务日志
userctl logs <service>

# 重新加载配置
userctl reload
```

### 示例

创建一个简单的服务配置：

```bash
# 创建配置目录
mkdir -p ~/.config/userd

# 创建服务配置文件
cat > ~/.config/userd/myapp.service << 'EOF'
name=myapp
cmd=/usr/bin/python3 /path/to/app.py
restart_policy=always:5
workdir=/path/to/app
EOF

# 启用服务
echo "myapp" >> ~/.config/userd/enable

# 启动守护进程（通常由系统服务管理器启动）
userd &
```

## 架构说明

### 守护进程 (userd)

- 使用 Unix Domain Socket 进行进程间通信
- 采用单线程消息队列处理所有服务操作
- 使用 `waitpid` 监控子进程状态
- 支持信号处理实现优雅关闭

### 客户端 (userctl)

- 通过 Unix Domain Socket 与守护进程通信
- 支持命令行参数解析
- 实时显示服务输出

### 消息协议

使用自定义的简单协议：

- `o` 前缀：命令消息
- `e` 前缀：错误消息
- `c` 前缀：连接关闭
- `v` 前缀：验证请求（环境变量/工作目录）
- `a` 前缀：验证响应
- `\x04` (EOT)：消息结束标记

## 日志

服务日志存储在 `~/.cache/userd/` 目录下，文件名格式为 `<service>.log`。

## 许可证

本项目采用 [MIT 许可证](../LICENSE)。

## 贡献

欢迎提交 Issue 和 Pull Request！

## 作者

Carry-Rao