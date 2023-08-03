# VP-Tree Experiment

Cost of constructing VPTree:

```
python3   vptree_4_4_test.py
```

With the VPTree constructed, we can obtain experimental data similar to the following output: 

1. Storage cost, indexing cost

2. SP ASD time、VO size、User Verification Time： 

```
Index-time parameters {'bucketSize': 10, 'chunkBucket': 1}
INFO:nmslib:bucketSize          = 10
INFO:nmslib:chunkBucket         = 1
INFO:nmslib:selectPivotAttempts = 5
INFO:nmslib:alphaLeft = 1 expLeft = 1
INFO:nmslib:alphaRight = 1 expRight = 1
INFO:nmslib:Set polynomial pruner query-time parameters:
INFO:nmslib:alphaLeft: 1 ExponentLeft: 1 alphaRight: 1 ExponentRight: 1
INFO:nmslib:Set VP-tree query-time parameters:
INFO:nmslib:maxLeavesToVisit=2147483647
INFO:nmslib:wtmCreateIndex_vptree_time is          = 11015.1
INFO:nmslib:total_vptree_merkle_node_time is          = 6176.78
INFO:nmslib:root Hash: 
F8F8598F9DC8535E3035E5FB6D32982B85457E331BE7B385693204AFD5206D9B
INFO:nmslib:wtm_get_merkle_root_time is          = 179.617
Index-time parameters {'bucketSize': 10, 'chunkBucket': 1}
Indexing time = 11.196718
```
