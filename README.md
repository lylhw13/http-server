http_server.md

## Benchmark
### with keep-alive
```sh
ab -k -c 200 -n 100000 http://localhost:333333/
```

### with connection:close
```sh
ab -c 200 -n 100000 http://localhost:333333/
```
```txt
-k     keep-alive Default is no keeplive
-c     concurrency Number of multiple requests to perform at a time. Default is one request at a time.
-n     number of requests
```