# f16 server

## About f16 server

The `f16` command includes a high-performance static web server,
which leverages `f16lib` library capabilities.

The server supports two modes of operation:
`serve` and `config`.

### Serve mode
In `serve` mode, `f16` serves static files from a specified root directory.

```sh
f16 serve <root_doc> [--bind=<address> --port=<port>]
```

- `<root_doc>`: The root directory to serve files from.
- `--bind=<address>`: The binding address (default: `0.0.0.0`).
- `--port=<port>`: The port to listen on (default: `80`).

### Config mode
In `config` mode, `f16` loads a JSON configuration file to define multiple servers with custom settings.

```sh
f16 config <config_path>
```

- `<config_path>`: The path to the JSON configuration file.

For the complete configuration reference, see [README_f16server_config.md](README_f16server_config.md).

### Example JSON configuration

```json
{
  "servers":
  [
    {
      "listen_address": "0.0.0.0",
      "listen_port": "4000",
      "ssl":
      {
        "certificate": "cert.pem",
        "certificate_key": "key.pem",
        "dhparam": "dh4096.pem",
        "password": "123456",
        "protocols": ["TLSv1.3"],
        "ciphers": "HIGH:!aNULL:!MD5",
        "prefer_server_ciphers": true,
        "verify_client": true,
        "client_certificate": "client.crt",
        "session_timeout_secs": 300,
        "session_cache": true,
        "session_cache_size": 40000 // about 10 MB
      },
      "locations":
      [
        {
          "location": "/",
          "root": "/var/www/html"
        },
        {
          "location": "/logs",
          "root": "/var/log"
        }
      ]
    },
    {
      "listen_address": "0.0.0.0",
      "listen_port": "5000",
      "locations":
      [
        {
          "location": "/blog",
          "root": "/var/www/blog/"
        },
        {
          "location": "/news",
          "root": "/var/www/news/"
        }
      ]
    }

  ]
}
```

### Configuration options

- `listen_address`: The binding address.
- `listen_port`: The listening port.
- `logs`: Optional global logging configuration (`flush_interval_ms`, `access`, `error`).
- `ssl`: SSL/TLS configuration.
- `return`: Optional fixed reply configuration.
- `locations`: A list of location-root mappings.

Each server entry should define one of `return` or `locations`.

### Request hardening behavior

The server uses `f16lib` request hardening defaults:

- malformed request syntax -> `400 Bad Request`
- request limits exceeded -> `413 Request Entity Too Large`
- read timeout expired -> `408 Request Timeout`

At the moment, request/body/header limits and read timeouts are configured through the library API (`server_options`) and are not yet exposed as CLI flags or JSON fields in `f16 config` mode.

### Command-line options

```sh
Usage:
  f16 serve <root_doc> [--bind=<address> --port=<port>]
  f16 config <config_path>
  f16 (-h | --help)
  f16 --version

Options:
  -h --help            Show this screen.   

  -v --version         Show version.
  -b --bind=<address>  The binding address [default: 0.0.0.0].
  -p --port=<port>     The port [default: 80].
```
