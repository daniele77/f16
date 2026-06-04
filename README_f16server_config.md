# f16 server configuration reference

This document describes the JSON configuration accepted by the `f16` command in `config` mode.

## Command

Run the server with:

```sh
f16 config <config_path>
```

## Top-level structure

```json
{
  "logs": { ... },
  "servers": [ ... ]
}
```

- `servers` is required.
- `logs` is optional.

---

## `logs` section (global)

`logs` configuration is global (shared by all configured servers).

Behavior notes when `logs` is omitted:
- Per-server `access` and `error` loggers are not configured from JSON.
- Internal `spdlog::info/warn/error` emitted by the process still follow spdlog default logger behavior.

### Supported fields

- `flush_interval_ms` (integer, optional)
  - Periodic flush interval for all registered loggers.
  - default: `1000` ms.
  - If `<= 0`, default is used.

- `access` (object, optional)
  - `enabled` (bool, optional, default: `true`)
  - `level` (string, optional, default: `info`) — minimum log level for access logger
  - `flush_on_level` (string, optional, default: `info`)
  - `sinks` (array, optional)

- `error` (object, optional)
  - `enabled` (bool, optional, default: `true`)
  - `level` (string, optional, default: `info`) — minimum log level for error logger
  - `flush_on_level` (string, optional, default: `info`)
  - `sinks` (array, optional)

### Supported sink types

Each entry in `sinks` must contain `type`.

#### `stdout`

```json
{ "type": "stdout", "color": true, "level": "info" }
```

- `color` optional (default: `true`)
- `level` optional (sink-level override)

#### `stderr`

```json
{ "type": "stderr", "color": true, "level": "error" }
```

- `color` optional (default: `true`)
- `level` optional (sink-level override)

#### `file`

```json
{
  "type": "file",
  "path": "/var/log/f16/error.log",
  "max_size": 10485760,
  "max_files": 5,
  "level": "warn"
}
```

- `path` required for file sink
- `max_size` optional (default: `10485760`)
- `max_files` optional (default: `5`)
- `level` optional (sink-level override)
- uses rotating file sink

#### `syslog`

```json
{
  "type": "syslog",
  "ident": "f16",
  "facility": "user",
  "option": "pid",
  "enable_formatting": false,
  "level": "warn"
}
```

- Supported on Unix-like platforms (`__unix__` / `__APPLE__`)
- `ident` optional (default: `"f16"`)
- `facility` optional (default: `"user"`)
  - can be string (`auth`, `authpriv`, `cron`, `daemon`, `ftp`, `kern`, `local0`..`local7`, `lpr`, `mail`, `news`, `syslog`, `user`, `uucp`) or integer
- `option` optional (default: `"pid"`)
  - can be string (`pid`, `cons`, `ndelay`, `odelay`, `nowait`, `perror`) or integer
- `options` optional alternative to `option` (array of strings/integers, bitwise OR)
- `enable_formatting` optional (default: `false`)
- `level` optional (sink-level override)

### Level values

Any value accepted by spdlog `from_str`, typically:

- `trace`
- `debug`
- `info`
- `warn`
- `error`
- `critical`
- `off`

### Not currently supported in sink config

- custom access `format` — currently ignored.

---

## `servers` section

`servers` is an array of server entries.

Each entry supports HTTP or HTTPS depending on presence of `ssl`.

### Common fields

- `listen_address` (string, required)
- `listen_port` (string, optional)
  - default `"80"` for HTTP
  - default `"443"` for HTTPS (`ssl` present)

### Content handling

A server should define one of these sections:

- `return`
- `locations`

If neither is present, server starts but logs a warning and does not handle requests.

---

## `return` section

```json
"return": {
  "status": "moved_permanently",
  "headers": [
    { "Location": "https://$host:4000$request_uri" }
  ]
}
```

- `status` optional, default: `ok`
- `headers` optional, default: empty array

### Supported status values

