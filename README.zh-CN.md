# PyEnTT

**English | [简体中文](README.zh-CN.md)**

[![Python](https://img.shields.io/badge/python-3.10%2B-blue.svg)](https://www.python.org/downloads/)
[![License](https://img.shields.io/badge/license-MIT-green.svg)](LICENSE)
[![Tests](https://img.shields.io/badge/tests-passing-brightgreen.svg)](tests/)

[EnTT](https://github.com/skypjack/entt) 的 Python 绑定 - 一个用于游戏开发和实时应用的高性能实体组件系统（ECS）库。

## 特性

- **高性能**: 利用 EnTT 的 C++ 核心，零开销抽象
- **ECS 架构**: 实体-组件-系统，支持灵活的游戏设计
- **类型安全**: 完整的 Python 类型提示支持
- **跨平台**: 支持 Linux、macOS 和 Windows
- **Python 3.10+**: Stable ABI wheels，广泛兼容
- **现代 Python**: 使用 nanobind 实现高效绑定

## 安装

从 PyPI 安装：

```bash
pip install entt
```

或从源码安装：

```bash
git clone https://github.com/your-username/PyEnTT.git
cd PyEnTT
pip install .
```

## 系统要求

- Python 3.10 或更高版本
- C++17 兼容的编译器（GCC 7+、Clang 5+、MSVC 2017+）

## 快速开始

```python
from entt import Registry, Entity, Component

# 定义组件
class Position:
    def __init__(self, x: float, y: float):
        self.x = x
        self.y = y

class Velocity:
    def __init__(self, x: float, y: float):
        self.x = x
        self.y = y

# 创建注册表
registry = Registry()

# 创建实体
entity1 = registry.create()
entity2 = registry.create()

# 添加组件
registry.emplace(entity1, Position(0.0, 0.0))
registry.emplace(entity1, Velocity(1.0, 0.5))
registry.emplace(entity2, Position(10.0, 5.0))

# 查询具有特定组件的实体
view = registry.view(Position, Velocity)

for entity in view:
    pos = registry.get(entity, Position)
    vel = registry.get(entity, Velocity)
    print(f"实体 {entity}: 位置=({pos.x}, {pos.y}), 速度=({vel.x}, {vel.y})")
```

## 高级用法

### 观察者模式

```python
from entt import SignalObserver, SignalEvent

class MyObserver(SignalObserver):
    def on_construct(self, reg, entity):
        print(f"实体 {entity} 已创建")
    
    def on_update(self, reg, entity):
        print(f"实体 {entity} 已更新")
    
    def on_destroy(self, reg, entity):
        print(f"实体 {entity} 已销毁")

observer = MyObserver()
observer.connect(registry, Position, SignalEvent.OnConstruct)
```

### 响应式存储

```python
from entt import ReactiveStorage

# 为响应式组件创建响应式存储
storage = registry.reactive(Position)

# 订阅事件
storage.on_construct(lambda entity: print(f"创建了 {entity}"))
```

## 文档

更详细的文档、示例和 API 参考，请访问我们的[文档](https://github.com/your-username/PyEnTT#readme)。

## 测试

运行测试：

```bash
pytest
```

## 从源码构建

```bash
# 安装构建依赖
pip install nanobind scikit-build-core

# 构建并安装
pip install -e .
```

## 贡献

欢迎贡献！请参阅 [CONTRIBUTING.md](CONTRIBUTING.md) 了解详情。

## 许可证

本项目采用 MIT 许可证 - 详见 [LICENSE](LICENSE) 文件。

## 致谢

- [EnTT](https://github.com/skypjack/entt) - 底层 C++ ECS 库
- [nanobind](https://github.com/wjakob/nanobind) - Python/C++ 绑定库
- [scikit-build-core](https://github.com/scikit-build/scikit-build-core) - 构建系统

## 相关链接

- [EnTT 文档](https://skypjack.github.io/entt/)
- [Python 包索引 (PyPI)](https://pypi.org/project/entt/)
- [问题追踪](https://github.com/your-username/PyEnTT/issues)
