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


