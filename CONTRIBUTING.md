# Contributing to PyEnTT

Thank you for your interest in contributing to PyEnTT! This document provides guidelines and instructions for contributing to the project.

## Table of Contents

- [Code of Conduct](#code-of-conduct)
- [Getting Started](#getting-started)
- [Development Setup](#development-setup)
- [Making Changes](#making-changes)
- [Testing](#testing)
- [Submitting Changes](#submitting-changes)
- [Style Guidelines](#style-guidelines)

## Code of Conduct

By participating in this project, you agree to abide by our Code of Conduct:
- Be respectful and inclusive
- Focus on what is best for the community
- Show empathy towards other community members

## Getting Started

### Prerequisites

- Python 3.10 or higher
- C++17 compatible compiler (GCC 7+, Clang 5+, MSVC 2017+)
- Git
- uv (Python package manager)

## Development Setup

1. **Fork and Clone**

   ```bash
   git clone https://github.com/your-username/PyEnTT.git
   cd PyEnTT
   ```

2. **Install Dependencies**

   ```bash
   # Install uv if needed
   pip install uv
   
   # Install development dependencies
   uv pip install -e .
   uv pip install pytest
   ```

3. **Build the Project**

   ```bash
   uv build --wheel
   ```

4. **Verify Installation**

   ```bash
   python -c "import entt; print(entt.__version__)"
   ```

## Making Changes

### Workflow

1. Create a new branch for your feature or bugfix:

   ```bash
   git checkout -b feature/your-feature-name
   # or
   git checkout -b fix/your-bugfix-name
   ```

2. Make your changes following the style guidelines

3. Write tests for your changes

4. Ensure all tests pass

5. Commit your changes with a descriptive message

6. Push to your fork

7. Create a pull request

### Branch Naming

- `feature/` - New features
- `fix/` - Bug fixes
- `docs/` - Documentation changes
- `refactor/` - Code refactoring
- `test/` - Test additions or modifications

## Testing

### Running Tests

```bash
# Run all tests
uv run pytest

# Run specific test file
uv run pytest tests/test_registry.py

# Run with verbose output
uv run pytest -v

# Run with coverage
uv run pytest --cov=entt
```

### Writing Tests

- Write tests for all new features
- Ensure tests are clear and concise
- Use pytest fixtures where appropriate
- Follow the existing test patterns in the `tests/` directory

## Submitting Changes

### Pull Request Process

1. Ensure your PR description clearly describes the problem and solution
2. Include the relevant issue number (e.g., "Fixes #123")
3. Update documentation as needed
4. Ensure all CI checks pass
5. Request review from maintainers

### Commit Message Format

Follow conventional commits format:

```
<type>(<scope>): <subject>

<body>

<footer>
```

Types:
- `feat`: New feature
- `fix`: Bug fix
- `docs`: Documentation changes
- `style`: Code style changes (formatting, etc.)
- `refactor`: Code refactoring
- `test`: Test changes
- `chore`: Maintenance tasks

Example:

```
feat(core): add entity validation

Add a new validation method to ensure entity IDs are valid
before component operations. This prevents potential crashes
when operating on invalid entities.

Closes #42
```

## Style Guidelines

### Python Code

- Follow PEP 8 style guidelines
- Use type hints for all public functions and methods
- Keep functions focused and single-purpose
- Write docstrings for all public APIs
- Maximum line length: 100 characters

### C++ Code

- Follow the existing code style in `src/registry_wrapper.cpp`
- Use modern C++ features (C++17)
- Keep functions short and well-named
- Add comments for complex logic
- Follow nanobind conventions for bindings

### Documentation

- Use clear, concise language
- Include code examples
- Keep examples up-to-date
- Use markdown for formatting

## Reporting Issues

When reporting issues, please include:

1. A clear description of the problem
2. Steps to reproduce the issue
3. Expected behavior
4. Actual behavior
5. Environment information:
   - Python version
   - Operating system
   - PyEnTT version
   - Relevant error messages or logs

## Questions and Discussions

For questions and discussions:
- Use GitHub Issues for bug reports and feature requests
- Use GitHub Discussions for general questions and ideas

## License

By contributing to PyEnTT, you agree that your contributions will be licensed under the MIT License.

## Getting Help

If you need help:
- Check the [documentation](README.md)
- Search [existing issues](https://github.com/your-username/PyEnTT/issues)
- Ask in [GitHub Discussions](https://github.com/your-username/PyEnTT/discussions)
- Contact maintainers via issue tracker

Thank you for contributing to PyEnTT! 🎉
