import pytest

from entt import Registry, SignalEvent


class DummyComponent:
    def __init__(self, val):
        self.val = val


@pytest.mark.parametrize(
    "event", [SignalEvent.OnConstruct, SignalEvent.OnUpdate, SignalEvent.OnDestroy]
)
def test_reactive_storage_events(event):
    reg = Registry()
    comp_type = DummyComponent

    reactive = reg.reactive()
    # bind signals
    reactive.on_construct(comp_type)
    reactive.on_update(comp_type)
    reactive.on_destroy(comp_type)

    ent = reg.create()
    if event == SignalEvent.OnConstruct:
        reg.emplace(ent, DummyComponent, 42)
        assert ent in reactive
    elif event == SignalEvent.OnUpdate:
        reg.emplace(ent, DummyComponent, 0)
        reg.emplace_or_replace(ent, DummyComponent, 1)
        assert ent in reactive
    else:  # OnDestroy
        reg.emplace(ent, DummyComponent, 7)
        reg.destroy(ent)
        assert ent in reactive


def test_registry_reactive_returns_reactive_storage():
    reg = Registry()
    reactive = reg.reactive()
    assert hasattr(reactive, "on_construct")
    assert hasattr(reactive, "on_update")
    assert hasattr(reactive, "on_destroy")


def test_reactive_signal_multiple_components():
    reg = Registry()
    comp1 = DummyComponent
    comp2 = str  # just as another dummy type
    reactive = reg.reactive()
    reactive.on_construct(comp1).on_construct(comp2)
    e1 = reg.create()
    e2 = reg.create()
    reg.emplace(e1, DummyComponent, 123)
    assert e1 in reactive and e2 not in reactive
    reg.emplace(e2, str, "abc")
    assert e1 in reactive and e2 in reactive
    reactive.clear()
    assert len(list(reactive)) == 0
