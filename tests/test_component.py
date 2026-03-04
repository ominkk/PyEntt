import pytest

from entt import Registry


class DummyComponent:
    def __init__(self, value):
        self.value = value


class AnotherComponent:
    def __init__(self, tag):
        self.tag = tag


def test_get_all_basic(reg: Registry):
    # Case 1: Entity has both components
    e = reg.create()
    obj1 = DummyComponent(123)
    obj2 = AnotherComponent("x")
    reg.emplace(e, DummyComponent, 123)
    reg.emplace(e, AnotherComponent, "x")
    all_comps = reg.get_all(e)
    val_types = {type(c) for c in all_comps}
    assert DummyComponent in val_types and AnotherComponent in val_types
    # Case 2: Only one component
    e2 = reg.create()
    reg.emplace(e2, DummyComponent, 55)
    all_comps2 = reg.get_all(e2)
    assert len(all_comps2) == 1 and isinstance(all_comps2[0], DummyComponent)
    # Case 3: No component
    e3 = reg.create()
    all_comps3 = reg.get_all(e3)
    assert all_comps3 == ()
    # Case 4: Invalid entity (destroyed)
    e4 = reg.create()
    reg.destroy(e4)
    all_comps4 = reg.get_all(e4)
    assert all_comps4 == ()


@pytest.fixture
def reg():
    return Registry()


# --- Component emplace/get/try_get/patch/erase ---
def test_emplace_and_get(reg: Registry):
    e = reg.create()
    obj = DummyComponent(42)
    comp = reg.emplace(e, DummyComponent, 42)
    assert isinstance(comp, DummyComponent)
    assert comp.value == 42
    # get
    comp2 = reg.get(e, DummyComponent)
    assert comp2.value == 42
    # try_get (exists)
    comp3 = reg.try_get(e, DummyComponent)
    assert comp3 is not None and comp3.value == 42
    # try_get (not exists)
    assert reg.try_get(e, AnotherComponent) is None


def test_emplace_and_get_with_type_name(reg: Registry):
    e = reg.create()
    comp = reg.emplace(e, DummyComponent, 99)
    assert isinstance(comp, DummyComponent)
    assert comp.value == 99
    # get
    comp2: DummyComponent = reg.get(e, "DummyComponent")
    assert comp2.value == 99
    # try_get (exists)
    comp3: DummyComponent | None = reg.try_get(e, "DummyComponent")
    assert comp3 is not None and comp3.value == 99
    # try_get (not exists)
    assert reg.try_get(e, "AnotherComponent") is None


def test_emplace_duplicate_raises(reg: Registry):
    e = reg.create()
    reg.emplace(e, DummyComponent, 1)
    with pytest.raises(Exception) as exc:
        reg.emplace(e, DummyComponent, 2)
    assert "Entity already has component" in str(exc.value)


def test_emplace_object(reg: Registry):
    e = reg.create()
    obj = DummyComponent(5)
    comp = reg.emplace(e, obj)
    assert comp.value == 5


def test_emplace_or_replace(reg: Registry):
    e = reg.create()
    reg.emplace(e, DummyComponent, 1)
    comp = reg.emplace_or_replace(e, DummyComponent, 2)
    assert comp.value == 2
    # Using object
    obj = DummyComponent(3)
    comp2 = reg.emplace_or_replace(e, obj)
    assert comp2.value == 3


def test_patch(reg: Registry):
    e = reg.create()
    reg.emplace(e, DummyComponent, 7)
    comp = reg.patch(e, DummyComponent, 8)
    assert comp.value == 8


def test_patch_nonexistent_raises(reg: Registry):
    e = reg.create()
    with pytest.raises(Exception) as exc:
        reg.patch(e, DummyComponent, 11)
    assert "Entity does not exist in" in str(exc.value)


def test_remove_and_erase(reg: Registry):
    e = reg.create()
    reg.emplace(e, DummyComponent, 20)
    removed = reg.remove(e, DummyComponent)
    assert removed == 1
    # Remove again should be 0
    assert reg.remove(e, DummyComponent) == 0
    # Emplace again and erase
    reg.emplace(e, DummyComponent, 22)
    reg.erase(e, DummyComponent)
    # Erase again should raise
    with pytest.raises(Exception) as exc:
        reg.erase(e, DummyComponent)
    assert "Entity does not exist in" in str(exc.value)


def test_clear(reg: Registry):
    e1 = reg.create()
    e2 = reg.create()
    reg.emplace(e1, DummyComponent, 1)
    reg.emplace(e2, DummyComponent, 2)
    reg.clear()
    # All entities should be invalid now
    assert not reg.valid(e1)
    assert not reg.valid(e2)


# --- test remove/erase with multiple types (args) ---
def test_remove_multiple_types(reg: Registry):
    e = reg.create()
    reg.emplace(e, DummyComponent, 1)
    reg.emplace(e, AnotherComponent, "tag")
    removed = reg.remove(e, DummyComponent, AnotherComponent)
    assert removed == 2


def test_erase_multiple_types(reg: Registry):
    e = reg.create()
    reg.emplace(e, DummyComponent, 1)
    reg.emplace(e, AnotherComponent, "tag")
    reg.erase(e, DummyComponent, AnotherComponent)
    # Both erased, so next erase should raise
    with pytest.raises(Exception):
        reg.erase(e, DummyComponent)
    with pytest.raises(Exception):
        reg.erase(e, AnotherComponent)
