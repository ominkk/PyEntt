import pytest

from entt import Registry


class DummyComponent:
    def __init__(self, value):
        self.value = value


class AnotherComponent:
    def __init__(self, tag):
        self.tag = tag


@pytest.fixture
def reg():
    return Registry()


def test_view_iter_single_component(reg: Registry):
    # create entities, some with DummyComponent
    e1 = reg.create()
    e2 = reg.create()
    e3 = reg.create()
    reg.emplace(e1, DummyComponent, 10)
    reg.emplace(e3, DummyComponent, 30)
    reg.emplace(e2, AnotherComponent, "foo")
    # view DummyComponent
    view = reg.view(DummyComponent)
    entities = set([e for e, _ in view])
    assert set(entities) == set([e1, e3])


def test_view_iter_multiple_components(reg: Registry):
    e1 = reg.create()
    e2 = reg.create()
    reg.emplace(e1, DummyComponent, 10)
    reg.emplace(e1, AnotherComponent, "x")
    reg.emplace(e2, DummyComponent, 20)
    # view entities with both components
    view = reg.view(DummyComponent, AnotherComponent)
    entities = set([e for e, _, _ in view])
    print(list(entities)[0] == e1)
    # Only e1 has both
    assert entities == {e1}


def test_view_exclude(reg: Registry):
    e1 = reg.create()
    e2 = reg.create()
    reg.emplace(e1, DummyComponent, 100)
    reg.emplace(e1, AnotherComponent, "x")
    reg.emplace(e2, DummyComponent, 200)
    # view DummyComponent, exclude AnotherComponent
    view = reg.view(DummyComponent)
    view.exclude(AnotherComponent)
    entities = set([e for e, _ in view])
    # Only e2 should remain
    assert entities == {e2}


def test_view_iterate_method(reg: Registry):
    e1 = reg.create()
    e2 = reg.create()
    reg.emplace(e1, DummyComponent, 1)
    reg.emplace(e2, DummyComponent, 2)
    # view DummyComponent
    view = reg.view(DummyComponent)
    # use iterate() method (should return self for chaining)
    ret = view.iterate(DummyComponent)
    assert ret is view
    entities = set([e for e, _ in view])
    assert entities == {e1, e2}


def test_view_empty(reg: Registry):
    # No entities yet
    view = reg.view(DummyComponent)
    entities = list(view)
    assert entities == []

    e = reg.create()
    # Still empty since e has no DummyComponent
    assert list(view) == []

    reg.emplace(e, AnotherComponent, "not dummy")


def test_view_invalid_component_type(reg: Registry):
    # Unregistered class (not used in any emplace)
    class Unregistered:
        pass

    # Should not raise, just return empty view
    view = reg.view(Unregistered)
    assert list(view) == []


def test_remove_and_readd_component(reg: Registry):
    e = reg.create()
    reg.emplace(e, DummyComponent, 123)
    view = reg.view(DummyComponent)
    assert set(e for e, _ in view) == {e}
    # 假设有 remove 或 destroy_component 方法
    reg.remove(e, DummyComponent)
    assert set(e for e, _ in view) == set()
    reg.emplace(e, DummyComponent, 456)
    assert set(e for e, _ in view) == {e}


def test_view_entity_destroyed(reg: Registry):
    e1 = reg.create()
    reg.emplace(e1, DummyComponent, 99)
    view = reg.view(DummyComponent)
    assert set(e for e, _ in view) == {e1}
    reg.destroy(e1)
    # 销毁后不应再出现在 view 里
    assert set(e for e, _ in view) == set()


def test_chain_exclude_iterate(reg: Registry):
    e1 = reg.create()
    e2 = reg.create()
    e3 = reg.create()
    reg.emplace(e1, DummyComponent, 1)
    reg.emplace(e2, DummyComponent, 2)
    reg.emplace(e3, DummyComponent, 3)
    reg.emplace(e2, AnotherComponent, "a")
    reg.emplace(e3, AnotherComponent, "b")
    # view DummyComponent，连续 exclude AnotherComponent 和再次 exclude DummyComponent（无副作用）
    view = reg.view(DummyComponent)
    ret = view.exclude(AnotherComponent)
    assert ret is view
    # 只有 e1 符合条件
    assert set(e for e, _ in view) == {e1}


def test_view_single_entity_edge_cases(reg: Registry):
    # 仅有一个实体且有组件
    e = reg.create()
    reg.emplace(e, DummyComponent, 42)
    view = reg.view(DummyComponent)
    entities = list(view)
    assert len(entities) == 1
    assert entities[0][0] == e or entities[0] == e  # 兼容不同view实现
    # 多次遍历结果一致
    assert list(view) == entities
    # 移除组件后应变空
    reg.remove(e, DummyComponent)
    assert list(view) == []

    # 多余参数（如 view() 中传无关类型）
    class Dummy2:
        pass

    assert list(reg.view(Dummy2)) == []
    # 仅有一个实体但没有组件
    e2 = reg.create()
    assert list(reg.view(DummyComponent)) == []


def test_view_empty_component_set(reg: Registry):
    # 创建一些实体
    e1 = reg.create()
    e2 = reg.create()
    reg.emplace(e1, DummyComponent, 1)
    reg.emplace(e2, DummyComponent, 2)
    # view() 不带任何组件类型，应返回所有实体
    view = reg.view().iterate(DummyComponent)
    entities = set(e for (e,) in view)
    assert entities == {e1, e2}
