"""
Post-build: упаковка релиза и генерация скрипта прошивки.
Работает для любого таргета (env): ESP32, ESP32-S3, ESP32-C3 и т.д.
- Определяет чип по имени env и подставляет CHIP и BOOTLOADER_OFFSET в скрипт прошивки.
- Ищет bootloader.bin/partitions.bin в корне сборки или в bootloader/partition_table/.
"""
from pathlib import Path
import shutil
import subprocess
import zipfile
from typing import Any, List, Optional, cast

try:
    Import  # type: ignore[name-defined]
except NameError:
    def Import(*_args: Any, **_kwargs: Any) -> None:  # type: ignore[no-redef]
        return None

Import("env")
env = cast(Any, globals().get("env"))


def _copy_if_exists(src: Path, dst_dir: Path, copied: list[str], missing: list[str]) -> None:
    if src.exists():
        shutil.copy2(src, dst_dir / src.name)
        copied.append(src.name)
    else:
        missing.append(src.name)


def _find_build_artifact(build_dir: Path, name: str, alt_paths: List[Path]) -> Optional[Path]:
    """Ищет артефакт в build_dir (корень или типичные подпапки PlatformIO/ESP-IDF)."""
    if (build_dir / name).exists():
        return build_dir / name
    for p in alt_paths:
        if p.exists():
            return p
    return None


def _chip_for_env(pioenv: str) -> str:
    lower = pioenv.lower()
    if "esp32-s3" in lower or ("s3" in lower and "c3" not in lower):
        return "esp32s3"
    if "esp32-c3" in lower or "c3" in lower:
        return "esp32c3"
    if "esp32-c6" in lower or "c6" in lower:
        return "esp32c6"
    if "esp32-h2" in lower or "h2" in lower:
        return "esp32h2"
    return "esp32"


def _build_target_flash_script(project_dir: Path, package_dir: Path, pioenv: str) -> Optional[Path]:
    template = project_dir / "scripts" / "flash_auto.bat"
    if not template.exists():
        return None

    chip = _chip_for_env(pioenv)
    script_name = f"flash_{pioenv}_auto.bat"
    out_path = package_dir / script_name

    content = template.read_text(encoding="utf-8")
    content = content.replace('set "CHIP=esp32"', f'set "CHIP={chip}"')
    # ESP32-S3, ESP32-C3, ESP32-C6, ESP32-H2 use bootloader at 0x0
    # Classic ESP32 uses 0x1000
    if chip in ("esp32s3", "esp32c3", "esp32c6", "esp32h2"):
        content = content.replace(
            'set "BOOTLOADER_OFFSET=0x1000"', 'set "BOOTLOADER_OFFSET=0x0"'
        )
    content = content.replace(
        "echo [INFO] PRM-X2 OLED auto flashing script (esp32dev)",
        f"echo [INFO] PRM-X2 auto flashing script ({pioenv})",
    )
    content = content.replace(
        "echo [ERROR] Run: pio run -e esp32dev -t buildfs",
        f"echo [ERROR] Run: pio run -e {pioenv} -t buildfs",
    )
    out_path.write_text(content, encoding="utf-8")
    return out_path


def _ensure_littlefs_image(project_dir: Path, build_dir: Path, pioenv: str) -> bool:
    littlefs_bin = build_dir / "littlefs.bin"
    if littlefs_bin.exists():
        return True

    print(f"[post-build] littlefs.bin is missing for {pioenv}, running buildfs...")
    python_exe = env.subst("$PYTHONEXE")
    cmd = [python_exe, "-m", "platformio", "run", "-e", pioenv, "-t", "buildfs"]
    proc = subprocess.run(cmd, cwd=project_dir, check=False)
    if proc.returncode != 0:
        print(f"[post-build] buildfs failed for {pioenv} (code={proc.returncode})")
        return False
    return littlefs_bin.exists()


