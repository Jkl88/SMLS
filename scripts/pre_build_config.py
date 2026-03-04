"""
Pre-build: собирает все -D дефайны из build_flags и записывает в include/config.h.
Нужно для линтера и совместимости с Arduino IDE — один файл конфигурации вместо флагов.
"""
from pathlib import Path
import re
from typing import Any, List, cast

Import("env")
env = cast(Any, env)


def _collect_defines(env: Any) -> List[str]:
    """Собирает из build_flags и CPPDEFINES все макросы в виде строк 'NAME' или 'NAME=value'."""
    result: List[str] = []
    seen: set = set()
    # BUILD_FLAGS (из platformio.ini build_flags)
    raw = env.get("BUILD_FLAGS", [])
    if isinstance(raw, str):
        raw = [raw]
    for item in raw:
        item = env.subst(item).strip()
        for part in re.split(r"\s+", item):
            if part.startswith("-D"):
                define = part[2:].strip()
                if define and define not in seen:
                    seen.add(define)
                    result.append(define)
    # CPPDEFINES (доп. дефайны из env)
    for cppdef in env.get("CPPDEFINES", []):
        if isinstance(cppdef, (list, tuple)):
            name, val = cppdef[0], cppdef[1] if len(cppdef) > 1 else 1
            define = f"{name}={val}" if val != 1 else str(name)
        else:
            define = str(cppdef)
        if define not in seen:
            seen.add(define)
            result.append(define)
    return result


def _emit_config_h(defines: List[str], out_path: Path) -> None:
    """Пишет include/config.h с охранными макросами и списком #define."""
    out_path.parent.mkdir(parents=True, exist_ok=True)
    guard = "SMLS_CONFIG_H"
    lines = [
        f"#ifndef {guard}",
        f"#define {guard}",
        "",
        "/* Автоматически сгенерировано pre_build_config.py из build_flags */",
        "",
    ]
    for d in defines:
        if "=" in d:
            name, value = d.split("=", 1)
            lines.append(f"#define {name.strip()} {value.strip()}")
        else:
            lines.append(f"#define {d} 1")
    lines.extend(["", f"#endif /* {guard} */", ""])
    out_path.write_text("\n".join(lines), encoding="utf-8")


def pre_build_config(source, target, env):
    project_dir = Path(env.get("PROJECT_DIR"))
    include_dir = project_dir / "include"
    config_h = include_dir / "config.h"
    defines = _collect_defines(env)
    _emit_config_h(defines, config_h)
    print(f"[pre-build] config.h: {len(defines)} define(s) -> {config_h}")


env.AddPreAction("buildprog", pre_build_config)
