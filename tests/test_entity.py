import pytest

from entt import Entity, Registry


class DummyComponent:
    def __init__(self, value):
        self.value = value


class AnotherComponent:
    def __init__(self, tag):
        self.tag = tag


@pytest.fixture
def reg():
    return Registry()


def test_create_and_valid(reg: Registry):
    e = reg.create()
    assert reg.valid(e)
    reg.destroy(e)
    assert not reg.valid(e)


def test_valid_with_int(reg: Registry):
    e = Entity(5)
    assert not reg.valid(e)
