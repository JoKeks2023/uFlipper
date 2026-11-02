#!/usr/bin/env python3
"""Official MCP Python SDK integration against the synthetic host or native app.
No device actions are performed by this script; native mode reads status only.
Run with mcp==2.3.0 in a separate environment.
"""
import asyncio
import importlib.metadata
import json
import os
import pathlib
import subprocess
import sys
import tempfile
from mcp import Client
from mcp.client.stdio import StdioServerParameters
from stdio_client import line, close

async def exercise(command, args, fixture=True):
    versions = []
    env = {key: os.environ[key] for key in ("DYLD_FRAMEWORK_PATH", "LD_LIBRARY_PATH") if key in os.environ}
    for mode, expected in (("auto", "2026-07-28"), ("legacy", "2025-11-25")):
        params = StdioServerParameters(command=command, args=args, env=env)
        async with Client(params, mode=mode, read_timeout_seconds=10) as client:
            assert client.protocol_version == expected, client.protocol_version
            listed = await client.list_tools()
            names = [tool.name for tool in listed.tools]
            name = "fixture_status" if fixture else "device_status"
            assert name in names, names
            result = await client.call_tool(name, {})
            assert not result.is_error
            if fixture:
                assert result.structured_content == {"fixture": True}
            else:
                assert "connected" in result.structured_content
                for tool in ("capabilities", "jobs_list", "backup_status", "firmware_status", "recordings_list"):
                    reply = await client.call_tool(tool, {})
                    assert not reply.is_error, tool
                # Deliberately no nfc_start/job_stop/device mutation.
            versions.append({"mode": mode, "version": client.protocol_version, "tools": names})
    return versions

def main():
    binary = str(pathlib.Path(sys.argv[1]).resolve())
    if "--native" in sys.argv:
        versions = asyncio.run(exercise(binary, ["--mcp"], False))
        backend = "native shared backend; status only, no device actions"
    else:
        with tempfile.TemporaryDirectory(prefix="uflipper-mcp-sdk-") as folder:
            host = subprocess.Popen([binary, "--fixture-server", folder], stdin=subprocess.PIPE, stdout=subprocess.PIPE, stderr=subprocess.PIPE, bufsize=0)
            try:
                assert line(host.stdout) == b"ready:disabled\n"
                host.stdin.write(b"grant\n"); host.stdin.flush()
                assert line(host.stdout) == b"enabled\n"
                versions = asyncio.run(exercise(binary, ["--relay", folder]))
            finally:
                close(host)
        backend = "synthetic; no device"
    print(json.dumps({"sdk": "mcp", "sdkVersion": importlib.metadata.version("mcp"), "backend": backend, "passed": versions}, indent=2))

if __name__ == "__main__":
    main()
