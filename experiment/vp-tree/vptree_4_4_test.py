import logging
logging.basicConfig(level=logging.NOTSET)

import numpy 
import sys 
import nmslib 
import time 
import math 
import random

print(sys.version)
print("NMSLIB version:", nmslib.__version__)


def ivecs_read(fname):
    a = numpy.fromfile(fname, dtype='int32')
    d = a[0]
    return a.reshape(-1, d + 1)[:, 1:].copy()


def fvecs_read(fname):
    return ivecs_read(fname).view('float32')


all_data_matrix = fvecs_read("/home/ubuntu/lulingling/testnmslib/sift/sift_base.fvecs") 
all_data_matrix_query = fvecs_read("/home/ubuntu/lulingling/testnmslib/sift/sift_query.fvecs")  

query_matrix = all_data_matrix_query[0:100]
data_matrix = all_data_matrix[0:600000]#800100   1000000 
print('Load data\n')



# Set index parameters
# These are the most important onese

num_threads = 1
#index_time_params = {'bucketSize':10, 'chunkBucket': 1,'tuneK':10, 'desiredRecall':0.9}#, 
index_time_params = {'bucketSize':10, 'chunkBucket': 1}

print('Index-time parameters', index_time_params)

# Number of neighbors 
K=10
# Space name should correspond to the space name 
# used for brute-force search
space_name='l2'#
# Intitialize the library, specify the space, the type of the vector and add data points 
index = nmslib.init(method='vptree', space=space_name, data_type=nmslib.DataType.DENSE_VECTOR) 
index.addDataPointBatch(data_matrix) #  

# Create an index
start = time.time()
index.createIndex(index_time_params) 

end = time.time() 
print('Index-time parameters', index_time_params)
print('Indexing time = %f' % (end-start))

# Setting query-time parameters 查询参数?
# efS = 100
#query_time_params = {'alphaLeft': 2.0, 'alphaRight': 2.0, 'expLeft': 1, 'expRight': 1,'maxLeavesToVisit': 500} #原本的例子

# query_time_params = {'alphaLeft': 4.74275, 'alphaRight': 2.69508, 'expLeft': 1, 'expRight': 1,'maxLeavesToVisit': 5000} #10万数据

#query_time_params = {'alphaLeft': 3.51986, 'alphaRight': 2.45497, 'expLeft': 1, 'expRight': 1, 'maxLeavesToVisit': 7000} #20万数据

#query_time_params = {'alphaLeft': 4.00837, 'alphaRight': 2.30051, 'expLeft': 1, 'expRight': 1,'maxLeavesToVisit': 8000} #30万数据

#query_time_params = {'alphaLeft': 4.00837, 'alphaRight': 2.30051, 'expLeft': 1, 'expRight': 1, 'maxLeavesToVisit': 10000}#40万数据

query_time_params = {'alphaLeft': 3.75618, 'alphaRight': 2.30051, 'expLeft': 1, 'expRight': 1, 'maxLeavesToVisit': 12000}#60万数据

#query_time_params = {'alphaLeft': 3.75618, 'alphaRight': 2.30051, 'expLeft': 1, 'expRight': 1, 'maxLeavesToVisit': 18000}#80万数据

#query_time_params = {'alphaLeft': 3.75618, 'alphaRight': 2.30051, 'expLeft': 1, 'expRight': 1, 'maxLeavesToVisit': 20000}#100万数据


print('Setting query-time parameters', query_time_params)
index.setQueryTimeParams(query_time_params)


nbrs = index.knnQueryBatch(query_matrix, k = K, num_threads = num_threads)
index.saveIndex('dense_index_vptree.bin')
