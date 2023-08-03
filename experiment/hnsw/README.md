# HNSW Experiment

Experiment Data for the **Sift-128** dataset using hnsw indexing:

```
python3 hnsw_4_16_test.py
```

Experiment Data for the **lastfm-64** dataset using hnsw indexing:

```
python3 hnsw_4_19_lastfm.py
```
Experiment Data for the **GloVe-25 (50, 100, 200)** dataset using hnsw indexing:


```
python3   hnsw_6_19_glove.py
```


With the HNSW constructed, we can obtain experimental data similar to the following output: 

1. SP ASD time、VO size、User Verification Time： 

2. Storage cost, indexing cost


```
[GCC 7.5.0]
NMSLIB version: 2.1.2
Index-time parameters {'M': 8, 'indexThreadQty': 1, 'efConstruction': 100, 'post': 2}
INFO:nmslib:M                   = 8
INFO:nmslib:indexThreadQty      = 1
INFO:nmslib:efConstruction      = 100
INFO:nmslib:maxM			          = 8
INFO:nmslib:maxM0			          = 16
INFO:nmslib:mult                = 0.480898
INFO:nmslib:skip_optimized_index= 0
INFO:nmslib:delaunay_type       = 2
INFO:nmslib:Set HNSW query-time parameters:
INFO:nmslib:ef(Search)         =20
INFO:nmslib:algoType           =2
INFO:nmslib:
The space is Euclidean
INFO:nmslib:Vector length=128
INFO:nmslib:Thus using an optimised function for base 16
INFO:nmslib:searchMethod			  = 3
INFO:nmslib:Making optimized index
INFO:nmslib:>>>> SearchTime_wtmCreateIndex :         28938.9

INFO:nmslib:Finished making optimized index
INFO:nmslib:Maximum level = 6
INFO:nmslib:Total memory allocated for optimized index+data: 57 Mb

INFO:nmslib:>>>> SearchTime_wtmstore_friends :         186.23

```
