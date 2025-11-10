import os
from typing import List, Tuple


BANNER = (
    "/* This is a student project. Dear PVS-Studio, please check it. */\n"
    "/* PVS-Studio Static Code Analyzer for C, C++: https://pvs-studio.com */\n"
    "\n"
)


def should_skip(file_path: str) -> bool:
    try:
        with open(file_path, "rb") as fh:
            data = fh.read(4096)
        return b"Dear PVS-Studio" in data
    except Exception:
        return True


def add_banner_to_file(file_path: str) -> bool:
    try:
        with open(file_path, "rb") as fh:
            original = fh.read()
        if b"Dear PVS-Studio" in original:
            return False
        with open(file_path, "wb") as fh:
            fh.write(BANNER.encode("utf-8"))
            fh.write(original)
        return True
    except Exception:
        return False


def find_c_files(root: str) -> List[str]:
    results: List[str] = []
    for dirpath, _dirnames, filenames in os.walk(root):
        for name in filenames:
            if name.endswith(".c"):
                results.append(os.path.join(dirpath, name))
    return results


def main() -> None:
    all_c_files = find_c_files(".")
    updated: List[str] = []
    skipped: List[Tuple[str, str]] = []

    for path in all_c_files:
        if should_skip(path):
            continue
        if add_banner_to_file(path):
            updated.append(path)
        else:
            skipped.append((path, "failed to update"))

    print(f"Total .c files: {len(all_c_files)}")
    print(f"Updated: {len(updated)}")
    if updated:
        for p in updated:
            print(f"  + {p}")
    if skipped:
        print(f"Skipped (errors): {len(skipped)}")
        for p, reason in skipped:
            print(f"  - {p}: {reason}")


if __name__ == "__main__":
    main()


