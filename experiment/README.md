# Overview

ANNProof project is a verifiable outsourced approximate nearest neighbor search (K-ANNS) framework. We offer a generation and validation tool for outsourced ANN queries. We build the ANNProof framework on blockchain to ensure dataset provider (DP) delivers reliable datasets to service providers (SP). SPs are responsible for answering queries for users. Users can verify the correctness of query results with $VO_{SP}$ from SP and $VO_{chain}$ from the blockchain. 

We build a permissioned blockchain network utilizing **Hyperledger Fabric v2.2**.We employ an endorsement policy adhering to a ``three out of four" requirement, signifying that consistent endorsement must be provided by at least three out of the four SPs.
We developed a **nmslib library variant**, **v-nmslib**, which SP employs to construct indexes and ADS on individual nodes. The query user operates on a single node. 
We implement the query processing and the result verification programs in C++. 
We also use the **Keccak-256** cryptographic hash function for VO generation. 

---

* We offer the source code of ANNProof in [source directory](source)

* We offer the blockchain network and smart contract code in [blockchain  directory](/blockchain). 

* We offer the benchmark script for baseline and ANNProof in [experiment directory](/experiment).  

# Datasets

You can find and download the test datasets we used for the experimental analysis in paper. 
We have a number of precomputed data sets for this. All data sets are pre-split into train/test and come with ground truth data in the form of the top 100 neighbors. We store them in a HDF5 format:

| Dataset                                                           | Dimensions | Train size | Test size | Neighbors | Distance  | Download                                                                   |
| ----------------------------------------------------------------- | ---------: | ---------: | --------: | --------: | --------- | -------------------------------------------------------------------------- |
| [DEEP1B](http://sites.skoltech.ru/compvision/noimi/)              |         96 |  9,990,000 |    10,000 |       100 | Angular   | [HDF5](http://ann-benchmarks.com/deep-image-96-angular.hdf5) (3.6GB)
| [Fashion-MNIST](https://github.com/zalandoresearch/fashion-mnist) |        784 |     60,000 |    10,000 |       100 | Euclidean | [HDF5](http://ann-benchmarks.com/fashion-mnist-784-euclidean.hdf5) (217MB) |
| [GIST](http://corpus-texmex.irisa.fr/)                            |        960 |  1,000,000 |     1,000 |       100 | Euclidean | [HDF5](http://ann-benchmarks.com/gist-960-euclidean.hdf5) (3.6GB)          |
| [GloVe](http://nlp.stanford.edu/projects/glove/)                  |         25 |  1,183,514 |    10,000 |       100 | Angular   | [HDF5](http://ann-benchmarks.com/glove-25-angular.hdf5) (121MB)            |
| GloVe                                                             |         50 |  1,183,514 |    10,000 |       100 | Angular   | [HDF5](http://ann-benchmarks.com/glove-50-angular.hdf5) (235MB)            |
| GloVe                                                             |        100 |  1,183,514 |    10,000 |       100 | Angular   | [HDF5](http://ann-benchmarks.com/glove-100-angular.hdf5) (463MB)           |
| GloVe                                                             |        200 |  1,183,514 |    10,000 |       100 | Angular   | [HDF5](http://ann-benchmarks.com/glove-200-angular.hdf5) (918MB)           |
| [Kosarak](http://fimi.uantwerpen.be/data/)                        |      27,983 |     74,962 |       500 |       100 | Jaccard   | [HDF5](http://ann-benchmarks.com/kosarak-jaccard.hdf5) (33MB)             |
| [MNIST](http://yann.lecun.com/exdb/mnist/)                        |        784 |     60,000 |    10,000 |       100 | Euclidean | [HDF5](http://ann-benchmarks.com/mnist-784-euclidean.hdf5) (217MB)         |
| [MovieLens-10M](https://grouplens.org/datasets/movielens/10m/)  |      65,134 |     69,363 |       500 |       100 | Jaccard   | [HDF5](http://ann-benchmarks.com/movielens10m-jaccard.hdf5) (63MB)             |
| [NYTimes](https://archive.ics.uci.edu/ml/datasets/bag+of+words)   |        256 |    290,000 |    10,000 |       100 | Angular   | [HDF5](http://ann-benchmarks.com/nytimes-256-angular.hdf5) (301MB)         |
| [SIFT](http://corpus-texmex.irisa.fr/)                           |        128 |  1,000,000 |    10,000 |       100 | Euclidean | [HDF5](http://ann-benchmarks.com/sift-128-euclidean.hdf5) (501MB)          |
| [Last.fm](https://github.com/erikbern/ann-benchmarks/pull/91)     |         65 |    292,385 |    50,000 |       100 | Angular   | [HDF5](http://ann-benchmarks.com/lastfm-64-dot.hdf5) (135MB)               |

















* In step 1, SP receives the dataset from DP and builds the hierarchical navigable small world graphs (HNSW) index on the dataset.

* In step 2, SP invokes the ADS Construction Smart Contract to build local ADS.

* In step 3, we store  ADS Merkle root, denoted as $VO_{chain}$, on the blockchain as the execution result of the ADS construction smart contract. 

* In step 4, $SP$ offers a verifiable query API to users. Given a user query $Q$, $SP$ searches the index to obtain $I_q$ and $R$, then generates $VO_{sp}$ for $I_q$ based on the ADS. Upon receiving $\{R, I_q, VO_{sp}\}$ from $SP$, the client user retrieves $VO_{chain}$. Combining the two cryptographic proofs, $VO_{sp}$ and $VO_{chain}$, the user can verify $I_q$'s correctness and subsequently execute a local search algorithm to validate $R$.

* For more details, please refer to our paper.

![image](</picture/architecture.JPG>)

# Prerequisites

**Our development environment**：We develop the program in Ubuntu 20.04.4 Trusty operating system. The code may not be compatible with compilation on Windows.

1. A modern compiler that supports C++11: G++ 4.7, Intel compiler 14, Clang 3.4, or Visual Studio 14 (version 12 can probably be used as well, but the project files need to be downgraded).

2. **64-bit** Linux is recommended, but most of our code builds on **64-bit** Windows and MACOS as well. 

3. Only for Linux/MACOS: CMake (GNU make is also required) 

4. An Intel or AMD processor that supports SSE 4.2 is recommended

5. Extended version of the library requires a development version of the following libraries: Boost, GNU scientific library, and Eigen3.
   To install additional prerequisite packages on Ubuntu, type the following
   
   ```
   sudo apt-get install libboost-all-dev libgsl0-dev libeigen3-dev
   ```

# Quick Start

#### 1.First, you can install vnmslib from source and compile the source code.

```
git clone https://github.com/lulinglingcufe/vnmslib.git
```

Make sure you satisfy the above prerequisites. To compile, go to the directory **source/similarity_search** and type:

```shell
cmake .
make 
```

##### 2.Then, you must compile the customized nmslib package we developed to use it within Python3.

To compile, go to the directory **source/python_bindings*** and type:

```
rm -R -f build
rm -R -f dist
rm -R -f nmslib.egg-info
rm -R -f similarity_search
cp -R ../similarity_search/  ../python_bindings/
pip3 uninstall  nmslib  -y
python3 setup.py install
```

Delete the old compilation intermediate files, then copy the '**source/similarity_search**' folder to the '**source/python_bindings**' folder. Make sure that **pip3** has removed the old version of nmslib. Finally, use the **setup.py** script to install the customized version of nmslib.

##### 3.Finally, you can use vnmslib in python.

Go to the directory **experiment** and type:

```
python3   hnsw_6_19_glove.py
```