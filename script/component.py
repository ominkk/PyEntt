from dataclasses import Field, asdict
from typing import Any, ClassVar


class Component:
    __dataclass_fields__: ClassVar[dict[str, Field[Any]]]

    @staticmethod
    def is_send() -> bool:
        return True

    @staticmethod
    def is_persist() -> bool:
        return True

    def save(self) -> dict:
        return asdict(self)

    def load(self, data: dict) -> None:
        for key, value in data.items():
            if hasattr(self, key):
                setattr(self, key, value)
