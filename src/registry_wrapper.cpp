#include "entt/entity/fwd.hpp"
#include "entt/entity/runtime_view.hpp"
#include <nanobind/make_iterator.h>
#include <nanobind/nanobind.h>
#include <nanobind/stl/chrono.h>
#include <nanobind/stl/function.h>
#include <nanobind/stl/pair.h>
#include <nanobind/stl/shared_ptr.h>
#include <nanobind/stl/string.h>
#include <nanobind/stl/string_view.h>
#include <nanobind/stl/unordered_map.h>
#include <nanobind/stl/vector.h>
#include <nanobind/trampoline.h>

#include <entt/entt.hpp>

// Mark entt::entity as opaque to nanobind to allow custom binding
NB_MAKE_OPAQUE(entt::entity);

namespace nb = nanobind;

using namespace entt::literals;

using ReactiveStorage = entt::reactive_mixin<entt::storage<void>>;

enum SignalEvent { OnConstruct, OnUpdate, OnDestroy };

static void on_construct(entt::registry &reg, nb::object &callback,
                         entt::entity entt) {
  // printf("on_construct: %u\n", (ENTT_ID_TYPE)entt);
}

class SignalObserver {

public:
  virtual void on_construct(entt::registry &reg, entt::entity entity) const {}
  virtual void on_update(entt::registry &reg, entt::entity entity) const {}
  virtual void on_destroy(entt::registry &reg, entt::entity entity) const {}
};

class PySignalObserver : public SignalObserver {
  NB_TRAMPOLINE(SignalObserver, 3);

public:
  void on_construct(entt::registry &reg, entt::entity entity) const override {
    // expand of NB_OVERRIDE & fix reg's policy to reference
    using nb_ret_type = decltype(NBBase ::on_construct(reg, entity));
    nanobind ::detail ::ticket nb_ticket(nb_trampoline, "on_construct", false);
    if (nb_ticket.key.is_valid()) {
      return nanobind ::cast<nb_ret_type>(nb_trampoline.base().attr(
          nb_ticket.key)(nb::cast(reg, nb::rv_policy::reference), entity));
    } else
      return NBBase ::on_construct(reg, entity);
  }
  void on_update(entt::registry &reg, entt::entity entity) const override {
    // expand of NB_OVERRIDE & fix reg's policy to reference
    using nb_ret_type = decltype(NBBase ::on_update(reg, entity));
    nanobind ::detail ::ticket nb_ticket(nb_trampoline, "on_update", false);
    if (nb_ticket.key.is_valid()) {
      return nanobind ::cast<nb_ret_type>(nb_trampoline.base().attr(
          nb_ticket.key)(nb::cast(reg, nb::rv_policy::reference), entity));
    } else
      return NBBase ::on_update(reg, entity);
  }
  void on_destroy(entt::registry &reg, entt::entity entity) const override {
    // expand of NB_OVERRIDE & fix reg's policy to reference
    using nb_ret_type = decltype(NBBase ::on_destroy(reg, entity));
    nanobind ::detail ::ticket nb_ticket(nb_trampoline, "on_destroy", false);
    if (nb_ticket.key.is_valid()) {
      return nanobind ::cast<nb_ret_type>(nb_trampoline.base().attr(
          nb_ticket.key)(nb::cast(reg, nb::rv_policy::reference), entity));
    } else
      return NBBase ::on_destroy(reg, entity);
  }
};

class PyComponent {
private:
  nb::object obj;

public:
  PyComponent() : obj(nb::none()) {}
  PyComponent(nb::object o) : obj(o) {}
  nb::object get() const { return obj; }
};

class PyContext {
private:
  nb::object obj;

public:
  PyContext() : obj(nb::none()) {}
  PyContext(nb::object o) : obj(o) {}
  nb::object get() const { return obj; }
};

