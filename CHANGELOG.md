# Changelog
All notable changes to this project will be documented in this file.

This project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [Unreleased]

- Request hardening options (`server_options`) for `http_server`/`https_server`
- Request parser enforcement for size/count limits with dedicated `too_large` parse result.
- HTTP status support for `408 Request Timeout` and `413 Request Entity Too Large` in reply/status mapping.
- Connection handling now maps parser/runtime failures consistently.
- TLS server connections now enforce handshake timeout.
- Logging for request rejection is now categorized and more diagnostic.

## [0.0.1] - 2024-08-20

 - Basic http server and library
 - Basic https server and library
