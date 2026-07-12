# userd

A lightweight user-level service manager for managing user-space service processes.

## Features

- **Service Management**: Start, stop, and restart services
- **Auto Restart**: Multiple restart policies (no, always, unless-stopped, on-failure)
- **Log Management**: Automatically redirect service output to log files
- **Status Query**: Query service running status
- **Hot Reload**: Reload configuration files without restarting the daemon
- **Environment Variables**: Pass environment variables from client to services
- **Working Directory**: Customize service working directory

## Project Structure

```
userd/
├── LICENSE                    # MIT License
├── Makefile                   # Build script
├── src/
│   ├── client/               # Client program
│   │   └── main.cpp         # User control tool (userctl)
│   └── server/               # Server daemon
│       ├── main.cpp          # Daemon main program
│       ├── message.hpp       # Message structure definition
│       ├── message_queue.hpp # Thread-safe message queue
│       └── service.hpp       # Service management class
└── bin/                      # Build output directory
```

## Build & Install

### Requirements

- C++17 or later
- POSIX compatible system (Linux, macOS)

### Build

```bash
# Build client and server
make

# Build debug version
make debug
```

### Install

```bash
# Install to /usr/local/bin
sudo make install
```

## Usage

### Configuration Files

Service configuration files are located in `~/.config/userd/` directory with `.service` extension.

Configuration file format:

```ini
name=service_name
cmd=command
restart_policy=restart_policy
env=environment_variables (optional)
workdir=working_directory (optional)
```

#### Restart Policies

- `no`: No auto restart
- `always`: Always auto restart
- `unless-stopped`: Restart unless manually stopped
- `on-failure`: Restart only on failure

Maximum restart count can be specified: `always:5`

#### Environment Variables

- `${SERVER}`: Use server environment variables (default)
- `${CLIENT}`: Receive environment variables from client
- `KEY1=val1,KEY2=val2`: Specify environment variables

#### Working Directory

- `${SERVER}`: Use server current directory (default)
- `${CLIENT}`: Receive working directory from client
- `/path/to/dir`: Specify working directory

### Enable Services

Add service names to be auto-started in `~/.config/userd/enable` file, one per line.

### Client Commands

```bash
# Start service
userctl start <service> [env] [workdir]

# Stop service
userctl stop <service>

# Restart service
userctl restart <service>

# View service status
userctl status [service]

# View service logs
userctl logs <service>

# Reload configuration
userctl reload
```

### Example

Create a simple service configuration:

```bash
# Create config directory
mkdir -p ~/.config/userd

# Create service config file
cat > ~/.config/userd/myapp.service << 'EOF'
name=myapp
cmd=/usr/bin/python3 /path/to/app.py
restart_policy=always:5
workdir=/path/to/app
EOF

# Enable service
echo "myapp" >> ~/.config/userd/enable

# Start daemon (usually started by system service manager)
userd &
```

## Architecture

### Daemon (userd)

- Uses Unix Domain Socket for IPC
- Single-threaded message queue for all service operations
- Uses `waitpid` to monitor child process status
- Signal handling for graceful shutdown

### Client (userctl)

- Communicates with daemon via Unix Domain Socket
- Command-line argument parsing
- Real-time service output display

### Message Protocol

Custom simple protocol:

- `o` prefix: Command message
- `e` prefix: Error message
- `c` prefix: Connection close
- `v` prefix: Verification request (env/workdir)
- `a` prefix: Verification response
- `\x04` (EOT): Message end marker

## Logs

Service logs are stored in `~/.cache/userd/` directory with filename format `<service>.log`.

## License

This project is licensed under the [MIT License](LICENSE).

## Contributing

Issues and Pull Requests are welcome!

## Author

Carry-Rao