template <typename Set> class ViewIterator final {
  using iterator_type = typename Set::iterator;
  using iterator_traits = std::iterator_traits<iterator_type>;

  [[nodiscard]] bool valid() const {
    return (!tombstone_check || *it != entt::tombstone) &&
           std::all_of(++pools->begin(), pools->end(),
                       [entt = *it](const auto *curr) {
                         return curr->contains(entt);
                       }) &&
           std::none_of(filter->cbegin(), filter->cend(),
                        [entt = *it](const auto *curr) {
                          return curr && curr->contains(entt);
                        });
  }

public:
  using value_type = typename nb::object;
  using pointer = typename nb::object *;
  using reference = typename nb::object &;
  using difference_type = typename iterator_traits::difference_type;
  using iterator_category = std::bidirectional_iterator_tag;

  constexpr ViewIterator() noexcept
      : pools{}, filter{}, it{}, tombstone_check{} {}

  // NOLINTNEXTLINE(bugprone-easily-swappable-parameters)
  ViewIterator(const std::vector<entt::storage<PyComponent> *> &storages,
               const std::vector<Set *> &cpools,
               const std::vector<Set *> &ignore, iterator_type curr) noexcept
      : storages(&storages), pools{&cpools}, filter{&ignore}, it{curr},
        tombstone_check{pools->size() == 1u &&
                        (*pools)[0u]->policy() ==
                            entt::deletion_policy::in_place} {
    if (it != (*pools)[0]->end() && !valid()) {
      ++(*this);
    }
    update_curr();
  }

  void update_curr() {
    if (it == (*pools)[0]->end()) {
      curobj = nb::none();
      return;
    }

    nb::list values;
    values.append(nb::cast(*it));
    for (const auto storage : *storages) {
      values.append(storage->get(*it).get());
    }
    curobj = nb::tuple(values);
  }

  ViewIterator &operator++() {
    ++it;
    for (const auto last = (*pools)[0]->end(); it != last && !valid(); ++it) {
    }
    update_curr();
    return *this;
  }

  ViewIterator operator++(int) {
    const ViewIterator orig = *this;
    return ++(*this), orig;
  }

  ViewIterator &operator--() {
    --it;
    for (const auto first = (*pools)[0]->begin(); it != first && !valid();
         --it) {
    }
    update_curr();
    return *this;
  }

  ViewIterator operator--(int) {
    const ViewIterator orig = *this;
    return operator--(), orig;
  }

  [[nodiscard]] pointer operator->() const noexcept { return &curobj; }

  [[nodiscard]] reference operator*() const noexcept { return *operator->(); }

  [[nodiscard]] constexpr bool
  operator==(const ViewIterator &other) const noexcept {
    return it == other.it;
  }

  [[nodiscard]] constexpr bool
  operator!=(const ViewIterator &other) const noexcept {
    return !(*this == other);
  }

private:
  const std::vector<entt::storage<PyComponent> *> *storages;
  const std::vector<Set *> *pools;
  const std::vector<Set *> *filter;
  iterator_type it;
  bool tombstone_check;
  mutable nb::object curobj;
};

class PyView : public entt::runtime_view {
  using container_type = std::vector<common_type *, allocator_type>;

public:
  PyView() : entt::runtime_view(), registry(nullptr) {}
  PyView(entt::registry *reg) : entt::runtime_view(), registry(reg) {}
  entt::registry *registry = nullptr;
  std::vector<entt::storage<PyComponent> *> storages;
  using iterator = ViewIterator<common_type>;
  container_type &get_pools() {
    char *base = reinterpret_cast<char *>(this);
    return *reinterpret_cast<container_type *>(base);
  }
  container_type &get_filter() {
    char *base = reinterpret_cast<char *>(this);
    return *reinterpret_cast<container_type *>(base + sizeof(container_type));
  }
  iterator begin() {
    container_type &pools = get_pools();
    container_type &filter = get_filter();
    return pools.empty() ? iterator{}
                         : iterator{storages, pools, filter, pools[0]->begin()};
  }
  iterator end() {
    container_type &pools = get_pools();
    container_type &filter = get_filter();
    return pools.empty() ? iterator{}
                         : iterator{storages, pools, filter, pools[0]->end()};
  }
};

inline auto get_type_name(nb::type_object &type) {
  return nb::cast<const char *>(type.attr("__name__"));
}

inline auto get_type_id(nb::type_object &type) {
  return entt::hashed_string(get_type_name(type));
}

auto &&get_type_storage(entt::registry *registry, nb::type_object &type) {
  auto type_id = get_type_id(type);
  return registry->storage<PyComponent>(type_id);
}

auto &&get_type_storage(entt::registry *registry, const char *type_name) {
  auto type_id = entt::hashed_string(type_name);
  return registry->storage<PyComponent>(type_id);
}

PyView *create_view(entt::registry *self, nb::type_object &type,
                    nb::args &args) {
  auto view = new PyView(self);
  auto &&storage = get_type_storage(self, type);
  view->iterate(storage);
  view->storages.push_back(&storage);
  for (auto arg : args) {
    nb::type_object type = nb::cast<nb::type_object>(arg);
    auto &&storage = get_type_storage(self, type);
    view->iterate(storage);
    view->storages.push_back(&storage);
  }
  return view;
};

