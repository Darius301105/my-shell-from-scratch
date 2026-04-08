# my-shell-from-scratch

A Unix-like shell written in C **from scratch**, built as a personal learning project to test and expand my knowledge of low-level Linux programming — processes, signals, file descriptors, terminal control, and more.

---

## About

This shell was built entirely from zero, line by line, without copying existing implementations. The goal was to deeply understand how a shell works under the hood — from reading input to forking processes, managing pipes, and handling signals.

Throughout this project I learned and applied:
- Process management with `fork()`, `execvp()`, `wait()`
- File descriptor manipulation with `dup2()` and `pipe()`
- Terminal control with `termios` (raw mode)
- Signal handling with `signal()`, `SIGINT`, `SIGCHLD`
- Directory navigation with `opendir()`, `readdir()`
- Dynamic memory management

---

## Features

### Built-in Commands
| Command | Description |
|---|---|
| `exit` | Exit the shell |
| `cd [dir]` | Change directory (`cd`, `cd ~`, `cd ..`, `cd path`) |
| `pwd` | Print current working directory |
| `help` | Show all built-in commands |
| `history` | Show command history |

### External Commands
Any system command works out of the box — `ls`, `grep`, `cat`, `nano`, `mkdir`, `rm`, `cp`, `mv`, `ps`, `kill`, and more — all executed via `fork()` + `execvp()`.

### Pipes
Chain multiple commands together:
```bash
ls | grep txt
ls | grep c | wc
ps aux | grep bash | wc
```
Supports **unlimited pipe chaining** (More than two commands).

### I/O Redirection
```bash
ls > output.txt        # write stdout to file
sort < input.txt       # read stdin from file
```

### Background Execution
```bash
sleep 5 &              # run in background
```
Displays PID when started and notifies when the process finishes.

### Environment Variables
```bash
echo $HOME
echo $USER
echo $PATH
```

### Dynamic Prompt
The prompt always shows the current directory:
```
my-shell:/home/user/projects>
```

### Arrow Key History Navigation
Use `↑` and `↓` to navigate through previously executed commands.

### Tab Completion
Press `Tab` to autocomplete file and directory names:
- **Single match** → completes automatically
- **Multiple matches** → lists all options

### Signal Handling
- `SIGINT` (`Ctrl+C`) is handled so the shell itself does not terminate unexpectedly
- `SIGCHLD` is handled to reap finished child processes and help prevent zombie processes

---

## Build & Run

**Requirements:** `gcc`, `make`, Linux

```bash
# Clone the repo
git clone https://github.com/Darius301105/my-shell-from-scratch.git
cd my-shell-from-scratch

# Build
make

# Run
./shell
```

To clean build files:
```bash
make clean
```

---

## Project Structure

```
my-shell-from-scratch/
├── shell.c       # Main loop, built-in commands, input handling
├── utils.c       # Helper functions (execute, pipe, redirect, signals)
├── utils.h       # Function declarations
└── Makefile      # Build configuration
```

---

## What I Learned

- How shells work internally at the OS level
- Process creation and management on Linux
- How pipes connect processes via file descriptors
- How terminals work in raw vs canonical mode
- Signal handling and background process management
- Writing clean, modular C code split across multiple files
