# TOPK



## v1: on 6th October

### solution

divide and conquer: Because of 1GB memory limit, we can't load 100GB url into memory at the same time.. So we can split the raw URL input into 1000 shards, for each shard, in expectation, its size will be 100MB, which can be loaded into memory. Then we can get topk for each shard.
So the code is similar as following:
1. Split input into 1000 shards, make sure same urls are stored into the same shard
2. for each shard, use hash_map to calculate the count of every url and update current topk(priority_queue)
3. current topk is the answer


### evaluation

For 1GB data, it takes 120s, of which:
- 30s to split
- 10s to read shard files
- 80s maybe the **software cost**, most of which results from hash_map insert operation


## v2: on 8th October

### effective optimization

For 1GB data
1. g++ -O2 or -O3: hash_map cost reduces to **20s**!!!

### current evaluation

- 30s to split(read 1GB and write 1GB(1000 shards))
- 10s read 1000 shards
- 20s: hash_map operations


### failed optimization

1. fast writefile by buffered write into file: failed because maybe OS has done this
2. fast_readfile by call syscall:read directly rather than fscanf: failed because the performance is similar(read 1GB file: about 10s)


## Tools used

linux perf
gperftools

## Environment

### Hardware
8 cores
16GB memory
100~200MB/s HDD bindwidth

### Software
Ubuntu18.04 Desktop(Linux 4.15.0)
ext4 
