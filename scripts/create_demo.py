#!/usr/bin/env python3
"""
Create a taskman database filled with a real-life example (e-commerce site MVP project).
Usage: create_demo.py <taskman-path> [db-path]
  taskman-path  (REQUIRED) Path to the taskman executable
  db-path       (optional) Output SQLite file (default: ./demo.db)
"""

import json
import os
import subprocess
import sys


def run(taskman: str, db_path: str, cmd: list[str], *, capture: bool = False) -> subprocess.CompletedProcess:
    env = os.environ.copy()
    env["TASKMAN_DB_NAME"] = db_path
    return subprocess.run(
        [taskman] + cmd,
        env=env,
        capture_output=capture,
        text=True,
    )


def run_ok(taskman: str, db_path: str, cmd: list[str], *, capture: bool = False) -> str | None:
    r = run(taskman, db_path, cmd, capture=capture)
    if r.returncode != 0:
        if r.stderr:
            print(r.stderr, file=sys.stderr)
        sys.exit(1)
    return r.stdout if capture else None


def main() -> None:
    if len(sys.argv) < 2:
        print("Usage: create_demo.py <taskman-path> [db-path]", file=sys.stderr)
        print("  taskman-path  (REQUIRED) Path to the taskman executable", file=sys.stderr)
        print("  db-path       (optional) SQLite file (default: ./demo.db)", file=sys.stderr)
        sys.exit(1)

    taskman_exe = sys.argv[1]
    db_path = sys.argv[2] if len(sys.argv) > 2 else "./demo.db"

    if not os.path.isfile(taskman_exe):
        print(f"Error: executable not found: {taskman_exe}", file=sys.stderr)
        sys.exit(1)

    # Empty DB for a reproducible demo
    if os.path.exists(db_path):
        try:
            os.remove(db_path)
        except OSError as e:
            print(f"Error: cannot remove {db_path} ({e}). Close any process using it.", file=sys.stderr)
            sys.exit(1)
    for p in (db_path + "-journal", db_path + "-wal", db_path + "-shm"):
        if os.path.exists(p):
            try:
                os.remove(p)
            except OSError:
                pass

    print(f"Creating {db_path} with a real-life example (e-commerce site MVP project)...")

    run_ok(taskman_exe, db_path, ["init"])
    print("  init")

    # Phases
    run_ok(taskman_exe, db_path, ["phase:add", "--id", "P1", "--name", "Design", "--status", "in_progress", "--sort-order", "1"])
    run_ok(taskman_exe, db_path, ["phase:add", "--id", "P2", "--name", "Development", "--sort-order", "2"])
    run_ok(taskman_exe, db_path, ["phase:add", "--id", "P3", "--name", "Acceptance", "--sort-order", "3"])
    run_ok(taskman_exe, db_path, ["phase:add", "--id", "P4", "--name", "Delivery", "--sort-order", "4"])
    print("  phases P1-P4")

    # Milestones
    run_ok(taskman_exe, db_path, ["milestone:add", "--id", "M1", "--phase", "P1", "--name", "Specs approved", "--criterion", "Document signed off by client", "--reached", "1"])
    run_ok(taskman_exe, db_path, ["milestone:add", "--id", "M2", "--phase", "P2", "--name", "MVP delivered", "--criterion", "Catalog, cart and test payment operational"])
    run_ok(taskman_exe, db_path, ["milestone:add", "--id", "M3", "--phase", "P3", "--name", "Acceptance OK", "--criterion", "E2E tests passing, blocking bugs resolved"])
    run_ok(taskman_exe, db_path, ["milestone:add", "--id", "M4", "--phase", "P4", "--name", "Production deployment", "--criterion", "App deployed and reachable"])
    print("  milestones M1-M4")

    # Tasks (order compatible with dependencies)
    def add_task(title: str, phase: str, **kwargs) -> str:
        cmd = ["task:add", "--title", title, "--phase", phase]
        if "milestone" in kwargs:
            cmd += ["--milestone", kwargs["milestone"]]
        if "role" in kwargs:
            cmd += ["--role", kwargs["role"]]
        if "description" in kwargs:
            cmd += ["--description", kwargs["description"]]
        out = run_ok(taskman_exe, db_path, cmd, capture=True)
        return json.loads(out)["id"]

    t1 = add_task("Write requirements document", "P1", milestone="M1", role="project-manager")
    t2 = add_task("Specify auth API", "P1", milestone="M1", role="software-architect")
    t3 = add_task("Validate specs", "P1", milestone="M1", role="project-designer")
    t4 = add_task("Implement auth module (API)", "P2", milestone="M2", role="developer", description="Endpoints login, logout, refresh")
    t5 = add_task("Implement login screen", "P2", milestone="M2", role="developer")
    t6 = add_task("Document auth API", "P2", milestone="M2", role="documentation-writer")
    t7 = add_task("Write E2E tests for login", "P3", milestone="M3", role="developer")
    t8 = add_task("Write deployment runbook", "P4", milestone="M4", role="documentation-writer")
    t9 = add_task("Deploy to production", "P4", milestone="M4", role="project-manager")
    print("  tasks 1-9")

    # Dependencies: 4->2, 5->4, 6->4, 7->5, 9->7, 9->8
    run_ok(taskman_exe, db_path, ["task:dep:add", t4, t2])
    run_ok(taskman_exe, db_path, ["task:dep:add", t5, t4])
    run_ok(taskman_exe, db_path, ["task:dep:add", t6, t4])
    run_ok(taskman_exe, db_path, ["task:dep:add", t7, t5])
    run_ok(taskman_exe, db_path, ["task:dep:add", t9, t7])
    run_ok(taskman_exe, db_path, ["task:dep:add", t9, t8])
    print("  dependencies")

    # Realistic state: t1, t2 done; t3, t4 in_progress
    run_ok(taskman_exe, db_path, ["task:edit", t1, "--status", "done"])
    run_ok(taskman_exe, db_path, ["task:edit", t2, "--status", "done"])
    run_ok(taskman_exe, db_path, ["task:edit", t3, "--status", "in_progress"])
    run_ok(taskman_exe, db_path, ["task:edit", t4, "--status", "in_progress"])
    print("  status (t1,t2 done; t3,t4 in_progress)")

    print(f"Done. Database: {os.path.abspath(db_path)}")


if __name__ == "__main__":
    main()