NB_MODULE(_entt, m) {
  m.doc() = "Python wrapper for EnTT";
  // Bind entt::entity to Python and support math/compare ops
  nb::class_<entt::entity>(m, "Entity")
      .def(nb::init<const ENTT_ID_TYPE &>(),
           nb::sig("def __init__(self, id: int)"))
      .def("__int__", [](entt::entity e) { return static_cast<uint32_t>(e); })
      .def("__index__", [](entt::entity e) { return static_cast<uint32_t>(e); })
      .def("__hash__", [](entt::entity e) { return static_cast<uint32_t>(e); })
      .def(
          "__add__",
          [](entt::entity a, int b) {
            return entt::entity{static_cast<uint32_t>(a) + b};
          },
          nb::is_operator())
      .def(
          "__sub__",
          [](entt::entity a, int b) {
            return entt::entity{static_cast<uint32_t>(a) - b};
          },
          nb::is_operator())
      .def(
          "__eq__", [](entt::entity a, entt::entity b) { return a == b; },
          nb::is_operator())
      .def(
          "__ne__", [](entt::entity a, entt::entity b) { return a != b; },
          nb::is_operator())
      .def(
          "__lt__",
          [](entt::entity a, entt::entity b) {
            return static_cast<uint32_t>(a) < static_cast<uint32_t>(b);
          },
          nb::is_operator())
      .def(
          "__le__",
          [](entt::entity a, entt::entity b) {
            return static_cast<uint32_t>(a) <= static_cast<uint32_t>(b);
          },
          nb::is_operator())
      .def(
          "__gt__",
          [](entt::entity a, entt::entity b) {
            return static_cast<uint32_t>(a) > static_cast<uint32_t>(b);
          },
          nb::is_operator())
      .def(
          "__ge__",
          [](entt::entity a, entt::entity b) {
            return static_cast<uint32_t>(a) >= static_cast<uint32_t>(b);
          },
          nb::is_operator())
      .def("__repr__", [](entt::entity e) {
        return std::string("<Entity: ") +
               std::to_string(static_cast<uint32_t>(e)) + ">";
      });

  nb::enum_<SignalEvent>(m, "SignalEvent", nb::is_arithmetic())
      .value("OnConstruct", SignalEvent::OnConstruct)
      .value("OnUpdate", SignalEvent::OnUpdate)
      .value("OnDestroy", SignalEvent::OnDestroy);

  nb::class_<SignalObserver, PySignalObserver>(m, "SignalObserver")
      .def(nb::init<>())
      .def("on_construct", &SignalObserver::on_construct)
      .def("on_update", &SignalObserver::on_update)
      .def("on_destroy", &SignalObserver::on_destroy);

  nb::class_<PyView>(m, "View", nb::sig("class View[*T]"))
      .def(
          "__iter__",
          [](PyView &self) {
            return nb::make_iterator(nb::type<PyView>(), "iterator",
                                     self.begin(), self.end());
          },
          nb::sig("def __iter__(self) -> "
                  "collections.abc.Iterator[tuple[Entity, *T]]"),
          nb::keep_alive<0, 1>())
      .def("contains",
           [](PyView *self, entt::registry *registry, entt::entity entity) {
             return self->contains(static_cast<entt::entity>(entity));
           })
      .def("clear", &PyView::clear)
      .def(
          "iterate",
          [](PyView *self, nb::type_object &type) {
            auto &&storage = get_type_storage(self->registry, type);
            self->iterate(storage);
            return self;
          },
          nb::sig("def iterate(self, type: type) -> typing.Self"),
          nb::rv_policy::reference)
      .def(
          "exclude",
          [](PyView *self, nb::type_object &type) {
            auto &&storage = get_type_storage(self->registry, type);
            self->exclude(storage);
            return self;
          },
          nb::sig("def exclude(self, type: type) -> typing.Self"),
          nb::rv_policy::reference);

  nb::class_<entt::registry::context>(m, "Context")
      .def(
          "contains",
          [](entt::registry::context *self, nb::type_object &type) {
            auto type_id = entt::hashed_string(get_type_name(type));
            return self->contains<PyContext>(type_id);
          },
          nb::sig("def contains[T](self, type: type[T]) -> bool"))
      .def("contains",
           [](entt::registry::context *self, const char *type_name) {
             auto type_id = entt::hashed_string(type_name);
             return self->contains<PyContext>(type_id);
           })
      .def(
          "find",
          [](entt::registry::context *self, nb::type_object &type) {
            auto type_id = entt::hashed_string(get_type_name(type));
            auto cur = self->find<PyContext>(type_id);
            if (cur == nullptr) {
              return nb::none();
            }
            return cur->get();
          },
          nb::sig("def find[T](self, type: type[T]) -> T | None"),
          nb::rv_policy::reference)
      .def(
          "find",
          [](entt::registry::context *self, const char *type_name) {
            auto type_id = entt::hashed_string(type_name);
            auto cur = self->find<PyContext>(type_id);
            if (cur == nullptr) {
              return nb::none();
            }
            return cur->get();
          },
          nb::sig("def find(self, type_name: str) -> typing.Any | None"),
          nb::rv_policy::reference)
      .def(
          "emplace",
          [](entt::registry::context *self, nb::type_object &type,
             nb::args &args) {
            auto ctx = type(*args);
            auto ctx_id = get_type_id(type);
            auto cur = self->find<PyContext>(ctx_id);
            if (cur != nullptr) {
              return cur->get();
            }

            return self->emplace_as<PyContext>(ctx_id, ctx).get();
          },
          nb::sig("def emplace[T](self, type: type[T], *args) -> T"),
          nb::rv_policy::reference)
      .def(
          "emplace",
          [](entt::registry::context *self, const char *type_name,
             nb::object &ctx) {
            auto ctx_id = entt::hashed_string(type_name);
            auto cur = self->find<PyContext>(ctx_id);
            if (cur != nullptr) {
              return cur->get();
            }

            return self->emplace_as<PyContext>(ctx_id, ctx).get();
          },
          nb::sig("def emplace(self, type_name: str, ctx: typing.Any) -> "
                  "typing.Any"),
          nb::rv_policy::reference)
      .def(
          "insert_or_assign",
          [](entt::registry::context *self, nb::object &ctx) {
            auto type = nb::cast<nb::type_object>(ctx.type());
            auto ctx_id = get_type_id(type);
            return self->insert_or_assign<PyContext>(ctx_id, PyContext(ctx))
                .get();
          },
          nb::sig("def insert_or_assign[T](self, ctx: T) -> T"),
          nb::rv_policy::reference)
      .def(
          "insert_or_assign",
          [](entt::registry::context *self, const char *type_name,
             nb::object &ctx) {
            auto ctx_id = entt::hashed_string(type_name);
            return self->insert_or_assign<PyContext>(ctx_id, PyContext(ctx))
                .get();
          },
          nb::sig("def insert_or_assign(self, type_name: str, ctx: typing.Any) "
                  "-> typing.Any"),
          nb::rv_policy::reference)
      .def(
          "get",
          [](entt::registry::context *self, nb::type_object &type) {
            auto type_id = entt::hashed_string(get_type_name(type));
            auto cur = self->find<PyContext>(type_id);
            if (cur == nullptr) {
              nb::raise("KeyError: Context does not contain component[%s]",
                        get_type_name(type));
            }
            return cur->get();
          },
          nb::sig("def get[T](self, type: type[T]) -> T"),
          nb::rv_policy::reference)
      .def(
          "get",
          [](entt::registry::context *self, const char *type_name) {
            auto type_id = entt::hashed_string(type_name);
            auto cur = self->find<PyContext>(type_id);
            if (cur == nullptr) {
              nb::raise("KeyError: Context does not contain component[%s]",
                        type_name);
            }
            return cur->get();
          },
          nb::sig("def get(self, type_name: str) -> typing.Any"),
          nb::rv_policy::reference)
      .def(
          "erase",
          [](entt::registry::context *self, nb::type_object &type) {
            auto type_id = entt::hashed_string(get_type_name(type));
            return self->erase<PyContext>(type_id);
          },
          nb::sig("def erase[T](self, type: type[T]) -> bool"))
      .def("erase", [](entt::registry::context *self, const char *type_name) {
        auto type_id = entt::hashed_string(type_name);
        return self->erase<PyContext>(type_id);
      });

  nb::class_<ReactiveStorage>(m, "ReactiveStorage")
      .def(
          "__iter__",
          [](ReactiveStorage &self) {
            return nb::make_iterator(nb::type<ReactiveStorage>(), "iterator",
                                     self.begin(), self.end());
          },
          nb::sig("def __iter__(self) -> "
                  "collections.abc.Iterator[Entity]"),
          nb::keep_alive<0, 1>())
      .def("__contains__",
           [](ReactiveStorage *self, entt::entity entity) {
             return self->contains(static_cast<entt::entity>(entity));
           })
      .def("remove",
           [](ReactiveStorage *self, entt::entity entity) {
             return self->remove(static_cast<entt::entity>(entity));
           })
      .def("erase",
           [](ReactiveStorage *self, entt::entity entity) {
             self->erase(static_cast<entt::entity>(entity));
           })
      .def("size", &ReactiveStorage::size)
      .def("clear", &ReactiveStorage::clear)
      .def(
          "on_construct",
          [](ReactiveStorage *self, nb::type_object &type) {
            self->on_construct<PyComponent>(get_type_id(type));
            return self;
          },
          nb::sig("def on_construct[T](self, type: type[T]) -> typing.Self"),
          nb::rv_policy::reference)
      .def(
          "on_update",
          [](ReactiveStorage *self, nb::type_object &type) {
            self->on_update<PyComponent>(get_type_id(type));
            return self;
          },
          nb::sig("def on_update[T](self, type: type[T]) -> typing.Self"),
          nb::rv_policy::reference)
      .def(
          "on_destroy",
          [](ReactiveStorage *self, nb::type_object &type) {
            self->on_destroy<PyComponent>(get_type_id(type));
            return self;
          },
          nb::sig("def on_destroy[T](self, type: type[T]) -> typing.Self"),
          nb::rv_policy::reference);

  nb::class_<entt::registry>(m, "Registry")
      .def(nb::init<>())
      .def(
          "create", [](entt::registry *self) { return self->create(); },
          nb::rv_policy::reference)
      .def("destroy", [](entt::registry *self,
                         entt::entity entity) { self->destroy(entity); })
      .def("valid", [](entt::registry *self,
                       entt::entity entity) { return self->valid(entity); })
      .def("current", [](entt::registry *self,
                         entt::entity entity) { return self->current(entity); })
      .def(
          "emplace",
          [](entt::registry *self, entt::entity entt, nb::type_object &type,
             nb::args &args) {
            auto &&storage = get_type_storage(self, type);
            if (storage.contains(entt)) {
              nb::raise("KeyError: Entity already has component[%s] in storage",
                        get_type_name(type));
            }
            auto comp = type(*args);
            return storage.emplace(entt, PyComponent(comp)).get();
          },
          nb::sig(
              "def emplace[T](self, entt: Entity, type: type[T], *args) -> T"),
          nb::rv_policy::reference)
      .def(
          "emplace",
          [](entt::registry *self, entt::entity entt, nb::object &comp) {
            auto type = nb::cast<nb::type_object>(comp.type());
            auto &&storage = get_type_storage(self, type);
            if (storage.contains(entt)) {
              nb::raise("KeyError: Entity already has component[%s] in storage",
                        get_type_name(type));
            }
            return storage.emplace(entt, PyComponent(comp)).get();
          },
          nb::sig("def emplace[T](self, entt: Entity, comp: T) -> T"),
          nb::rv_policy::reference)
      .def(
          "emplace_or_replace",
          [](entt::registry *self, entt::entity entt, nb::type_object &type,
             nb::args &args) {
            auto &&storage = get_type_storage(self, type);
            auto comp = type(*args);
            if (storage.contains(entt)) {
              return storage
                  .patch(entt,
                         [&comp](auto &...curr) {
                           ((curr = PyComponent(comp)), ...);
                         })
                  .get();
            } else {
              return storage.emplace(entt, PyComponent(comp)).get();
            }
          },
          nb::sig("def emplace_or_replace[T](self, entt: Entity, type: "
                  "type[T], *args) -> T"),
          nb::rv_policy::reference)
      .def(
          "emplace_or_replace",
          [](entt::registry *self, entt::entity entt, nb::object &comp) {
            auto type = nb::cast<nb::type_object>(comp.type());
            auto &&storage = get_type_storage(self, type);
            if (storage.contains(entt)) {
              return storage
                  .patch(entt,
                         [&comp](auto &...curr) {
                           ((curr = PyComponent(comp)), ...);
                         })
                  .get();
            } else {
              return storage.emplace(entt, PyComponent(comp)).get();
            }
          },
          nb::sig(
              "def emplace_or_replace[T](self, entt: Entity, comp: T) -> T"),
          nb::rv_policy::reference)
      .def(
          "get",
          [](entt::registry *self, entt::entity entt, nb::type_object &type) {
            auto &&storage = get_type_storage(self, type);
            if (!storage.contains(entt)) {
              nb::raise(
                  "KeyError: Entity does not exist in component[%s] storage",
                  get_type_name(type));
            }
            return storage.get(entt).get();
          },
          nb::sig("def get[T](self, entt: Entity, type: type[T]) -> T"),
          nb::rv_policy::reference)
      .def(
          "get",
          [](entt::registry *self, entt::entity entt, const char *type_name) {
            auto &&storage = get_type_storage(self, type_name);
            if (!storage.contains(entt)) {
              nb::raise(
                  "KeyError: Entity does not exist in component[%s] storage",
                  type_name);
            }
            return storage.get(entt).get();
          },
          nb::sig("def get(self, entt: Entity, type_name: str) -> typing.Any"),
          nb::rv_policy::reference)
      .def(
          "try_get",
          [](entt::registry *self, entt::entity entt, nb::type_object &type) {
            auto &&storage = get_type_storage(self, type);
            if (storage.contains(entt)) {
              return storage.get(entt).get();
            } else {
              return nb::none();
            }
          },
          nb::sig(
              "def try_get[T](self, entt: Entity, type: type[T]) -> T | None"),
          nb::rv_policy::reference)
      .def(
          "try_get",
          [](entt::registry *self, entt::entity entt, const char *type_name) {
            auto &&storage = get_type_storage(self, type_name);
            if (storage.contains(entt)) {
              return storage.get(entt).get();
            } else {
              return nb::none();
            }
          },
          nb::sig("def try_get(self, entt: Entity, type_name: str) -> "
                  "typing.Any | None"),
          nb::rv_policy::reference)
      .def(
          "get_all",
          [](entt::registry *self, entt::entity entt) {
            nb::list values;
            for (auto &&curr : self->storage()) {
              auto &storage =
                  static_cast<entt::storage<PyComponent> &>(curr.second);
              if (storage.contains(entt)) {
                values.append(storage.get(entt).get());
              }
            }
            return nb::tuple(values);
          },
          nb::sig(
              "def get_all(self, entt: Entity) -> tuple[entt.Component, ...]"))
      .def(
          "patch",
          [](entt::registry *self, entt::entity entt, nb::type_object &type,
             nb::args &args) {
            auto &&storage = get_type_storage(self, type);
            auto comp = type(*args);
            if (!storage.contains(entt)) {
              nb::raise(
                  "KeyError: Entity does not exist in component[%s] storage",
                  get_type_name(type));
            }
            return storage
                .patch(entt,
                       [&comp](auto &...curr) {
                         ((curr = PyComponent(comp)), ...);
                       })
                .get();
          },
          nb::sig(
              "def patch[T](self, entt: Entity, type: type[T], *args) -> T"),
          nb::rv_policy::reference)
      .def(
          "remove",
          [](entt::registry *self, entt::entity entt, nb::type_object &type,
             nb::args &args) {
            auto &&storage = get_type_storage(self, type);
            int removed = storage.remove(entt) ? 1 : 0;
            for (auto arg : args) {
              nb::type_object type = nb::cast<nb::type_object>(arg);
              auto &&storage = get_type_storage(self, type);
              removed += storage.remove(entt) ? 1 : 0;
            }
            return removed;
          },
          nb::sig(
              "def remove[T](self, entt: Entity, type: type[T], *args) -> int"))
      .def(
          "remove",
          [](entt::registry *self, nb::iterator entities, nb::type_object &type,
             nb::args &args) {
            int removed = 0;
            for (auto entity : entities) {
              auto entt =
                  static_cast<entt::entity>(nb::cast<ENTT_ID_TYPE>(entity));
              auto &&storage = get_type_storage(self, type);
              removed = storage.remove(entt) ? 1 : 0;
              for (auto arg : args) {
                nb::type_object type = nb::cast<nb::type_object>(arg);
                auto &&storage = get_type_storage(self, type);
                removed += storage.remove(entt) ? 1 : 0;
              }
            }
            return removed;
          },
          nb::sig("def remove[T](self, entities: "
                  "collections.abc.Iterable[Entity], type: "
                  "type[T], *args) -> int"))
      .def("remove",
           [](entt::registry *self, entt::entity entt, const char *type_name,
              nb::args &args) {
             auto &&storage = get_type_storage(self, type_name);
             int removed = storage.remove(entt) ? 1 : 0;
             for (auto arg : args) {
               const char *type_name = nb::cast<const char *>(arg);
               auto &&storage = get_type_storage(self, type_name);
               removed += storage.remove(entt) ? 1 : 0;
             }
             return removed;
           })
      .def("remove",
           [](entt::registry *self, nb::iterator entities,
              const char *type_name, nb::args &args) {
             int removed = 0;
             for (auto entity : entities) {
               auto entt =
                   static_cast<entt::entity>(nb::cast<ENTT_ID_TYPE>(entity));
               auto &&storage = get_type_storage(self, type_name);
               removed = storage.remove(entt) ? 1 : 0;
               for (auto arg : args) {
                 const char *type_name = nb::cast<const char *>(arg);
                 auto &&storage = get_type_storage(self, type_name);
                 removed += storage.remove(entt) ? 1 : 0;
               }
             }
             return removed;
           })
      .def(
          "erase",
          [](entt::registry *self, entt::entity entt, nb::type_object &type,
             nb::args &args) {
            auto &&storage = get_type_storage(self, type);
            if (!storage.contains(entt)) {
              nb::raise(
                  "KeyError: Entity does not exist in component[%s] storage",
                  get_type_name(type));
            }
            storage.remove(entt);
            for (auto arg : args) {
              nb::type_object type = nb::cast<nb::type_object>(arg);
              auto &&storage = get_type_storage(self, type);
              if (!storage.contains(entt)) {
                nb::raise("KeyError: Entity does not exist in component[%s] "
                          "storage",
                          get_type_name(type));
              }
              storage.remove(entt);
            }
          },
          nb::sig(
              "def erase[T](self, entt: Entity, type: type[T], *args) -> None"))
      .def(
          "erase",
          [](entt::registry *self, nb::iterator entities, nb::type_object &type,
             nb::args &args) {
            for (auto entity : entities) {
              auto entt = nb::cast<entt::entity>(entity);
              auto &&storage = get_type_storage(self, type);
              if (!storage.contains(entt)) {
                nb::raise("KeyError: Entity does not exist in component[%s] "
                          "storage",
                          get_type_name(type));
              }
              storage.remove(entt);
              for (auto arg : args) {
                nb::type_object type = nb::cast<nb::type_object>(arg);
                auto &&storage = get_type_storage(self, type);
                if (!storage.contains(entt)) {
                  nb::raise("KeyError: Entity does not exist in component[%s] "
                            "storage",
                            get_type_name(type));
                }
                storage.remove(entt);
              }
            }
          },
          nb::sig("def erase[T](self, entities: "
                  "collections.abc.Iterable[Entity], type: "
                  "type[T], *args) -> None"))
      .def("erase",
           [](entt::registry *self, entt::entity entt, const char *type_name,
              nb::args &args) {
             auto &&storage = get_type_storage(self, type_name);
             if (!storage.contains(entt)) {
               nb::raise(
                   "KeyError: Entity does not exist in component[%s] storage",
                   type_name);
             }
             storage.remove(entt);
             for (auto arg : args) {
               auto type_name = nb::cast<const char *>(arg);
               auto &&storage = get_type_storage(self, type_name);
               if (!storage.contains(entt)) {
                 nb::raise("KeyError: Entity does not exist in component[%s] "
                           "storage",
                           type_name);
               }
               storage.remove(entt);
             }
           })
      .def("erase",
           [](entt::registry *self, nb::iterator entities,
              const char *type_name, nb::args &args) {
             for (auto entity : entities) {
               auto entt = nb::cast<entt::entity>(entity);
               auto &&storage = get_type_storage(self, type_name);
               if (!storage.contains(entt)) {
                 nb::raise("KeyError: Entity does not exist in component[%s] "
                           "storage",
                           type_name);
               }
               storage.remove(entt);
               for (auto arg : args) {
                 auto type_name = nb::cast<const char *>(arg);
                 auto &&storage = get_type_storage(self, type_name);
                 if (!storage.contains(entt)) {
                   nb::raise("KeyError: Entity does not exist in component[%s] "
                             "storage",
                             type_name);
                 }
                 storage.remove(entt);
               }
             }
           })
      .def("clear", [](entt::registry *self) { self->clear(); })
      .def(
          "view",
          [](entt::registry *self) {
            auto view = new PyView(self);
            return view;
          },
          nb::sig("def view(self) -> View"))
      .def("view", &create_view,
           nb::sig("def view[T](self, type: type[T]) -> View[T]"))
      .def("view", &create_view,
           nb::sig("def view[T1, T2](self, t1: type[T1], t2: type[T2]) -> "
                   "View[T1, T2]"))
      .def("view", &create_view,
           nb::sig("def view[T1, T2, T3](self, t1: type[T1], t2: type[T2], "
                   "t3: type[T3]) -> View[T1, T2, T3]"))
      .def("view", &create_view,
           nb::sig("def view[T1, T2, T3, T4](self, t1: type[T1], t2: type[T2], "
                   "t3: type[T3], t4: type[T4]) -> View[T1, T2, T3, T4]"))
      .def("view", &create_view,
           nb::sig(
               "def view[T1, T2, T3, T4, T5](self, t1: type[T1], t2: type[T2], "
               "t3: type[T3], t4: type[T4], t5: type[T5]) -> "
               "View[T1, T2, T3, T4, T5]"))
      .def("view",
           [](entt::registry *self, const char *type_name, nb::args &args) {
             auto view = new PyView(self);
             auto &&storage = get_type_storage(self, type_name);
             view->iterate(storage);
             view->storages.push_back(&storage);
             for (auto arg : args) {
               auto type_name = nb::cast<const char *>(arg);
               auto &&storage = get_type_storage(self, type_name);
               view->iterate(storage);
               view->storages.push_back(&storage);
             }
             return view;
           })
      .def(
          "ctx", [](entt::registry *self) { return &self->ctx(); },
          nb::rv_policy::reference)
      .def(
          "reactive",
          [](entt::registry *self) {
            ReactiveStorage *rs = new ReactiveStorage();
            rs->bind(*self);
            return rs;
          },
          nb::rv_policy::reference

          )
      .def("connect",
           [](entt::registry *self, nb::type_object &type,
              const SignalEvent &event, SignalObserver &observer) {
             auto &&storage = get_type_storage(self, type);
             switch (event) {
             case SignalEvent::OnConstruct: {
               storage.on_construct().connect<&SignalObserver::on_construct>(
                   observer);
               break;
             }
             case SignalEvent::OnUpdate: {
               storage.on_update().connect<&SignalObserver::on_update>(
                   observer);
               break;
             }
             case SignalEvent::OnDestroy: {
               storage.on_destroy().connect<&SignalObserver::on_destroy>(
                   observer);
               break;
             }
             }
           })
      .def("connect",
           [](entt::registry *self, const char *type_name,
              const SignalEvent &event, SignalObserver &observer) {
             auto &&storage = get_type_storage(self, type_name);
             switch (event) {
             case SignalEvent::OnConstruct: {
               storage.on_construct().connect<&SignalObserver::on_construct>(
                   observer);
               break;
             }
             case SignalEvent::OnUpdate: {
               storage.on_update().connect<&SignalObserver::on_update>(
                   observer);
               break;
             }
             case SignalEvent::OnDestroy: {
               storage.on_destroy().connect<&SignalObserver::on_destroy>(
                   observer);
               break;
             }
             }
           })
      .def("disconnect",
           [](entt::registry *self, nb::type_object &type,
              const SignalEvent &event, SignalObserver &observer) {
             auto &&storage = get_type_storage(self, type);
             switch (event) {
             case SignalEvent::OnConstruct: {
               storage.on_construct().disconnect<&SignalObserver::on_construct>(
                   observer);
               break;
             }
             case SignalEvent::OnUpdate: {
               storage.on_update().disconnect<&SignalObserver::on_update>(
                   observer);
               break;
             }
             case SignalEvent::OnDestroy: {
               storage.on_destroy().disconnect<&SignalObserver::on_destroy>(
                   observer);
               break;
             }
             }
           })
      .def("disconnect", [](entt::registry *self, const char *type_name,
                            const SignalEvent &event,
                            SignalObserver &observer) {
        auto &&storage = get_type_storage(self, type_name);
        switch (event) {
        case SignalEvent::OnConstruct: {
          storage.on_construct().disconnect<&SignalObserver::on_construct>(
              observer);
          break;
        }
        case SignalEvent::OnUpdate: {
          storage.on_update().disconnect<&SignalObserver::on_update>(observer);
          break;
        }
        case SignalEvent::OnDestroy: {
          storage.on_destroy().disconnect<&SignalObserver::on_destroy>(
              observer);
          break;
        }
        }
      });
  ;
}
