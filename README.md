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

## version 1
```sh
$ ab -k -c 200 -n 100000 http://localhost:33333/
This is ApacheBench, Version 2.3 <$Revision: 1843412 $>
Copyright 1996 Adam Twiss, Zeus Technology Ltd, http://www.zeustech.net/
Licensed to The Apache Software Foundation, http://www.apache.org/

Benchmarking localhost (be patient)
Completed 10000 requests
Completed 20000 requests
Completed 30000 requests
Completed 40000 requests
Completed 50000 requests
Completed 60000 requests
Completed 70000 requests
Completed 80000 requests
Completed 90000 requests
Completed 100000 requests
Finished 100000 requests


Server Software:        
Server Hostname:        localhost
Server Port:            33333

Document Path:          /
Document Length:        13 bytes

Concurrency Level:      200
Time taken for tests:   5.376 seconds
Complete requests:      100000
Failed requests:        0
Keep-Alive requests:    100000
Total transferred:      11700000 bytes
HTML transferred:       1300000 bytes
Requests per second:    18601.28 [#/sec] (mean)
Time per request:       10.752 [ms] (mean)
Time per request:       0.054 [ms] (mean, across all concurrent requests)
Transfer rate:          2125.34 [Kbytes/sec] received

Connection Times (ms)
              min  mean[+/-sd] median   max
Connect:        0    0   2.0      0      74
Processing:     0   11  17.2      2      88
Waiting:        0    3   4.3      1      43
Total:          0   11  17.3      2      88

Percentage of the requests served within a certain time (ms)
  50%      2
  66%      5
  75%      9
  80%     12
  90%     46
  95%     50
  98%     53
  99%     56
 100%     88 (longest request)
```