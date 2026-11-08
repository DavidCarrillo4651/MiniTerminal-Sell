# MyBash

A modular UNIX shell implementation written in C for the Systems Programming curriculum at the Faculty of Mathematics, Astronomy, Physics, and Computation (FaMAF), National University of Córdoba (UNC).

`MyBash` implements process execution and system call orchestration (`fork`, `execvp`, `waitpid`), input/output redirection, background process management, and multi-stage pipelines (`|`).

## Key Features

- **Process Control:** Handles foreground and background command execution with custom signal handling (`SIGINT`, `SIGCHLD`).
- **Pipelines & Redirection:** Multi-stage pipeline parsing and execution using POSIX `pipe` and `dup2` stream redirection.
- **Modular Architecture:** Clear separation between command parsing, abstract syntax tree (AST) construction, and process execution.
- **Architecture Detection:** Automatically links precompiled syntax object files (`objects-<arch>`) based on host platform architecture.

## Dependencies

The project targets Linux environments (including WSL2 on Windows) and requires the following packages:

* Build Tools: `gcc`, `make`, `pkg-config`
* Libraries: `glib-2.0`
* Testing Framework: `check`
* Memory Auditing: `valgrind` *(optional)*

On Debian/Ubuntu-based systems, dependencies can be installed via:

```bash
sudo apt update && sudo apt install -y build-essential pkg-config libglib2.0-dev check valgrind
```

## Building & Usage

### 1. Build
Compile the project into an executable binary:

```bash
make
```

The Makefile evaluates system host architecture (`uname -m`) to link the appropriate precompiled lexer and parser binaries.

### 2. Execution
Run the interactive shell:

```bash
./mybash
```

### 3. Clean
Remove compiled binaries and intermediate build artifacts:

```bash
make clean
```

## Test Suite

Automated unit tests are located in `tests/` and built with the `check` framework.

| Target | Description |
| :--- | :--- |
| `make test` | Runs the complete unit test suite. |
| `make test-command` | Evaluates command structure parsing and argument allocation. |
| `make test-parsing` | Validates line parsing and pipeline data structures. |
| `make memtest` | Executes test targets under **Valgrind** to detect memory leaks. |

## Repository Structure

```text
.
├── Makefile             # Build automation and platform architecture handling
├── *.c / *.h            # Shell entry point, command parser, and execution logic
├── objects-*/           # Platform-specific precompiled lexer and parser objects
└── tests/               # Unit tests using the Check framework
```
