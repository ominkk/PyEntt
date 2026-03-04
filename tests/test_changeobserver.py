import pytest

from entt import Registry, SignalEvent, SignalObserver


class DummyComponent:
    def __init__(self, value):
        self.value = value


class TrackerObserver(SignalObserver):
    """A test observer that tracks method calls"""

    def __init__(self):
        super().__init__()
        self.construct_calls = []
        self.update_calls = []
        self.destroy_calls = []

    def on_construct(self, reg, entity):
        print("on_construct called for entity", reg, entity)
        self.construct_calls.append(entity)

    def on_update(self, reg, entity):
        self.update_calls.append(entity)

    def on_destroy(self, reg, entity):
        self.destroy_calls.append(entity)


@pytest.fixture
def reg():
    return Registry()


@pytest.fixture
def observer():
    return TrackerObserver()


def test_observer_init():
    """Test that observer can be instantiated"""
    observer = SignalObserver()
    assert isinstance(observer, SignalObserver)


def test_observer_connect_construct(reg: Registry, observer: TrackerObserver):
    """Test that connecting observer to OnConstruct event works"""
    # Connect observer to DummyComponent's construct event
    reg.connect(DummyComponent, SignalEvent.OnConstruct, observer)

    # Create entity and add component
    entity = reg.create()
    reg.emplace(entity, DummyComponent, 42)

    # Verify observer was notified
    assert entity in observer.construct_calls
    assert len(observer.construct_calls) == 1
    assert len(observer.update_calls) == 0
    assert len(observer.destroy_calls) == 0


def test_observer_connect_update(reg: Registry, observer: TrackerObserver):
    """Test that connecting observer to OnUpdate event works"""
    # Connect observer to DummyComponent's update event
    reg.connect(DummyComponent, SignalEvent.OnUpdate, observer)

    # Create entity and add component
    entity = reg.create()
    reg.emplace(entity, DummyComponent, 42)

    # Update component
    reg.patch(entity, DummyComponent, 84)

    # Verify observer was notified for update but not construct
    assert entity in observer.update_calls
    assert len(observer.construct_calls) == 0
    assert len(observer.update_calls) == 1
    assert len(observer.destroy_calls) == 0


def test_observer_connect_destroy(reg: Registry, observer: TrackerObserver):
    """Test that connecting observer to OnDestroy event works"""
    # Connect observer to DummyComponent's destroy event
    reg.connect(DummyComponent, SignalEvent.OnDestroy, observer)

    # Create entity and add component
    entity = reg.create()
    reg.emplace(entity, DummyComponent, 42)

    # Remove component
    reg.remove(entity, DummyComponent)

    # Verify observer was notified
    assert entity in observer.destroy_calls
    assert len(observer.construct_calls) == 0
    assert len(observer.update_calls) == 0
    assert len(observer.destroy_calls) == 1


def test_observer_disconnect(reg: Registry, observer: TrackerObserver):
    """Test that disconnecting observer prevents future notifications"""
    # Connect observer
    reg.connect(DummyComponent, SignalEvent.OnConstruct, observer)

    # Create first entity
    entity1 = reg.create()
    reg.emplace(entity1, DummyComponent, 42)

    # Verify observer was notified
    assert entity1 in observer.construct_calls

    # Disconnect observer
    reg.disconnect(DummyComponent, SignalEvent.OnConstruct, observer)

    # Create second entity
    entity2 = reg.create()
    reg.emplace(entity2, DummyComponent, 84)

    # Verify observer was not notified for second entity
    assert entity2 not in observer.construct_calls
    assert len(observer.construct_calls) == 1


def test_multiple_observers(reg: Registry):
    """Test that multiple observers can be connected to the same event"""
    observer1 = TrackerObserver()
    observer2 = TrackerObserver()

    # Connect both observers
    reg.connect(DummyComponent, SignalEvent.OnConstruct, observer1)
    reg.connect(DummyComponent, SignalEvent.OnConstruct, observer2)

    # Create entity
    entity = reg.create()
    reg.emplace(entity, DummyComponent, 42)

    # Verify both observers were notified
    assert entity in observer1.construct_calls
    assert entity in observer2.construct_calls


def test_observer_multiple_events(reg: Registry, observer: TrackerObserver):
    """Test that observer can be connected to multiple events"""
    # Connect observer to all events
    reg.connect(DummyComponent, SignalEvent.OnConstruct, observer)
    reg.connect(DummyComponent, SignalEvent.OnUpdate, observer)
    reg.connect(DummyComponent, SignalEvent.OnDestroy, observer)

    # Create entity and component
    entity = reg.create()
    reg.emplace(entity, DummyComponent, 42)

    # Update component
    reg.patch(entity, DummyComponent, 84)

    # Remove component
    reg.remove(entity, DummyComponent)

    # Verify observer was notified for all events
    assert entity in observer.construct_calls
    assert entity in observer.update_calls
    assert entity in observer.destroy_calls
    assert len(observer.construct_calls) == 1
    assert len(observer.update_calls) == 1
    assert len(observer.destroy_calls) == 1


def test_observer_with_emplace_or_replace(reg: Registry, observer: TrackerObserver):
    """Test observer behavior with emplace_or_replace"""
    # Connect observer to all events
    reg.connect(DummyComponent, SignalEvent.OnConstruct, observer)
    reg.connect(DummyComponent, SignalEvent.OnUpdate, observer)

    # Create entity
    entity = reg.create()

    # First time should trigger construct
    reg.emplace_or_replace(entity, DummyComponent, 42)
    assert entity in observer.construct_calls
    assert len(observer.construct_calls) == 1
    assert len(observer.update_calls) == 0

    # Second time should trigger update
    reg.emplace_or_replace(entity, DummyComponent, 84)
    assert entity in observer.update_calls
    assert len(observer.construct_calls) == 1
    assert len(observer.update_calls) == 1
