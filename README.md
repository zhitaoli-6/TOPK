# TOPK

## v1

### solution

1. divide and conquer
2. single core used

### evaluation

For 1GB data, it takes 120s, of which:
- 30s to split
- 10s to read shard files
- 80s maybe the **software cost**

