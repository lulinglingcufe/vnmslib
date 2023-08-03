# Get Dataset Recall

To obtain the results for the **experimental section (Section VI.D of the paper)** and understand the impact of different hnsw parameters on the ANNProof verifiable query performance, we can use the following scripts to obtain the recall for the query indices using hnsw indexing: 

Recall for the **lastfm** dataset using hnsw indexing:

```
python3 lastfm_recall4_19__hnsw_op.py
```

Recall for the **GloVe-25 (50, 100, 200)** dataset using hnsw indexing:

```
python3 glove_recall4_19__hnsw_op.py
```

Recall for the **Sift-128** dataset using hnsw indexing:

```
python3 testrecall4_16__hnsw_op.py
```

Recall for the **Sift-128** dataset using vp-tree indexing:

```
python3   vptree_1_test.py
```

(Note: Please ensure that the necessary datasets and libraries are available and properly configured before running the scripts.) 