- `ok`
- `created`
- `accepted`
- `no_content`
- `multiple_choices`
- `moved_permanently`
- `moved_temporarily`
- `not_modified`
- `bad_request`
- `unauthorized`
- `forbidden`
- `not_found`
- `internal_server_error`
- `not_implemented`
- `bad_gateway`
- `service_unavailable`
- `gateway_timeout`
- `http_version_not_supported`

### Header variable substitution

Inside header values, these variables are supported:

- `$host`
- `$request_uri`

Notes:
- If `$host` is used and request has no `Host` header, reply is `400 Bad Request`.
- Host port suffix is stripped before replacement.
- Replacement is applied once per variable occurrence (`$host` and `$request_uri`).

---

## `locations` section

```json
"locations": [
  { "location": "/", "root": "/var/www/html" },
  { "location": "/logs", "root": "/var/log" }
]
```

Each entry requires:

- `location` (URL path prefix)
- `root` (filesystem root)

---

## `ssl` section (optional)

If present, HTTPS server is created.

```json
"ssl": {
  "certificate": "cert.pem",
  "certificate_key": "key.pem",
  "dhparam": "dh4096.pem",
  "password": "secret",
  "protocols": ["TLSv1.3"],
  "ciphers": "HIGH:!aNULL:!MD5",
  "prefer_server_ciphers": true,
  "verify_client": true,
  "client_certificate": "client.crt",
  "session_timeout_secs": 300,
  "session_cache": true,
  "session_cache_size": 40000
}
```

Supported fields:

- `certificate` (string, optional, default: empty)
- `certificate_key` (string, optional, default: empty)
- `dhparam` (string, optional, default: empty)
- `password` (string, optional, default: empty)
- `protocols` (array, optional, default: `TLSv1`, `TLSv1.1`, `TLSv1.2`, `TLSv1.3`)
  - accepted values: `SSLv2`, `SSLv3`, `TLSv1`, `TLSv1.1`, `TLSv1.2`, `TLSv1.3`
- `ciphers` (string, optional, default: empty)
- `prefer_server_ciphers` (bool, optional, default: `false`)
- `verify_client` (bool, optional, default: `false`)
- `client_certificate` (string, optional, default: empty)
  - used only when `verify_client` is `true`
- `session_timeout_secs` (integer, optional, default: `-1` meaning unchanged)
- `session_cache` (bool, optional, default: `false`)
- `session_cache_size` (integer, optional, default: `-1` meaning unchanged)

Behavior notes:
- If `verify_client` is `true`, `client_certificate` is used for peer validation.
- Missing SSL file paths lead to runtime TLS setup errors.

---

## Minimal example

```json
{
  "servers": [
    {
      "listen_address": "0.0.0.0",
      "listen_port": "8080",
      "locations": [
        { "location": "/", "root": "/var/www/html" }
      ]
    }
  ]
}
```

## Full example with logging

```json
{
  "logs": {
    "flush_interval_ms": 500,
    "access": {
      "enabled": true,
      "flush_on_level": "warn",
      "sinks": [
        { "type": "stdout", "color": true },
        { "type": "file", "path": "/var/log/access.log", "max_size": 10485760, "max_files": 5 }
      ]
    },
    "error": {
      "enabled": true,
      "level": "info",
      "flush_on_level": "info",
      "sinks": [
        { "type": "stderr", "color": true, "level": "error" },
        { "type": "file", "path": "/var/log/error.log", "max_size": 10485760, "max_files": 5 }
      ]
    }
  },
  "servers": [
    {
      "listen_address": "0.0.0.0",
      "listen_port": "3000",
      "return": {
        "status": "moved_permanently",
        "headers": [
          { "Location": "https://$host:4000$request_uri" }
        ]
      }
    },
    {
      "listen_address": "0.0.0.0",
      "listen_port": "4000",
      "ssl": {
        "certificate": "cert.pem",
        "certificate_key": "key.pem",
        "dhparam": "dh4096.pem",
        "password": "123456",
        "protocols": ["TLSv1.3"]
      },
      "locations": [
        { "location": "/", "root": "/var/www/html" }
      ]
    }
  ]
}
```
