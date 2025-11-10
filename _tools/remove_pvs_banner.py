import os
from typing import List


HEADER_LINES = {
    "/* This is a student project. Dear PVS-Studio, please check it. */",
    "/* PVS-Studio Static Code Analyzer for C, C++: https://pvs-studio.com */",
    "// This is a personal academic project. Dear PVS-Studio, please check it.",
    "// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com",
    "/* This is a personal project. Dear PVS-Studio, please check it. */",
}


def strip_banner_from_lines(lines: List[str]) -> List[str]:
    """
    Remove known PVS-Studio banner lines if they appear at the very top of the file.
    Also remove a single following blank line if present (common after banners).
    """
    idx = 0
    n = len(lines)
    removed_any = False

    # Remove any number of known header lines at the very top
    while idx < n and lines[idx].strip() in HEADER_LINES:
        idx += 1
        removed_any = True

    # If we removed at least one header line, also drop leading blank lines right after
    if removed_any:
        while idx < n and lines[idx].strip() == "":
            idx += 1

    return lines[idx:] if idx > 0 else lines


def process_file(path: str) -> bool:
    try:
        with open(path, "r", encoding="utf-8", errors="replace") as f:
            original_lines = f.readlines()
        new_lines = strip_banner_from_lines(original_lines)
        if new_lines != original_lines:
            with open(path, "w", encoding="utf-8") as f:
                f.writelines(new_lines)
            return True
        return False
    except Exception:
        return False


def main() -> None:
    updated = 0
    total = 0
    for dirpath, _dirnames, filenames in os.walk("."):
        for name in filenames:
            if not name.endswith(".c"):
                continue
            total += 1
            path = os.path.join(dirpath, name)
            if process_file(path):
                updated += 1
                print(f"- cleaned: {path}")
    print(f"\nC files scanned: {total}")
    print(f"Files modified: {updated}")


if __name__ == "__main__":
    main()


