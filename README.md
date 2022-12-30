# Http-asio

This is a http server written in C++, initally to further improve upon my C++ skills.

## Features

| Syntax      | Description |
| ----------- | ----------- |
| HTTP 1 | ✅ |
| HTTP 1.1 | ✅ |
| HTTP 2.x | ❌ |
| SSL | ✅ |


## Enter dev environment

```
nix develop
```

## Build docker image

```
nix build #.docker
```
Env var: HTTP_ASIO_CONFIG_PATH=location of config.json
## Running

```
http_server --path config.json
```

## Benchmark

```
wrk -t12 -c400 -d30s http://127.0.0.1:8080/index.html                 

Running 30s test @ http://127.0.0.1:8080/index.html
  12 threads and 400 connections
  Thread Stats   Avg      Stdev     Max   +/- Stdev
    Latency   133.46ms    5.52ms 138.79ms   99.51%
    Req/Sec   247.71     68.80   555.00     58.06%
  88876 requests in 30.06s, 20.77MB read
  Socket errors: connect 0, read 88874, write 0, timeout 0
Requests/sec:   2956.39
Transfer/sec:    707.34KB
```
