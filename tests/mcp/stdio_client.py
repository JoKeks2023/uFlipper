#!/usr/bin/env python3
"""Independent Python wire client driving the production C++ stdio relay/gateway.
The host backend is explicitly synthetic; this is not a device evidence script.
"""
import json, os, pathlib, selectors, socket, stat, subprocess, sys, tempfile

def line(stream, timeout=8):
    selector = selectors.DefaultSelector()
    selector.register(stream, selectors.EVENT_READ)
    try:
        if not selector.select(timeout):
            raise AssertionError("Timed out waiting for a subprocess line")
        result = stream.readline()
        if not result:
            raise AssertionError("Unexpected EOF")
        return result
    finally:
        selector.close()

def send(process, value):
    process.stdin.write(json.dumps(value).encode() + b"\n")
    process.stdin.flush()

def rpc(process, method, params=None, request_id=1):
    send(process, {"jsonrpc": "2.0", "id": request_id, "method": method, "params": params or {}})
    reply = json.loads(line(process.stdout))
    assert reply["id"] == request_id and reply["jsonrpc"] == "2.0", reply
    return reply

def modern():
    return {"_meta": {"io.modelcontextprotocol/protocolVersion": "2026-07-28",
                      "io.modelcontextprotocol/clientCapabilities": {},
                      "io.modelcontextprotocol/clientInfo": {"name": "independent-python-wire-client", "version": "1"}}}

def relay(binary, root):
    return subprocess.Popen([binary, "--relay", str(root)], stdin=subprocess.PIPE, stdout=subprocess.PIPE, stderr=subprocess.PIPE, bufsize=0)

def close(process):
    if process.stdin and not process.stdin.closed:
        process.stdin.close()
    try:
        process.wait(timeout=5)
    except subprocess.TimeoutExpired:
        process.kill()
        process.wait()
        raise AssertionError("Process did not stop on stdin EOF")

def run(binary):
    checks = []
    with tempfile.TemporaryDirectory(prefix="uflipper-mcp-contract-") as folder:
        root = pathlib.Path(folder)
        denied = relay(binary, root)
        out, err = denied.communicate(timeout=5)
        assert denied.returncode == 2 and out == b"" and b"disabled" in err, (out, err)
        checks.append("default_denied")
        host = subprocess.Popen([binary, "--fixture-server", folder], stdin=subprocess.PIPE, stdout=subprocess.PIPE, stderr=subprocess.PIPE, bufsize=0)
        clients = []
        try:
            assert line(host.stdout) == b"ready:disabled\n"
            host.stdin.write(b"grant\n"); host.stdin.flush()
            assert line(host.stdout) == b"enabled\n"
            endpoint = json.loads((root / "endpoint.json").read_text())
            assert stat.S_IMODE((root / "endpoint.json").stat().st_mode) == 0o600
            assert stat.S_IMODE((root / "grant.json").stat().st_mode) == 0o600
            assert stat.S_IMODE(root.stat().st_mode) == 0o700
            checks.append("private_files_and_explicit_grant")
            attacker = socket.socket(socket.AF_UNIX)
            attacker.settimeout(5); attacker.connect(endpoint["socket"])
            attacker.sendall(b'{"token":"wrong"}\n')
            assert attacker.recv(100) == b""; attacker.close()
            checks.append("wrong_token_denied")
            client = relay(binary, root); clients.append(client)
            discovery = rpc(client, "server/discover", modern())["result"]
            assert discovery["resultType"] == "complete" and "2026-07-28" in discovery["supportedVersions"]
            listed = rpc(client, "tools/list", modern())["result"]
            assert listed["tools"][0]["name"] == "fixture_status" and listed["ttlMs"] == 0
            args = modern(); args.update(name="fixture_status", arguments={})
            result = rpc(client, "tools/call", args)["result"]
            assert result["structuredContent"] == {"fixture": True} and not result["isError"]
            assert "error" in rpc(client, "tools/list")  # No cross-request modern capabilities.
            checks.append("modern_discovery_list_call_and_request_isolation")
            legacy = relay(binary, root); clients.append(legacy)
            init = rpc(legacy, "initialize", {"protocolVersion": "2025-11-25", "capabilities": {}, "clientInfo": {"name": "independent-legacy-client", "version": "1"}})
            assert init["result"]["protocolVersion"] == "2025-11-25"
            send(legacy, {"jsonrpc": "2.0", "method": "notifications/initialized"})
            assert rpc(legacy, "tools/call", {"name": "fixture_status"})["result"]["structuredContent"] == {"fixture": True}
            close(legacy); clients.remove(legacy)
            checks.append("legacy_handshake_call_and_eof_shutdown")
            # Malformed JSON returns a protocol error without stdout diagnostics.
            client.stdin.write(b'{\n'); client.stdin.flush()
            assert json.loads(line(client.stdout))["error"]["code"] == -32700
            checks.append("malformed_json_protocol_error")
            host.stdin.write(b"revoke\n"); host.stdin.flush()
            assert line(host.stdout) == b"disabled\n"
            client.wait(timeout=5)
            assert not (root / "endpoint.json").exists() and not (root / "grant.json").exists()
            denied = relay(binary, root); out, err = denied.communicate(timeout=5)
            assert denied.returncode == 2 and out == b""
            checks.append("revocation_disconnects_and_denies_reconnect")
            # A copied grant for another machine must not activate this machine.
            (root / "grant.json").write_text(json.dumps({"schema": 1, "machine": "different-machine", "access": "full"}))
            os.chmod(root / "grant.json", 0o600)
            close(host)
            host = subprocess.Popen([binary, "--fixture-server", folder], stdin=subprocess.PIPE, stdout=subprocess.PIPE, stderr=subprocess.PIPE, bufsize=0)
            assert line(host.stdout) == b"ready:disabled\n"
            checks.append("other_machine_grant_ignored")
        finally:
            for child in clients:
                close(child)
            close(host)
    print(json.dumps({"backend": "synthetic; no device", "passed": checks}, indent=2))

if __name__ == "__main__":
    run(sys.argv[1])
