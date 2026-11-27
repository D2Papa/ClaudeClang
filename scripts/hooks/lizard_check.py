#!/usr/bin/env python3
"""Claude Code PostToolUse Hook: src/ 아래 소스 파일이 수정되면 Lizard로 즉시 검사한다.

기준(CLAUDE.md 2.3): 순환복잡도(CCN) 10 이하, 함수 라인 수 80라인 이하.
- 기준을 초과하면 종료 코드 2로 끝내고 stderr에 내용을 출력한다. Claude Code가 이를 Claude에게 전달해 수정하게 한다.
- 검사 도구(claudeclangbuild 이미지 또는 로컬 lizard)를 찾지 못하면 종료 코드 1로 끝나 경고만 표시된다.
"""
import json
import os
import pathlib
import shutil
import subprocess
import sys

CCN_LIMIT = 10
LENGTH_LIMIT = 80
IMAGE = "claudeclangbuild"
EXTS = {".c", ".h", ".cpp", ".hpp"}


def target_file(event: dict, root: pathlib.Path):
    """검사 대상 파일 경로(프로젝트 기준 상대 경로)를 돌려준다. 대상이 아니면 None."""
    raw = (event.get("tool_input") or {}).get("file_path")
    result = None
    if raw:
        path = pathlib.Path(raw)
        if not path.is_absolute():
            path = root / path
        try:
            rel = path.resolve().relative_to(root.resolve())
        except ValueError:
            rel = None
        if rel is not None and rel.parts[:1] == ("src",) and rel.suffix in EXTS and path.exists():
            result = rel.as_posix()
    return result


def image_exists() -> bool:
    docker = shutil.which("docker")
    ok = False
    if docker:
        probe = subprocess.run([docker, "image", "inspect", IMAGE], capture_output=True)
        ok = probe.returncode == 0
    return ok


def build_command(rel: str, root: pathlib.Path):
    """lizard 실행 명령을 만든다. 컨테이너를 우선 쓰고, 없으면 로컬 lizard를 쓴다."""
    args = ["lizard", rel, "-C", str(CCN_LIMIT), "-L", str(LENGTH_LIMIT), "-w"]
    command = None
    if image_exists():
        command = ["docker", "run", "--rm", "-v", "%s:/work" % root, "-w", "/work", IMAGE] + args
    elif shutil.which("lizard"):
        command = args
    return command


def main() -> int:
    # Windows 콘솔 기본 인코딩(cp949)으로 한글이 깨지지 않도록 UTF-8로 고정한다.
    sys.stderr.reconfigure(encoding="utf-8")
    try:
        event = json.load(sys.stdin)
    except (json.JSONDecodeError, ValueError):
        event = {}

    root = pathlib.Path(os.environ.get("CLAUDE_PROJECT_DIR") or event.get("cwd") or os.getcwd())
    rel = target_file(event, root)
    result = 0

    if rel is not None:
        command = build_command(rel, root)
        if command is None:
            sys.stderr.write("[lizard hook] 검사 도구가 없다. docker build -t claudeclangbuild . 를 실행하거나 lizard를 설치한다.\n")
            result = 1
        else:
            proc = subprocess.run(command, capture_output=True, text=True, encoding="utf-8", errors="replace")
            if proc.returncode != 0:
                sys.stderr.write(
                    "[lizard hook] %s 소스 메트릭 기준 초과 (CCN > %d 또는 함수 > %d라인). 함수를 분리해서 해소해라.\n%s%s"
                    % (rel, CCN_LIMIT, LENGTH_LIMIT, proc.stdout, proc.stderr)
                )
                result = 2
    return result


if __name__ == "__main__":
    sys.exit(main())