def package_release(source, target, env):  # noqa: ARG001
    pioenv = env.get("PIOENV", "")
    if not pioenv:
        return

    project_dir = Path(env.get("PROJECT_DIR"))
    build_dir = project_dir / ".pio" / "build" / pioenv
    package_dir = project_dir / "build" / f"release_{pioenv}"
    archive_path = project_dir / f"{pioenv}.zip"

    if package_dir.exists():
        shutil.rmtree(package_dir)
    package_dir.mkdir(parents=True, exist_ok=True)

    copied: list[str] = []
    missing: list[str] = []

    _ensure_littlefs_image(project_dir, build_dir, pioenv)

    project_pdf = project_dir / "PROJECT_GUIDE_ESP32DEV_RU.pdf"
    project_pdf_v2 = project_dir / "PROJECT_GUIDE_ESP32DEV_RU_v2.pdf"
    if project_pdf_v2.exists():
        project_pdf = project_pdf_v2

    # Артефакты сборки: поддерживаем корень и подпапки (bootloader/, partition_table/)
    bootloader_src = _find_build_artifact(
        build_dir,
        "bootloader.bin",
        [build_dir / "bootloader" / "bootloader.bin"],
    )
    partitions_src = _find_build_artifact(
        build_dir,
        "partitions.bin",
        [
            build_dir / "partition_table" / "partition-table.bin",
            build_dir / "partition_table" / "partitions.bin",
        ],
    )
    firmware_src = build_dir / "firmware.bin"
    if not firmware_src.exists():
        firmware_src = build_dir / "prm_x2.bin"
    littlefs_src = build_dir / "littlefs.bin"

    files = [
        #project_dir / "FLASH_SCRIPT_GUIDE_ESP32DEV_RU.pdf",
        #project_pdf,
        project_dir / "partitions.csv",
        project_dir / "scripts" / "esptool.exe",
    ]
    if bootloader_src:
        shutil.copy2(bootloader_src, package_dir / "bootloader.bin")
        copied.append("bootloader.bin")
    else:
        missing.append("bootloader.bin")
    if partitions_src:
        shutil.copy2(partitions_src, package_dir / "partitions.bin")
        copied.append("partitions.bin")
    else:
        missing.append("partitions.bin")
    if firmware_src.exists():
        shutil.copy2(firmware_src, package_dir / firmware_src.name)
        copied.append(firmware_src.name)
    else:
        missing.append("firmware.bin")
    if littlefs_src.exists():
        shutil.copy2(littlefs_src, package_dir / "littlefs.bin")
        copied.append("littlefs.bin")
    else:
        missing.append("littlefs.bin")

    for f in files:
        _copy_if_exists(f, package_dir, copied, missing)

    target_flash_script = _build_target_flash_script(project_dir, package_dir, pioenv)
    if target_flash_script:
        copied.append(target_flash_script.name)
    else:
        missing.append("flash_auto.bat(template)")

    flash_script_name = target_flash_script.name if target_flash_script else "flash_auto.bat"
    readme = package_dir / "README_PACKAGE.txt"
    readme.write_text(
        "\n".join(
            [
                f"Package target: {pioenv}",
                "Auto-generated after build.",
                "",
                "Included files:",
                *[f"- {name}" for name in copied],
                "",
                "Missing files (if any):",
                *([f"- {name}" for name in missing] if missing else ["- none"]),
                "",
                "Flashing script:",
                f"- {flash_script_name}",
                "",
                "Run from project root:",
                rf"cmd /c build\release_{pioenv}\{flash_script_name}",
            ]
        ),
        encoding="utf-8",
    )

    if archive_path.exists():
        archive_path.unlink()

    with zipfile.ZipFile(archive_path, "w", zipfile.ZIP_DEFLATED) as zf:
        for item in package_dir.iterdir():
            zf.write(item, item.name)

    print(f"[post-build] release package created: {archive_path}")


env.AddPostAction("$BUILD_DIR/${PROGNAME}.bin", package_release)
