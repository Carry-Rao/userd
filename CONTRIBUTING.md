# Contributing Guide

Thank you for your interest in contributing to the userd project! This document will help you understand how to contribute.

## Table of Contents

- [How to Contribute](#how-to-contribute)
- [Development Environment Setup](#development-environment-setup)
- [Code Standards](#code-standards)
- [Commit Conventions](#commit-conventions)
- [Pull Request Process](#pull-request-process)

## How to Contribute

### Reporting Bugs

If you find a bug, please submit a report via GitHub Issues with the following information:

1. Detailed description of the problem
2. Steps to reproduce
3. Expected behavior and actual behavior
4. Operating system and compiler version
5. Relevant configuration files or logs

### Submitting Feature Suggestions

Feature suggestions should also be submitted via GitHub Issues, including:

1. Detailed description of the feature
2. Use cases
3. Expected implementation approach

### Submitting Code

1. Fork this repository
2. Create your feature branch (`git checkout -b feature/your-feature`)
3. Commit your changes (`git commit -m 'Add some feature'`)
4. Push to the branch (`git push origin feature/your-feature`)
5. Create a Pull Request

## Development Environment Setup

### Requirements

- C++17 or later
- POSIX compatible system (Linux, macOS)
- g++ or clang++ compiler

### Building the Project

```bash
# Clone the repository
git clone https://github.com/Carry-Rao/userd.git
cd userd

# Build debug version
make debug
```

### Running Tests

```bash
# Run all tests (if any)
make test
```

## Code Standards

### C++ Code Style

1. Follow [Google C++ Style Guide](https://google.github.io/styleguide/cppguide.html)
2. Use 4 spaces for indentation, no tabs
3. Line width limit of 100 characters
4. Use `snake_case` for variables and functions
5. Use `PascalCase` for classes and structs
6. Use `UPPER_CASE` for constants and macros

### File Organization

1. Header files use `.hpp` extension
2. Source files use `.cpp` extension
3. Each class or struct should be defined in a separate header file
4. Use `#pragma once` to prevent duplicate header file inclusion

### Naming Conventions

1. Class names: `PascalCase`
2. Function names: `snake_case`
3. Variable names: `snake_case`
4. Constant names: `UPPER_CASE`
5. Member variables: Use underscore prefix `_variable`

### Comment Standards

1. All public interfaces must have documentation comments
2. Complex algorithms or logic need comments
3. Use `//` for line comments
4. Use `/* */` for block comments

## Commit Conventions

Commit messages should follow this format:

```
Type: Brief description

Detailed description (optional)
```

### Types

- `Feature`: New feature
- `Fix`: Bug fix
- `Docs`: Documentation update
- `Style`: Code format adjustment (does not affect code execution)
- `Refactor`: Code refactoring
- `Test`: Add or modify tests
- `Chore`: Build process or auxiliary tool changes

### Examples

```
Feature: Add service status query functionality

Support querying service running status through userctl status command
```

```
Fix: Fix service restart failure issue

Fixed the issue where services could not restart correctly after abnormal exit
```

## Pull Request Process

1. **Fork Repository**: Fork this repository on GitHub
2. **Create Branch**: Create your feature branch from the `main` branch
3. **Write Code**: Write code according to code standards
4. **Add Tests**: Add tests for new features or bug fixes
5. **Update Documentation**: Update related documentation and comments
6. **Commit Code**: Commit code according to commit conventions
7. **Create PR**: Create a Pull Request on GitHub
8. **Code Review**: Wait for project maintainer to conduct code review
9. **Merge Code**: After approval, code will be merged to main branch

### PR Description Template

```markdown
## Description

Briefly describe your changes

## Change Type

- [ ] Bug fix
- [ ] New feature
- [ ] Documentation update
- [ ] Code refactoring
- [ ] Other

## Testing

Describe how you tested these changes

## Related Issues

Close #issue_number
```

## Getting Help

If you have any questions or need help, please contact us through:

- Creating a GitHub Issue
- Sending an email to the project maintainer

Thank you for your contribution!