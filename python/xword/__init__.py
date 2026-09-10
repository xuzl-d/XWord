"""XWord 2.0: native C++17 WordprocessingML generation.

The native module and its DLL are built for the active Python interpreter.
All public names use snake_case; see docs/api.md for units and ownership.
"""
import os as _os
from pathlib import Path as _Path

_dll_directory = None
if _os.name == "nt" and hasattr(_os, "add_dll_directory"):
    _dll_directory = _os.add_dll_directory(str(_Path(__file__).parent))

from ._native import *
from ._native import __version__
