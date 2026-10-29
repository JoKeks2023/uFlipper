#!/usr/bin/env python3
"""Read-only, independent development check of a completed uFlipper backup.

Never opens the device, changes the catalog, or prints device filenames/content.
"""
import argparse
import datetime
import hashlib
import json
from pathlib import Path, PurePosixPath
import re
import sqlite3

ID = re.compile(r"[0-9a-f]{32}")

def digest(path):
    value = hashlib.sha256()
    with path.open("rb") as stream:
        for chunk in iter(lambda: stream.read(65536), b""):
            value.update(chunk)
    return value.hexdigest()

def verify(folder, expect_full=False):
    folder = folder.resolve(strict=True)
    backup_id = folder.name
    if not ID.fullmatch(backup_id):
        raise ValueError("Invalid backup identifier")
    database = folder.parent / "catalog.sqlite"
    with sqlite3.connect(database.as_uri() + "?mode=ro", uri=True) as db:
        info = db.execute("SELECT scope,state,manifest_sha256,inventory FROM backups WHERE id=?", (backup_id,)).fetchone()
        if not info or info[1] != "verified":
            raise ValueError("Backup catalog does not mark this backup verified")
        manifest_file = folder / "manifest.json"
        manifest_hash = digest(manifest_file)
        if manifest_hash != info[2]:
            raise ValueError("Manifest differs from the catalog checksum")
        manifest = json.loads(manifest_file.read_bytes())
        full = info[0] == "full"
        if expect_full and not full:
            raise ValueError("Test-subset backup cannot satisfy a full-device check")
        if manifest["format"] != 1 or manifest["id"] != backup_id or manifest["scope"] != ("full-device" if full else "test-subset"):
            raise ValueError("Unsupported or inconsistent manifest identity/scope")
        if full and info[3] != 3:
            raise ValueError("Full backup lacks both inventory confirmations")
        rows = db.execute("SELECT path,kind,blob,bytes,sha256,ready FROM entries WHERE backup=? ORDER BY path", (backup_id,)).fetchall()
    entries = manifest["entries"]
    if len(rows) != len(entries):
        raise ValueError("Manifest and catalog have different entry counts")
    files = directories = total = empty_files = 0
    seen = set()
    for row, entry in zip(rows, entries):
        device_path, kind, blob, size, sha, ready = row
        parts = PurePosixPath(device_path).parts
        if not device_path.startswith(("/int/", "/ext/")) and device_path not in ("/int", "/ext"):
            raise ValueError("Entry outside device storage roots")
        if ".." in parts or str(PurePosixPath(device_path)) != device_path or "\x00" in device_path or device_path in seen:
            raise ValueError("Unsafe or duplicate device path")
        seen.add(device_path)
        directory = kind == "directory"
        if kind not in ("directory", "file") or not ready or entry != dict(path=device_path, directory=directory, bytes=size, sha256=sha, blob=blob):
            raise ValueError("Manifest entry differs from its completed catalog entry")
        if directory:
            if blob or sha or size:
                raise ValueError("Directory contains unexpected payload fields")
            directories += 1
        else:
            if not ID.fullmatch(blob) or size < 0:
                raise ValueError("Invalid payload identifier or length")
            payload = folder / "payload" / blob
            if payload.is_symlink() or not payload.is_file() or payload.stat().st_size != size or digest(payload) != sha:
                raise ValueError("Payload content or length differs from its manifest")
            files += 1
            total += size
            empty_files += size == 0
    if full and not {"/int", "/ext"}.issubset(seen):
        raise ValueError("Full backup lacks internal/SD root directories")
    return dict(success=True, backupId=backup_id, scope=manifest["scope"], files=files, directories=directories,
                emptyFiles=empty_files, bytes=total, manifestSha256=manifest_hash,
                checkedAt=datetime.datetime.now(datetime.timezone.utc).isoformat(),
                method="independent-python-sqlite-manifest-sha256")

def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("directory", type=Path)
    parser.add_argument("--expect-full", action="store_true")
    args = parser.parse_args()
    try:
        result = verify(args.directory, args.expect_full)
    except (ValueError, KeyError, OSError, sqlite3.Error, TypeError) as error:
        print(json.dumps(dict(success=False, error=str(error))))
        return 1
    print(json.dumps(result, indent=2))
    return 0

if __name__ == "__main__":
    raise SystemExit(main())
