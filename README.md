http-server

http-server is a simple http server implement in c with the reactor pattern.

# Benchmark

## Benchmark Methods
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

## Benchmark Result 
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
Document Length:        100 bytes

Concurrency Level:      200
Time taken for tests:   2.238 seconds
Complete requests:      100000
Failed requests:        0
Keep-Alive requests:    100000
Total transferred:      20400000 bytes
HTML transferred:       10000000 bytes
Requests per second:    44682.91 [#/sec] (mean)
Time per request:       4.476 [ms] (mean)
Time per request:       0.022 [ms] (mean, across all concurrent requests)
Transfer rate:          8901.67 [Kbytes/sec] received

Connection Times (ms)
              min  mean[+/-sd] median   max
Connect:        0    0   0.2      0       7
Processing:     0    4  11.7      1      68
Waiting:        0    1   2.4      0      27
Total:          0    4  11.7      1      68

Percentage of the requests served within a certain time (ms)
  50%      1
  66%      1
  75%      1
  80%      2
  90%      8
  95%     44
  98%     47
  99%     49
 100%     68 (longest request)
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
Document Length:        100 bytes

Concurrency Level:      200
Time taken for tests:   5.252 seconds
Complete requests:      100000
Failed requests:        0
Total transferred:      19900000 bytes
HTML transferred:       10000000 bytes
Requests per second:    19041.69 [#/sec] (mean)
Time per request:       10.503 [ms] (mean)
Time per request:       0.053 [ms] (mean, across all concurrent requests)
Transfer rate:          3700.48 [Kbytes/sec] received

Connection Times (ms)
              min  mean[+/-sd] median   max
Connect:        0    4   1.9      4      30
Processing:     1    6   3.2      5      35
Waiting:        0    5   2.6      4      27
Total:          5   10   4.0      9      54

Percentage of the requests served within a certain time (ms)
  50%      9
  66%      9
  75%     10
  80%     11
  90%     15
  95%     19
  98%     24
  99%     28
 100%     54 (longest request)
```

# Nginx benchmark
```sh
$ ab -c 200 -n 100000 http://localhost:8000/
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


Server Software:        nginx/1.18.0
Server Hostname:        localhost
Server Port:            8000

Document Path:          /
Document Length:        13 bytes

Concurrency Level:      200
Time taken for tests:   6.636 seconds
Complete requests:      100000
Failed requests:        0
Total transferred:      19100000 bytes
HTML transferred:       1300000 bytes
Requests per second:    15069.64 [#/sec] (mean)
Time per request:       13.272 [ms] (mean)
Time per request:       0.066 [ms] (mean, across all concurrent requests)
Transfer rate:          2810.84 [Kbytes/sec] received

Connection Times (ms)
              min  mean[+/-sd] median   max
Connect:        0    6   2.4      5      27
Processing:     2    8   3.0      7      30
Waiting:        0    6   2.4      6      29
Total:          8   13   4.2     12      43

Percentage of the requests served within a certain time (ms)
  50%     12
  66%     13
  75%     13
  80%     13
  90%     18
  95%     23
  98%     28
  99%     31
 100%     43 (longest request)
hello@ubuntu:~/code/me/http-server$ ab -k -c 200 -n 100000 http://localhost:8000/This is ApacheBench, Version 2.3 <$Revision: 1843412 $>
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


Server Software:        nginx/1.18.0
Server Hostname:        localhost
Server Port:            8000

Document Path:          /
Document Length:        13 bytes

Concurrency Level:      200
Time taken for tests:   2.380 seconds
Complete requests:      100000
Failed requests:        0
Keep-Alive requests:    99046
Total transferred:      19595230 bytes
HTML transferred:       1300000 bytes
Requests per second:    42013.96 [#/sec] (mean)
Time per request:       4.760 [ms] (mean)
Time per request:       0.024 [ms] (mean, across all concurrent requests)
Transfer rate:          8039.78 [Kbytes/sec] received

Connection Times (ms)
              min  mean[+/-sd] median   max
Connect:        0    0   0.6      0      18
Processing:     0    5   2.6      4      24
Waiting:        0    5   2.6      4      24
Total:          0    5   2.7      4      25

Percentage of the requests served within a certain time (ms)
  50%      4
  66%      4
  75%      6
  80%      6
  90%      8
  95%     10
  98%     13
  99%     15
 100%     25 (longest request)
```

nginx setting
```txt
user www-data;
worker_processes 1;

events {
        worker_connections 768;
}

http {
    server {
      listen 8000;
      location / {
          add_header Content-Type text/plain;
          return 200 'Hello, World!';
      }
    }
}
```