#!/usr/bin/env python3

import argparse
import http.client
import json
import socket
import subprocess
import time
import unittest
from datetime import datetime, timezone
from email.utils import parsedate_to_datetime
from typing import Optional, Tuple


def _wait_until_ready(host: str, port: int, timeout_seconds: float = 8.0) -> None:
    deadline = time.time() + timeout_seconds
    while time.time() < deadline:
        try:
            conn = http.client.HTTPConnection(host, port, timeout=1)
            conn.request("GET", "/health")
            response = conn.getresponse()
            body = response.read().decode("utf-8")
            conn.close()
            if response.status == 200 and "ok" in body:
                return
        except OSError:
            time.sleep(0.1)
        except Exception:
            time.sleep(0.1)
    raise TimeoutError("Server did not become ready in time")


def _reserve_free_port() -> int:
    with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
        s.bind(("127.0.0.1", 0))
        return int(s.getsockname()[1])


class BlackBoxHttpServerTests(unittest.TestCase):
    DATE_MAX_SKEW_SECONDS = 10

    app_path: str = ""
    host: str = "127.0.0.1"
    port: int = 18080
    doc_root: str = "."
    process: Optional[subprocess.Popen] = None

    @classmethod
    def configure(cls, app_path: str, host: str, port: int, doc_root: str) -> None:
        cls.app_path = app_path
        cls.host = host
        cls.port = port
        cls.doc_root = doc_root

    @classmethod
    def setUpClass(cls) -> None:
        cls.process = subprocess.Popen(
            [
                cls.app_path,
                "--host",
                cls.host,
                "--port",
                str(cls.port),
                "--doc-root",
                cls.doc_root,
            ],
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,
            text=True,
        )
        _wait_until_ready(cls.host, cls.port)

    @classmethod
    def tearDownClass(cls) -> None:
        if cls.process is None:
            return

        if cls.process.poll() is None:
            cls.process.terminate()
            try:
                cls.process.wait(timeout=3)
            except subprocess.TimeoutExpired:
                cls.process.kill()
                cls.process.wait(timeout=3)

    def _request(self, method: str, path: str, body: Optional[str] = None, headers=None) -> Tuple[int, dict, str]:
        headers = headers or {}
        conn = http.client.HTTPConnection(self.host, self.port, timeout=3)
        conn.request(method, path, body=body, headers=headers)
        response = conn.getresponse()
        payload = response.read().decode("utf-8")
        raw_headers = response.getheaders()
        self._assert_no_duplicate_headers(raw_headers)
        response_headers = {k.lower(): v for k, v in raw_headers}
        self._assert_date_header_format(response_headers.get("date"))
        status = response.status
        conn.close()
        return status, response_headers, payload

    def _raw_request(self, request_bytes: bytes) -> bytes:
        with socket.create_connection((self.host, self.port), timeout=3) as sock:
            sock.sendall(request_bytes)
            chunks = []
            while True:
                part = sock.recv(4096)
                if not part:
                    break
                chunks.append(part)
        response_bytes = b"".join(chunks)
        self._assert_raw_response_has_valid_date(response_bytes)
        return response_bytes

    def _assert_date_header_format(self, date_value: Optional[str]) -> None:
        self.assertIsNotNone(date_value, "Missing Date header")

        assert date_value is not None
        self.assertRegex(
            date_value,
            r"^(Mon|Tue|Wed|Thu|Fri|Sat|Sun), \d{2} "
            r"(Jan|Feb|Mar|Apr|May|Jun|Jul|Aug|Sep|Oct|Nov|Dec) "
            r"\d{4} \d{2}:\d{2}:\d{2} GMT$",
            "Date header is not in IMF-fixdate format",
        )

        try:
            parsed_dt = parsedate_to_datetime(date_value)
        except (TypeError, ValueError) as exc:
            self.fail(f"Date header is not parseable: {date_value!r} ({exc})")

        if parsed_dt.tzinfo is None:
            parsed_dt = parsed_dt.replace(tzinfo=timezone.utc)

        now_utc = datetime.now(timezone.utc)
        skew_seconds = abs((now_utc - parsed_dt).total_seconds())
        self.assertLessEqual(
            skew_seconds,
            self.DATE_MAX_SKEW_SECONDS,
            f"Date header time skew is too large: {skew_seconds:.3f}s (max {self.DATE_MAX_SKEW_SECONDS}s)",
        )

    def _assert_raw_response_has_valid_date(self, response_bytes: bytes) -> None:
        header_section = response_bytes.split(b"\r\n\r\n", 1)[0]
        header_text = header_section.decode("iso-8859-1", errors="replace")

        header_lines = header_text.split("\r\n")
        self._assert_no_duplicate_raw_headers(header_lines[1:])

        date_value = None
        for line in header_lines:
            if line.lower().startswith("date:"):
                date_value = line.split(":", 1)[1].strip()
                break

        self._assert_date_header_format(date_value)

    def _assert_no_duplicate_headers(self, headers: list[tuple[str, str]]) -> None:
        seen = set()
        duplicates = []

        for name, _ in headers:
            normalized = name.lower()
            if normalized in seen:
                duplicates.append(name)
            else:
                seen.add(normalized)

        self.assertEqual(duplicates, [], f"Duplicate headers found: {duplicates}")

    def _assert_no_duplicate_raw_headers(self, header_lines: list[str]) -> None:
        names = []
        for line in header_lines:
            if not line or ":" not in line:
                continue
            names.append((line.split(":", 1)[0], ""))

        self._assert_no_duplicate_headers(names)

    def test_health_endpoint_json(self) -> None:
        """Test that the /health endpoint returns the expected JSON response with correct headers."""
        status, headers, body = self._request("GET", "/health")
        self.assertEqual(status, 200)
        self.assertEqual(headers.get("content-type"), "application/json")
        self.assertEqual(json.loads(body), {"status": "ok"})

    def test_health_endpoint_json_trailing_slash(self) -> None:
        """Test that the /health/ endpoint (with trailing slash) returns the expected JSON response with correct headers."""
        status, headers, body = self._request("GET", "/health/")
        self.assertEqual(status, 200)
        self.assertEqual(headers.get("content-type"), "application/json")
        self.assertEqual(json.loads(body), {"status": "ok"})

    def test_dynamic_path_parameter(self) -> None:
        """Test that the /hello/{name} endpoint correctly extracts the name parameter and returns the expected greeting."""
        status, _, body = self._request("GET", "/hello/Daniele")
        self.assertEqual(status, 200)
        self.assertEqual(body, "Hello Daniele!")

    def test_query_parameter_sum(self) -> None:
        """Test that the /sum endpoint correctly parses query parameters a and b, computes their sum, and returns the expected JSON response with correct headers."""
        status, headers, body = self._request("GET", "/sum?a=4&b=5")
        self.assertEqual(status, 200)
        self.assertEqual(headers.get("content-type"), "application/json")
        self.assertEqual(json.loads(body), {"sum": 9})

    def test_query_parameter_validation(self) -> None:
        """Test that the /sum endpoint returns a 400 Bad Request status and an appropriate error message when the query parameters a and b are not valid integers."""
        status, _, body = self._request("GET", "/sum?a=foo&b=5")
        self.assertEqual(status, 400)
        self.assertEqual(body, "invalid integers")

    def test_post_and_put_routes(self) -> None:
        """Test that the /items/{id} endpoint correctly handles POST and PUT requests, extracting the id parameter and returning the expected JSON response with correct headers."""
        post_status, post_headers, post_body = self._request("POST", "/items/42")
        self.assertEqual(post_status, 200)
        self.assertEqual(post_headers.get("content-type"), "application/json")
        self.assertEqual(json.loads(post_body), {"method": "POST", "id": "42"})

        put_status, put_headers, put_body = self._request("PUT", "/items/42")
        self.assertEqual(put_status, 200)
        self.assertEqual(put_headers.get("content-type"), "application/json")
        self.assertEqual(json.loads(put_body), {"method": "PUT", "id": "42"})

    def test_echo_post_body(self) -> None:
        """Test that the /echo endpoint correctly echoes back the body of a POST request."""
        payload = "This is a test."
        status, headers, body = self._request("POST", "/echo", body=payload)
        self.assertEqual(status, 200)
        self.assertEqual(headers.get("content-type"), "text/plain")
        self.assertEqual(body, payload)

    def test_path_variables(self) -> None:
        """Test that the /path/{var1}/to/{var2} endpoint correctly extracts multiple path variables and returns them in the response."""
        status, _, body = self._request("GET", "/path/foo/to/bar")
        self.assertEqual(status, 200)
        self.assertEqual(json.loads(body), {"method": "GET", "parameters": "yes", "var1": "foo", "var2": "bar"})

    def test_common_prefix_routes(self) -> None:
        """Test that routes with common prefixes (e.g., /path/foo/to/bar and /path) are correctly distinguished and return the expected responses."""
        status, _, body = self._request("GET", "/path")
        self.assertEqual(status, 200)
        self.assertEqual(json.loads(body), {"method": "GET", "parameters": "no"})

    def test_header_access(self) -> None:
        """Test that the /headers/user-agent endpoint correctly reads the User-Agent header from the request and returns it in the response body."""
        status, _, body = self._request("GET", "/headers/user-agent", headers={"User-Agent": "blackbox-suite/1.0"})
        self.assertEqual(status, 200)
        self.assertEqual(body, "blackbox-suite/1.0")

    def test_static_content_and_head(self) -> None:
        """Test that static files are served correctly with the expected content and headers, and that HEAD requests return the correct headers without a body."""
        get_status, get_headers, get_body = self._request("GET", "/static/index.html")
        self.assertEqual(get_status, 200)
        self.assertEqual(get_headers.get("content-type"), "text/html")
        self.assertIn("f16 blackbox", get_body)

        head_status, head_headers, head_body = self._request("HEAD", "/static/index.html")
        self.assertEqual(head_status, 200)
        self.assertEqual(head_headers.get("content-type"), "text/html")
        self.assertEqual(head_body, "")

    def test_directory_listing_content(self) -> None:
        """Test that a directory without index.html is listed and contains only regular files."""
        status, headers, body = self._request("GET", "/static/listing/")
        self.assertEqual(status, 200)
        self.assertEqual(headers.get("content-type"), "text/html")
        self.assertIn('href="alpha.txt"', body)
        self.assertIn('href="beta.json"', body)
        self.assertNotIn('href="subdir"', body)

    def test_directory_redirect(self) -> None:
        """Test that requests to a directory path without a trailing slash are redirected to the same path with a trailing slash, and that the Location header is set correctly."""
        status, headers, _ = self._request("GET", "/static")
        self.assertEqual(status, 301)
        self.assertEqual(headers.get("location"), "/static/")

    def test_directory_redirect_preserves_query(self) -> None:
        """Test that requests to a directory path without a trailing slash that include query parameters are redirected to the same path with a trailing slash, and that the Location header preserves the query parameters."""
        status, headers, _ = self._request("GET", "/static?source=bb")
        self.assertEqual(status, 301)
        self.assertEqual(headers.get("location"), "/static/?source=bb")

    def test_directory_redirect_head(self) -> None:
        """Test that HEAD requests to a directory path without a trailing slash are redirected to the same path with a trailing slash, and that the Location header is set correctly, while the body is empty."""
        status, headers, body = self._request("HEAD", "/static")
        self.assertEqual(status, 301)
        self.assertEqual(headers.get("location"), "/static/")
        self.assertEqual(body, "")

    def test_directory_redirect_head_preserves_query(self) -> None:
        """Test that HEAD requests to a directory path without a trailing slash that include query parameters are redirected to the same path with a trailing slash, and that the Location header preserves the query parameters, while the body is empty."""
        status, headers, body = self._request("HEAD", "/static?source=bb")
        self.assertEqual(status, 301)
        self.assertEqual(headers.get("location"), "/static/?source=bb")
        self.assertEqual(body, "")

    def test_not_found_dynamic_route(self) -> None:
        """Test that requests to non-existent dynamic routes return a 404 Not Found status."""
        status, _, _ = self._request("GET", "/nonexistent/route")
        self.assertEqual(status, 404)

    def test_not_found_static_file(self) -> None:
        """Test that requests to non-existent static files return a 404 Not Found status."""
        status, _, _ = self._request("GET", "/static/missing_file.txt")
        self.assertEqual(status, 404)

    def test_bad_request_for_parent_path(self) -> None:
        """Test that requests containing parent path segments (e.g., /../secret) are rejected with a 400 Bad Request status to prevent directory traversal attacks."""
        response = self._raw_request(
            b"GET /../secret HTTP/1.0\r\n"
            b"Host: localhost\r\n"
            b"\r\n"
        )
        self.assertIn(b"400 Bad Request", response)

    def test_request_too_large_for_oversized_request_line(self) -> None:
        """Test that an oversized request line is rejected with 413 Request Entity Too Large."""
        oversized_path = b"/" + (b"a" * 9000)
        response = self._raw_request(
            b"GET " + oversized_path + b" HTTP/1.1\r\n"
            b"Host: localhost\r\n"
            b"\r\n"
        )
        self.assertIn(b"413 Request Entity Too Large", response)

    def test_request_too_large_for_oversized_header_section(self) -> None:
        """Test that an oversized header section is rejected with 413 Request Entity Too Large."""
        huge_header = b"x" * 33000
        response = self._raw_request(
            b"GET /health HTTP/1.1\r\n"
            b"Host: localhost\r\n"
            b"X-Big: " + huge_header + b"\r\n"
            b"\r\n"
        )
        self.assertIn(b"413 Request Entity Too Large", response)

    def test_request_too_large_for_oversized_content_length(self) -> None:
        """Test that Content-Length above configured max body size is rejected with 413 Request Entity Too Large."""
        response = self._raw_request(
            b"POST /echo HTTP/1.1\r\n"
            b"Host: localhost\r\n"
            b"Content-Length: 1048577\r\n"
            b"\r\n"
        )
        self.assertIn(b"413 Request Entity Too Large", response)

    def test_request_timeout_for_incomplete_headers(self) -> None:
        """Test that an incomplete request is timed out and rejected with 408 Request Timeout."""
        with socket.create_connection((self.host, self.port), timeout=3) as sock:
            sock.settimeout(4)
            sock.sendall(
                b"GET /health HTTP/1.1\r\n"
                b"Host: localhost\r\n"
            )
            chunks = []
            while True:
                try:
                    part = sock.recv(4096)
                except socket.timeout:
                    self.fail("Expected timeout response, but no response was received")
                if not part:
                    break
                chunks.append(part)

        response = b"".join(chunks)
        self._assert_raw_response_has_valid_date(response)
        self.assertIn(b"408 Request Timeout", response)


def _parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description="Run black-box tests for f16 sample app")
    parser.add_argument("--app", required=True, help="Path to the blackbox sample executable")
    parser.add_argument("--host", default="127.0.0.1")
    parser.add_argument("--port", type=int, default=18080)
    parser.add_argument("--doc-root", required=True)
    return parser.parse_args()


def main() -> int:
    args = _parse_args()
    port = args.port if args.port != 0 else _reserve_free_port()
    BlackBoxHttpServerTests.configure(
        app_path=args.app,
        host=args.host,
        port=port,
        doc_root=args.doc_root,
    )
    suite = unittest.defaultTestLoader.loadTestsFromTestCase(BlackBoxHttpServerTests)
    result = unittest.TextTestRunner(verbosity=2).run(suite)
    return 0 if result.wasSuccessful() else 1


if __name__ == "__main__":
    raise SystemExit(main())