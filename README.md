http_server.md

## Benchmark
### with keep-alive
```sh
ab -k -c 200 -n 100000 http://localhost:33333/
```

### with connection:close
```sh
ab -c 200 -n 100000 http://localhost:33333/
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
Time taken for tests:   2.427 seconds
Complete requests:      100000
Failed requests:        0
Keep-Alive requests:    100000
Total transferred:      11700000 bytes
HTML transferred:       1300000 bytes
Requests per second:    41204.08 [#/sec] (mean)
Time per request:       4.854 [ms] (mean)
Time per request:       0.024 [ms] (mean, across all concurrent requests)
Transfer rate:          4707.89 [Kbytes/sec] received

Connection Times (ms)
              min  mean[+/-sd] median   max
Connect:        0    0   1.0      0      26
Processing:     0    5  12.1      1      64
Waiting:        0    1   1.9      0      26
Total:          0    5  12.1      1      64

Percentage of the requests served within a certain time (ms)
  50%      1
  66%      1
  75%      2
  80%      2
  90%      8
  95%     44
  98%     47
  99%     49
 100%     64 (longest request)
```

```sh
$ ab -c 200 -n 100000 http://localhost:33333/
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
Time taken for tests:   5.063 seconds
Complete requests:      100000
Failed requests:        0
Total transferred:      11200000 bytes
HTML transferred:       1300000 bytes
Requests per second:    19749.97 [#/sec] (mean)
Time per request:       10.127 [ms] (mean)
Time per request:       0.051 [ms] (mean, across all concurrent requests)
Transfer rate:          2160.15 [Kbytes/sec] received

Connection Times (ms)
              min  mean[+/-sd] median   max
Connect:        0    4   1.7      4      31
Processing:     1    6   2.8      6      45
Waiting:        1    4   2.4      4      33
Total:          5   10   3.6      9      55

Percentage of the requests served within a certain time (ms)
  50%      9
  66%     10
  75%     10
  80%     10
  90%     11
  95%     16
  98%     21
  99%     25
 100%     55 (longest request)
```