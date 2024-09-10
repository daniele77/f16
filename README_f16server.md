# f16 server

## About f16 server

f16 server is a high-performance static web server,
which leverages `f16lib` library’s capabilities.

f16 server is a command-line tool that offers two modes of operation:
simple and advanced.

### Simple Mode
In simple mode, `f16-server` serves static files from a specified root directory.

```sh
f16 simple <root_doc> [--bind=<address> --port=<port>]
```

- <root_doc>: The root directory to serve files from.
- --bind=<address>: The binding address (default: 0.0.0.0).
- --port=<port>: The port to listen on (default: 80).

### Advanced Mode
In advanced mode, `f16-server` loads a JSON configuration file to define multiple servers with custom settings.

```sh
f16 advanced <cfg_file>
```

- <cfg_file>: The path to the JSON configuration file.

### Example JSON configuration:

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

### Configuration options:

- listen_address: The binding address.
- listen_port: The listening port.
- ssl: SSL/TLS configuration.
- locations: A list of location-root mappings.

### Command-line options

```sh
Usage:
  f16 simple <root_doc> [--bind=<address> --port=<port>]
  f16 advanced <cfg_file>
  f16 (-h | --help)
  f16 --version

Options:
  -h --help            Show this screen.   

  -v --version         Show version.
  -b --bind=<address>  The binding address [default: 0.0.0.0].
  -p --port=<port>     The port [default: 80].
```
