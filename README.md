# PyEnTT

**[简体中文](README.zh-CN.md) | English**

[![Python](https://img.shields.io/badge/python-3.10%2B-blue.svg)](https://www.python.org/downloads/)
[![License](https://img.shields.io/badge/license-MIT-green.svg)](LICENSE)
[![Tests](https://img.shields.io/badge/tests-passing-brightgreen.svg)](tests/)

Python bindings for [EnTT](https://github.com/skypjack/entt) - A high-performance Entity Component System library for game development and real-time applications.

## Features

- **High Performance**: Zero-overhead abstractions leveraging EnTT's C++ core
- **ECS Architecture**: Entity-Component-System for flexible game design
- **Type Safety**: Full Python type hints support
- **Cross-Platform**: Supports Linux, macOS, and Windows
- **Python 3.10+**: Stable ABI wheels for wide compatibility
- **Modern Python**: Uses nanobind for efficient bindings

## Installation

Install from PyPI:

```bash
pip install entt
```

Or install from source:

```bash
git clone https://github.com/your-username/PyEnTT.git
cd PyEnTT
pip install .
```

## Requirements

- Python 3.10 or higher
- C++17 compatible compiler (GCC 7+, Clang 5+, MSVC 2017+)

## Quick Start

```python
from entt import Registry, Entity, Component

# Define a component
class Position:
    def __init__(self, x: float, y: float):
        self.x = x
        self.y = y

class Velocity:
    def __init__(self, x: float, y: float):
        self.x = x
        self.y = y

# Create a registry
registry = Registry()

# Create entities
entity1 = registry.create()
entity2 = registry.create()

# Add components
registry.emplace(entity1, Position, 0.0, 0.0)
registry.emplace(entity1, Velocity, 1.0, 0.5)
registry.emplace(entity2, Position, 10.0, 5.0)

# Query entities with specific components
view = registry.view(Position, Velocity)

for entity in view:
    pos = registry.get(entity, Position)
    vel = registry.get(entity, Velocity)
    print(f"Entity {entity}: pos=({pos.x}, {pos.y}), vel=({vel.x}, {vel.y})")
```

## Advanced Usage

### Observer Pattern

```python
from entt import SignalObserver, SignalEvent

class MyObserver(SignalObserver):
    def on_construct(self, reg, entity):
        print(f"Entity {entity} constructed")
    
    def on_update(self, reg, entity):
        print(f"Entity {entity} updated")
    
    def on_destroy(self, reg, entity):
        print(f"Entity {entity} destroyed")

observer = MyObserver()
registry.connect(Position, SignalEvent.OnConstruct, observer)
```

### Reactive Storage

```python
from entt import ReactiveStorage

# Create reactive storage for reactive components
storage = registry.reactive()

# Subscribe to events
storage.on_construct(Position)
```

## Documentation

For more detailed documentation, examples, and API reference, please visit our [documentation](https://github.com/your-username/PyEnTT#readme).

## Testing

Run tests:

```bash
pytest
```

## Building from Source

```bash
# Install build dependencies
pip install nanobind scikit-build-core

# Build and install
pip install -e .
```

## Contributing

Contributions are welcome! Please see [CONTRIBUTING.md](CONTRIBUTING.md) for details.

## License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

## Acknowledgments

- [EnTT](https://github.com/skypjack/entt) - The underlying C++ ECS library
- [nanobind](https://github.com/wjakob/nanobind) - Python/C++ binding library
- [scikit-build-core](https://github.com/scikit-build/scikit-build-core) - Build system

## Links

- [EnTT Documentation](https://skypjack.github.io/entt/)
- [Python Package Index (PyPI)](https://pypi.org/project/entt/)
- [Issue Tracker](https://github.com/your-username/PyEnTT/issues)
