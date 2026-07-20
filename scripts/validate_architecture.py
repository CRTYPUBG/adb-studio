#!/usr/bin/env python3
import json
import pathlib
import re
import sys


def main() -> int:
    root = pathlib.Path(sys.argv[1] if len(sys.argv) > 1 else ".").resolve()
    policy_path = root / "docs" / "architecture" / "module-dependencies.json"
    try:
        policy = json.loads(policy_path.read_text(encoding="utf-8"))
    except (OSError, json.JSONDecodeError) as error:
        print(f"architecture validation: {error}", file=sys.stderr)
        return 1

    modules = policy.get("modules", {})
    allowed = policy.get("allowedLayerDependencies", {})
    graph = {module: [] for module in modules}
    errors = []
    cmake_text = "\n".join(
        path.read_text(encoding="utf-8") for path in root.rglob("CMakeLists.txt")
        if "build" not in path.parts and "scrcpy-master" not in path.parts
    )
    declared_layers = dict(re.findall(
        r'adb_studio_declare_layer\s*\(\s*([^\s\)]+)\s+"([^"]+)"\s*\)', cmake_text
    ))
    for module, layer in modules.items():
        if declared_layers.get(module) != layer:
            errors.append(
                f"CMake layer drift for {module}: expected {layer}, "
                f"found {declared_layers.get(module, 'missing')}"
            )

    expected_edges = {tuple(edge) for edge in policy.get("dependencies", [])}
    actual_edges = set()
    for module in modules:
        pattern = re.compile(
            rf"target_link_libraries\s*\(\s*{re.escape(module)}\b(.*?)\)", re.DOTALL
        )
        for match in pattern.finditer(cmake_text):
            for candidate in modules:
                if candidate != module and re.search(rf"\b{re.escape(candidate)}\b", match.group(1)):
                    actual_edges.add((module, candidate))
    for edge in sorted(expected_edges - actual_edges):
        errors.append(f"declared dependency missing from CMake: {edge[0]} -> {edge[1]}")
    for edge in sorted(actual_edges - expected_edges):
        errors.append(f"CMake dependency missing from policy: {edge[0]} -> {edge[1]}")

    for source, target in policy.get("dependencies", []):
        if source not in modules or target not in modules:
            errors.append(f"unknown dependency endpoint: {source} -> {target}")
            continue
        graph[source].append(target)
        if modules[target] not in allowed.get(modules[source], []):
            errors.append(
                f"forbidden layer dependency: {source} ({modules[source]}) -> "
                f"{target} ({modules[target]})"
            )

    visiting = set()
    visited = set()

    def visit(module: str, path: list[str]) -> None:
        if module in visiting:
            start = path.index(module)
            errors.append("circular dependency: " + " -> ".join(path[start:] + [module]))
            return
        if module in visited:
            return
        visiting.add(module)
        for dependency in graph[module]:
            visit(dependency, path + [dependency])
        visiting.remove(module)
        visited.add(module)

    for module in graph:
        visit(module, [module])
    if errors:
        print("\n".join(f"architecture validation: {error}" for error in errors), file=sys.stderr)
        return 1
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
