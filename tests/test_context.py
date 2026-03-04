from typing import TYPE_CHECKING

import pytest

from entt import Registry


class DummyContext:
    def __init__(self, value):
        self.value = value


class AnotherContext:
    def __init__(self, tag):
        self.tag = tag


@pytest.fixture
def reg():
    return Registry()


def test_context_emplace(reg: Registry):
    ctx = reg.ctx()
    obj = DummyContext(123)
    # emplace returns the object if not present
    res = ctx.emplace(DummyContext, 123)
    assert isinstance(res, DummyContext)
    assert res.value == 123
    # emplace again returns the existing one (not new)
    res2 = ctx.emplace(DummyContext, 456)
    assert res2 is res and res2 is not None
    assert res2.value == 123


def test_context_insert_or_assign(reg: Registry):
    ctx = reg.ctx()
    obj = DummyContext(999)
    res = ctx.insert_or_assign(obj)
    assert res is obj
    # insert_or_assign with another instance replaces
    obj2 = DummyContext(111)
    res2 = ctx.insert_or_assign(obj2)
    assert res2 is obj2


def test_context_get(reg: Registry):
    ctx = reg.ctx()
    # get before emplace returns None or raises
    with pytest.raises(Exception):
        ctx.get(DummyContext)
    ctx.emplace(DummyContext, 222)
    res = ctx.get(DummyContext)
    assert isinstance(res, DummyContext)
    assert res.value == 222


def test_context_erase(reg: Registry):
    ctx = reg.ctx()
    ctx.emplace(DummyContext, 333)
    # Erase returns True
    assert ctx.erase(DummyContext) is True
    # Erase again returns False
    assert ctx.erase(DummyContext) is False
    # Insert and erase again for another type
    ctx.emplace(AnotherContext, "tag")
    assert ctx.erase(AnotherContext) is True
    # Now get should raise
    with pytest.raises(Exception):
        ctx.get(AnotherContext)


def test_context_contains(reg: Registry):
    ctx = reg.ctx()
    # Should not contain DummyContext at first
    assert ctx.contains(DummyContext) is False
    # After emplace, should contain
    ctx.emplace(DummyContext, 1)
    assert ctx.contains(DummyContext) is True
    # Should not contain AnotherContext
    assert ctx.contains(AnotherContext) is False
    # Insert AnotherContext, then contains returns True
    ctx.emplace(AnotherContext, "x")
    assert ctx.contains(AnotherContext) is True
    # After erase, should not contain
    ctx.erase(DummyContext)


def test_context_find(reg: Registry):
    ctx = reg.ctx()
    # Should return None before emplace
    assert ctx.find(DummyContext) is None
    # After emplace, should return the object
    obj = ctx.emplace(DummyContext, 42)
    found = ctx.find(DummyContext)
    assert found is obj and found is not None
    assert found.value == 42
    # Erase and then find should return None again
    ctx.erase(DummyContext)
    assert ctx.find(DummyContext) is None
    # Insert_or_assign also makes find work
    obj2 = DummyContext(99)
    ctx.insert_or_assign(obj2)
    found2 = ctx.find(DummyContext)
    assert found2 is obj2
    # Erase again
    ctx.erase(DummyContext)
    assert ctx.find(DummyContext) is None
    # AnotherContext should be None initially
    assert ctx.find(AnotherContext) is None
    ctx.emplace(AnotherContext, "abc")
    found_other = ctx.find(AnotherContext)
    assert isinstance(found_other, AnotherContext)
    assert found_other.tag == "abc"
    ctx.erase(AnotherContext)
    assert ctx.find(AnotherContext) is None


def test_context_emplace_str(reg: Registry):
    ctx = reg.ctx()
    obj = DummyContext(321)
    res = ctx.emplace("Dummy", obj)
    assert isinstance(res, DummyContext)
    assert res.value == 321
    # emplace with same type_name returns the existing one
    obj2 = DummyContext(654)
    res2 = ctx.emplace("Dummy", obj2)
    assert res2 is res
    assert res2.value == 321


def test_context_insert_or_assign_str(reg: Registry):
    ctx = reg.ctx()
    obj = DummyContext(888)
    res = ctx.insert_or_assign("Dummy", obj)
    assert res is obj
    # insert_or_assign with same type_name and another instance replaces
    obj2 = DummyContext(999)
    res2 = ctx.insert_or_assign("Dummy", obj2)
    assert res2 is obj2
    assert res2.value == 999

    # Try with another type
    other = AnotherContext("hello")
    res_other = ctx.insert_or_assign("Another", other)
    assert res_other is other
    assert res_other.tag == "hello"
    # Replace again
    other2 = AnotherContext("world")
    res_other2 = ctx.insert_or_assign("Another", other2)
    assert res_other2 is other2
    assert res_other2.tag == "world"

    assert ctx.contains(DummyContext) is False
