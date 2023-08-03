/**
 * Non-metric Space Library
 *
 * Main developers: Bilegsaikhan Naidan, Leonid Boytsov, Yury Malkov, Ben Frederickson, David Novak
 *
 * For the complete list of contributors and further details see:
 * https://github.com/nmslib/nmslib
 *
 * Copyright (c) 2013-2018
 *
 * This code is released under the
 * Apache License Version 2.0 http://www.apache.org/licenses/.
 *
 */
#include "ztimer.h"
#include <algorithm>
#include <iomanip>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <cmath>
#include <stdio.h>
#include <unistd.h>
#include <cstdio>

#include "portable_prefetch.h"
#if defined(_WIN32) || defined(WIN32)
#include <intrin.h>
#endif

#include "portable_simd.h"
#include "space.h"
#include "rangequery.h"
#include "knnquery.h"
#include "searchoracle.h"
#include "method/vptree.h"
#include "method/vptree_utils.h"
#include "methodfactory.h"

#define MIN_PIVOT_SELECT_DATA_QTY 10
#define MAX_PIVOT_SELECT_ATTEMPTS 5

namespace similarity {

using std::string;
using std::stringstream;
using std::endl;
using std::cout;
using std::cerr;

const IdType PIVOT_ID_NULL_NODE = -2;
const IdType PIVOT_ID_NULL_PIVOT = -1;
const uint32_t VERSION_NUMBER = 2;

int VPtreeVisitTimes_test = 0;
int VPtreeVisitTimes_leaf = 0;
int leaf_node_number = 0;
unsigned MAX_LEVEL = 0;
int query_times = 0;
double totalSearchTime = 0;
double totalSearchTime_load = 0;
double total_vptree_merkle_node_time =0;

//std::uint8_t buffer_test[3];
//这4个变量不要紧，都是用过一次就完了，不涉及到遍历。
char node_hash_value_test[Keccak256::HASH_LEN];
char actualHash_test[Keccak256::HASH_LEN];
char dest[540];
char float_array[12];
//char buffer[3];
//std::uint8_t  internal_node_hash_value_[(Keccak256::HASH_LEN)*3];

template <typename dist_t, typename SearchOracle>
VPTree<dist_t, SearchOracle>::VPTree(
                       bool  PrintProgress,
                       Space<dist_t>& space,
                       const ObjectVector& data,
                       bool use_random_center) :
                              Index<dist_t>(data),
                              space_(space),
                              PrintProgress_(PrintProgress),
                              use_random_center_(use_random_center),
                              max_pivot_select_attempts_(MAX_PIVOT_SELECT_ATTEMPTS),
                              oracle_(space, data, PrintProgress),
                              QueryTimeParams_(oracle_.GetQueryTimeParamNames()) { 
                                QueryTimeParams_.push_back("maxLeavesToVisit");
                              }

template <typename dist_t, typename SearchOracle>
void VPTree<dist_t, SearchOracle>::CreateIndex(const AnyParams& IndexParams) {

        WallClockTimer wtmCreateIndex_vptree;
        wtmCreateIndex_vptree.reset();


  AnyParamManager pmgr(IndexParams);

  pmgr.GetParamOptional("bucketSize", BucketSize_, 50);
  pmgr.GetParamOptional("chunkBucket", ChunkBucket_, true);
  pmgr.GetParamOptional("selectPivotAttempts", max_pivot_select_attempts_, MAX_PIVOT_SELECT_ATTEMPTS);

  CHECK_MSG(max_pivot_select_attempts_ >= 1, "selectPivotAttempts should be >=1");

  LOG(LIB_INFO) << "bucketSize          = " << BucketSize_;
  LOG(LIB_INFO) << "chunkBucket         = " << ChunkBucket_;
  LOG(LIB_INFO) << "selectPivotAttempts = " << max_pivot_select_attempts_;

  // Call this function *ONLY AFTER* the bucket size is obtained!
  oracle_.SetIndexTimeParams(pmgr);
  oracle_.LogParams();

  pmgr.CheckUnused();

  this->ResetQueryTimeParams(); // reset query-time parameters

  unique_ptr<ProgressDisplay>   progress_bar(PrintProgress_ ? 
                                              new ProgressDisplay(this->data_.size(), cerr):
                                              NULL);
  //LOG(LIB_INFO) << "This is  root test ";
  root_.reset(new VPNode(//0,
                     progress_bar.get(), 
                     oracle_, 
                     space_, this->data_,
                     max_pivot_select_attempts_,
                     BucketSize_, ChunkBucket_,
                     NULL,
                     0,
                     use_random_center_ /* use random center */));


        wtmCreateIndex_vptree.split(); //构建vptree index耗费的时间。计时器。
        const double wtmCreateIndex_vptree_time  = double(wtmCreateIndex_vptree.elapsed())/1e3;
        LOG(LIB_INFO) << "wtmCreateIndex_vptree_time is          = " << wtmCreateIndex_vptree_time;
        LOG(LIB_INFO) << "total_vptree_merkle_node_time is          = " << total_vptree_merkle_node_time;


  if (progress_bar) { // make it 100%
    (*progress_bar) += (progress_bar->expected_count() - progress_bar->count());
  }


  //二、从下往上（通过递归的方法）构建整棵树的 merle hash：
  //LOG(LIB_INFO) << "Begin root Hash: ";

        WallClockTimer wtm_get_merkle_root;
        wtm_get_merkle_root.reset();

  std::uint8_t * sum_value = root_->GetHashValueTest();

            LOG(LIB_INFO) << "root Hash: ";
             for(int j = 0; j < 32; j++) {
                 printf("%02X", sum_value[j]);
                }
                printf("\n");   

        wtm_get_merkle_root.split();
        const double wtm_get_merkle_root_time  = double(wtm_get_merkle_root.elapsed())/1e3;
        LOG(LIB_INFO) << "wtm_get_merkle_root_time is          = " << wtm_get_merkle_root_time;

  // for ( int i = 0; i < 2; i++ )
  //  {
  //      cout << "*(p + " << i << ") : ";
  //      cout << *(sum_value + i) << endl;
  //  } 

  //LOG(LIB_INFO) << "sum_value is          = " << sum_value;
  //相当于这里直接获取到merkle root 



}




template <typename dist_t,typename SearchOracle>
VPTree<dist_t, SearchOracle>::~VPTree() {
}

template <typename dist_t,typename SearchOracle>
const std::string VPTree<dist_t, SearchOracle>::StrDesc() const {
  return "vptree: " + SearchOracle::GetName();
}

template <typename dist_t, typename SearchOracle>
void VPTree<dist_t, SearchOracle>::Search(RangeQuery<dist_t>* query, IdType) const {
  int mx = MaxLeavesToVisit_;
  root_->GenericSearch(query, mx);
  LOG(LIB_INFO) << "RangeQuery VPtreeVisitTimes_test          = " << VPtreeVisitTimes_test;
  VPtreeVisitTimes_test = 0;  
  VPtreeVisitTimes_leaf = 0;
}

template <typename dist_t, typename SearchOracle>
void VPTree<dist_t, SearchOracle>::Search(KNNQuery<dist_t>* query, IdType) const {
  int mx = MaxLeavesToVisit_;
   
  //SP构造VO的计时器 
        WallClockTimer wtm;
        wtm.reset();

        WallClockTimer wtm_GenericSearch;
        wtm_GenericSearch.reset();//搜索时间的计时器。
        root_->GenericSearch(query, mx);
        wtm_GenericSearch.split();//计时结束。
        const double GenericSearch_Time  = double(wtm_GenericSearch.elapsed())/1e3; //这样的结果好像已经是毫秒了 ms


  //获得vptree访问过的叶子节点的逻辑开始。
  LOG(LIB_INFO) << "KNNQuery VPtreeVisitTimes_test          = " << VPtreeVisitTimes_test;
  VPtreeVisitTimes_test = 0;  
  //VPtreeVisitTimes_leaf = 0;
  query_times++;
  root_->RecursiveToGet_VPtreeVisitTimes_leaf(); 
  root_->RecursiveToSet_if_set_node_hash();
  LOG(LIB_INFO) << "KNNQuery average VPtreeVisitTimes_leaf record          = " << VPtreeVisitTimes_leaf/query_times;
 //获得vptree访问过的叶子节点的逻辑结束。只需要注释这一小段代码就行。





  //  //SP构造VO的时间实验代码
  //  root_->RecursiveToConstructHash();//构建完整的验证merkle tree(把凑数的node找出来)
  //   LOG(LIB_INFO) << "Save index ";
  //  // //把VO数据结构存下来
  //   SaveIndexVO();

  //  root_->RecursiveToSet_if_set_node_hash(); //把节点的if_set_node_hash全部重新置为fasle

  //       wtm.split();//计时结束。
  //       const double SearchTime  = double(wtm.elapsed())/1e3; //这样的结果好像已经是毫秒了 ms
  //       totalSearchTime += SearchTime; 
  //       LOG(LIB_INFO) << ">>>> SP VO construction time:         " << totalSearchTime/(query_times*1.0);
  //       //SP构造VO结束。

 
  // LOG(LIB_INFO) << "Load index ";
  // //把VO数据结构读取出来。client验证VO的时间实验代码
  // WallClockTimer wtmload;
  // wtmload.reset();
  // LoadIndexVO();
  // //root_->GenericSearch(query, mx); //前面已经完成搜索时间计时了。GenericSearch_Time

  //       wtmload.split();//计时结束。
  //       const double SearchTimeload  = double(wtmload.elapsed())/1e3;
  //       totalSearchTime_load += SearchTimeload; //totalSearchTime_load是一个全局变量。
  //       totalSearchTime_load += GenericSearch_Time;
  //       LOG(LIB_INFO) << ">>>> User VO verification totalSearchTime_load:         " << totalSearchTime_load/(query_times*1.0);
  // //用户验证VO结束。



  //下面的代码没有用到！！！！
  // //计时器 
        // wtm.split();
        // const double SearchTime  = double(wtm.elapsed())/1e3;
        // totalSearchTime += SearchTime; 
        // LOG(LIB_INFO) << ">>>> Search time:         " << totalSearchTime/(query_times*1.0);


  //用户侧验证VO的正确性
  //root_->VerifyHashValue();


  //打印发送给用户的验证merkle tree，只需要发送对于验证树来说的叶子，给用户即可
  // for (unsigned i=MAX_LEVEL; i<=MAX_LEVEL; i++){
  // printf("level  %d  :",i);
  // root_->RecursiveToPrintHashLevel(i);
  // printf("\n");
  // }
  //root_->RecursivePrintHashTree(); 

}



template <typename dist_t, typename SearchOracle>
void VPTree<dist_t, SearchOracle>::SaveIndexVO() const{

  std::string location = "/home/ubuntu/lulingling/nmslib/similarity_search/vptreevo/vptree_test_vo"+std::to_string(query_times);
  //std::string location = "/home/ubuntu/lulingling/nmslib/similarity_search/vptreevo/vptree_test_vo1";

  std::ofstream output(location, std::ios::binary);
  CHECK_MSG(output, "Cannot open file '" + location + "' for writing");
  output.exceptions(ios::badbit | ios::failbit);

  // Save version number
  const uint32_t version = VERSION_NUMBER;
  writeBinaryPOD(output, version);

  // Save dataset size, so that we can compare with dataset size upon loading
  writeBinaryPOD(output, this->data_.size());

  // Save tree parameters
  writeBinaryPOD(output, max_pivot_select_attempts_);
  writeBinaryPOD(output, BucketSize_);
  writeBinaryPOD(output, ChunkBucket_);
  writeBinaryPOD(output, use_random_center_);


  // Save node data
  if (root_) {
    //SaveVONodeData(output, root_.get());
    SaveVONodeDataTest(output, root_.get());
  }
  output.close();
}












template <typename dist_t, typename SearchOracle>
void VPTree<dist_t, SearchOracle>::SaveIndex(const std::string& location) {

  std::ofstream output(location, std::ios::binary);
  CHECK_MSG(output, "Cannot open file '" + location + "' for writing");
  output.exceptions(ios::badbit | ios::failbit);

  // Save version number
  const uint32_t version = VERSION_NUMBER;
  writeBinaryPOD(output, version);

  // Save dataset size, so that we can compare with dataset size upon loading
  writeBinaryPOD(output, this->data_.size());

  // Save tree parameters
  writeBinaryPOD(output, max_pivot_select_attempts_);
  writeBinaryPOD(output, BucketSize_);
  writeBinaryPOD(output, ChunkBucket_); //bool
  writeBinaryPOD(output, use_random_center_);

  // Save node data
  if (root_) {
    SaveNodeData(output, root_.get());  //这里是存储ADS：merkle tree。而不是索引本身了。
  }
  output.close();
}

template <typename dist_t, typename SearchOracle>
void VPTree<dist_t, SearchOracle>::LoadIndex(const std::string& location) {

  std::ifstream input(location, std::ios::binary);
  CHECK_MSG(input, "Cannot open file '" + location + "' for reading");
  input.exceptions(ios::badbit | ios::failbit);

  uint32_t version;
  readBinaryPOD(input, version);
  if (version != VERSION_NUMBER) {
    PREPARE_RUNTIME_ERR(err) << "File version number (" << version << ") differs from "
                             << "expected version (" << VERSION_NUMBER << ")";
    THROW_RUNTIME_ERR(err);
  }

  size_t datasize;
  readBinaryPOD(input, datasize);
  if (datasize != this->data_.size()) {
    PREPARE_RUNTIME_ERR(err) << "Saved dataset size (" << datasize
                             << ") differs from actual size (" << this->data_.size() << ")";
    THROW_RUNTIME_ERR(err);
  }

  readBinaryPOD(input, max_pivot_select_attempts_);
  readBinaryPOD(input, BucketSize_);
  readBinaryPOD(input, ChunkBucket_);
  readBinaryPOD(input, use_random_center_);

  vector<IdType> IdMapper;

  CreateObjIdToPosMapper(this->data_, IdMapper);

  root_ = std::unique_ptr<VPNode>(LoadNodeData(input, ChunkBucket_, IdMapper));
}




template <typename dist_t, typename SearchOracle>
void VPTree<dist_t, SearchOracle>::LoadIndexVO() const{
  std::string location = "/home/ubuntu/lulingling/nmslib/similarity_search/vptreevo/vptree_test_vo"+std::to_string(query_times);
  //std::ifstream input("/home/ubuntu/lulingling/nmslib/similarity_search/vptreevo/vptree_test_vo1", std::ios::binary);
  std::ifstream input(location, std::ios::binary);


  //CHECK_MSG(input, "Cannot open file '" + "'/home/ubuntu/lulingling/nmslib/similarity_search/vptree_test_vo'" + "' for reading");
  CHECK_MSG(input, "Cannot open file for reading");

  input.exceptions(ios::badbit | ios::failbit);

  uint32_t version;
  readBinaryPOD(input, version);
  if (version != VERSION_NUMBER) {
    PREPARE_RUNTIME_ERR(err) << "File version number (" << version << ") differs from "
                             << "expected version (" << VERSION_NUMBER << ")";
    THROW_RUNTIME_ERR(err);
  }

  size_t datasize;
  readBinaryPOD(input, datasize);
  if (datasize != this->data_.size()) {
    PREPARE_RUNTIME_ERR(err) << "Saved dataset size (" << datasize
                             << ") differs from actual size (" << this->data_.size() << ")";
    THROW_RUNTIME_ERR(err);
  }

  readBinaryPOD(input, max_pivot_select_attempts_);
  readBinaryPOD(input, BucketSize_);
  readBinaryPOD(input, ChunkBucket_);
  readBinaryPOD(input, use_random_center_);

  vector<IdType> IdMapper;

  CreateObjIdToPosMapper(this->data_, IdMapper);

  //LOG(LIB_INFO) << "CreateObjIdToPosMapper  : ";

  unique_ptr<VPNode>  test_root_  = std::unique_ptr<VPNode>(LoadVONodeDataTest(input, ChunkBucket_, IdMapper));

   //unique_ptr<VPNode>  test_root_  = std::unique_ptr<VPNode>(LoadVONodeData(input, ChunkBucket_, IdMapper));
  std::uint8_t * test_sum_value = test_root_->GetHashValueForVOTest();
  
  //node_hash_value_;
  
            LOG(LIB_INFO) << "test root Hash: ";
             for(int j = 0; j < 32; j++) {
                 printf("%02X", test_sum_value[j]);
                }
                printf("\n"); 




}
















//这里是存储ADS：merkle tree
template <typename dist_t, typename SearchOracle>
void
VPTree<dist_t, SearchOracle>::SaveNodeData(
  std::ofstream& output,
  const typename VPTree<dist_t, SearchOracle>::VPNode* node) const {

  // Nodes are written to output in pre-order. 
  // If a node is null, we write a sentinel value instead (PIVOT_ID_NULL_NODE).
  // Regular nodes are
  // serialized in the following order:
  //
  // * pivot ID (or PIVOT_ID_NULL_NODE or PIVOT_ID_NULL_PIVOT)
  // * median distance
  // * number of bucket elements: if the bucket is empty or null, we write zero
  // * IDs of bucket data elements
  // * left child tree
  // * right child tree

  IdType pivotId = PIVOT_ID_NULL_NODE;
  if (node != nullptr) {
    pivotId = node->pivot_ ? node->pivot_->id() : PIVOT_ID_NULL_PIVOT;
  }
  //writeBinaryPOD(output, pivotId);

  if (node == nullptr) {
    return;
  }

  CHECK(node != nullptr);
  //writeBinaryPOD(output, node->mediandist_); //中间值(不需要存储)

  // size_t bucket_size = node->bucket_ ? node->bucket_->size() : 0; 
  // writeBinaryPOD(output, bucket_size); //这个好像是在存储节点数据

  memcpy(node_hash_value_test, node->node_hash_value_, Keccak256::HASH_LEN);
  writeBinaryPOD(output, node_hash_value_test); 

  // if (node->bucket_) {
  //   for (const auto& element : *(node->bucket_)) {
  //     writeBinaryPOD(output, element->id());
  //   }
  // } 



  //遍历的过程
  SaveNodeData(output, node->left_child_);
  SaveNodeData(output, node->right_child_);
}









template <typename dist_t, typename SearchOracle>
void
VPTree<dist_t, SearchOracle>::SaveVONodeDataTest(
  std::ofstream& output,
  const typename VPTree<dist_t, SearchOracle>::VPNode* node) const {

  IdType pivotId = PIVOT_ID_NULL_NODE;
  if (node != nullptr) {
    pivotId = node->pivot_ ? node->pivot_->id() : PIVOT_ID_NULL_PIVOT;
  }
  writeBinaryPOD(output, pivotId);

  if (node == nullptr) {
    return;
  }

  CHECK(node != nullptr);
  sprintf(float_array, "%f", node->mediandist_);
  writeBinaryPOD(output, float_array);
  //writeBinaryPOD(output, node->mediandist_);
  writeBinaryPOD(output, node->if_set_node_hash);

//如果是凑数的，只需要存储 node_hash。也无需遍历了。
if (node->if_set_node_hash == 2){
  size_t bucket_size = 0; 
  writeBinaryPOD(output, bucket_size); 
  memcpy(node_hash_value_test, node->node_hash_value_, Keccak256::HASH_LEN);
  writeBinaryPOD(output, node_hash_value_test); 

} else if (node->if_set_node_hash == true) {

if(node->left_child_ != NULL){ //对于非叶子节点，需要存actualHash
  size_t bucket_size =  0; 
  writeBinaryPOD(output, bucket_size); 
  memcpy(actualHash_test, node->actualHash, Keccak256::HASH_LEN); 
  writeBinaryPOD(output, actualHash_test); 
} else{ 
  //对叶子节点，需要存储bucket数据 和 node_hash_value_  ？
  size_t bucket_size = node->bucket_ ? node->bucket_->size() : 0; 
  writeBinaryPOD(output, bucket_size);
  if (node->bucket_) {
    for (const auto& element : *(node->bucket_)) {
      writeBinaryPOD(output, element->id());
    }
  } 
  //memcpy(node_hash_value_test, node->node_hash_value_, Keccak256::HASH_LEN);
  //writeBinaryPOD(output, node_hash_value_test); 
}

  if(node->left_child_ != NULL && node->if_set_node_hash == true){ 
  SaveVONodeDataTest(output, node->left_child_);
  }
  if(node->right_child_ != NULL && node->if_set_node_hash == true){
  SaveVONodeDataTest(output, node->right_child_);
  }

} 

}















template <typename dist_t, typename SearchOracle>
typename VPTree<dist_t, SearchOracle>::VPNode*
VPTree<dist_t, SearchOracle>::LoadVONodeDataTest(std::ifstream& input, bool ChunkBucket, const vector<IdType>& IdMapper) const {

  IdType pivotId = 0;
  // read one element, the pivot
  readBinaryPOD(input, pivotId);
  if (pivotId == PIVOT_ID_NULL_NODE) {  
    // empty node
    return nullptr;
  }


  VPNode* node = new VPNode(oracle_);
  if (pivotId >= 0) {
    pivotId = ConvertId(pivotId, IdMapper);
    CHECK_MSG(pivotId >= 0, "Incorrect element ID: " + ConvertToString(pivotId));
    CHECK_MSG(pivotId < this->data_.size(), "Incorrect element ID: " + ConvertToString(pivotId));
    node->pivot_ = this->data_[pivotId];
  } else {
    CHECK(pivotId == PIVOT_ID_NULL_PIVOT);
  }

  
  readBinaryPOD(input, float_array);
  //readBinaryPOD(input, mediandist);
  float mediandist = atof(float_array);

  node->mediandist_ = mediandist;
  int if_set_node_hash;
  readBinaryPOD(input, if_set_node_hash);
  node->if_set_node_hash = if_set_node_hash;
  

  size_t bucketsize;

  readBinaryPOD(input, bucketsize); 
  if (bucketsize) {
    // read bucket content
    ObjectVector bucket(bucketsize); //创建了bucket
    IdType dataId = 0;
    for (size_t i = 0; i < bucketsize; i++) {
      readBinaryPOD(input, dataId);
      CHECK(dataId >= 0);
      dataId = ConvertId(dataId, IdMapper);
      CHECK_MSG(dataId >= 0, "Incorrect element ID: " + ConvertToString(dataId));
      CHECK_MSG(dataId < this->data_.size(), "Incorrect element ID: " + ConvertToString(dataId));
      bucket[i] = this->data_[dataId];
    }
    node->CreateBucket(ChunkBucket, bucket, nullptr); //数据的指针需要再创建;ChunkBucket是bool。这里已经把数据创建好了。后面直接就可以进行hash了。
  }



  if(if_set_node_hash == 2){
    //对于凑数节点，读取node_hash_value_
  readBinaryPOD(input, node_hash_value_test);
  memcpy(node->node_hash_value_, node_hash_value_test, Keccak256::HASH_LEN);

  } else if (node->if_set_node_hash == true && bucketsize ==  0) {
    //对于非叶子节点
  readBinaryPOD(input, actualHash_test);
  memcpy(node->actualHash, actualHash_test, Keccak256::HASH_LEN);

  } else {
    //对叶子节点，读取node_hash_value_
  //readBinaryPOD(input, node_hash_value_test);
  //memcpy(node->node_hash_value_, node_hash_value_test, Keccak256::HASH_LEN);
  }


  if(if_set_node_hash == true &&  bucketsize == 0){  //对于VO tree的非叶子节点，读取他们的左右孩子。
  //LOG(LIB_INFO) << "We readBinaryPOD  left_child_.  ";
  node->left_child_ = LoadVONodeDataTest(input, ChunkBucket, IdMapper);

  //LOG(LIB_INFO) << "We readBinaryPOD  right_child_.  ";
  node->right_child_ = LoadVONodeDataTest(input, ChunkBucket, IdMapper);
  }


  return node;
}








template <typename dist_t, typename SearchOracle>
typename VPTree<dist_t, SearchOracle>::VPNode*
VPTree<dist_t, SearchOracle>::LoadNodeData(std::ifstream& input, bool ChunkBucket, const vector<IdType>& IdMapper) const {
  IdType pivotId = 0;
  // read one element, the pivot
  readBinaryPOD(input, pivotId);

  if (pivotId == PIVOT_ID_NULL_NODE) {
    // empty node
    return nullptr;
  }

  VPNode* node = new VPNode(oracle_);
  if (pivotId >= 0) {
    pivotId = ConvertId(pivotId, IdMapper);
    CHECK_MSG(pivotId >= 0, "Incorrect element ID: " + ConvertToString(pivotId));
    CHECK_MSG(pivotId < this->data_.size(), "Incorrect element ID: " + ConvertToString(pivotId));
    node->pivot_ = this->data_[pivotId];
  } else {
    CHECK(pivotId == PIVOT_ID_NULL_PIVOT);
  }

  float mediandist;
  readBinaryPOD(input, mediandist);
  node->mediandist_ = mediandist;

  size_t bucketsize;
  readBinaryPOD(input, bucketsize); //把节点数据读取进来之后

  if (bucketsize) {
    // read bucket content
    ObjectVector bucket(bucketsize);
    IdType dataId = 0;
    for (size_t i = 0; i < bucketsize; i++) {
      readBinaryPOD(input, dataId);
      CHECK(dataId >= 0);
      dataId = ConvertId(dataId, IdMapper);
      CHECK_MSG(dataId >= 0, "Incorrect element ID: " + ConvertToString(dataId));
      CHECK_MSG(dataId < this->data_.size(), "Incorrect element ID: " + ConvertToString(dataId));
      bucket[i] = this->data_[dataId];
    }
    node->CreateBucket(ChunkBucket, bucket, nullptr); //，数据的指针需要再创建;ChunkBucket是bool
  }


  node->left_child_ = LoadNodeData(input, ChunkBucket, IdMapper);

  node->right_child_ = LoadNodeData(input, ChunkBucket, IdMapper);

  return node;
}











template <typename dist_t, typename SearchOracle>
void VPTree<dist_t, SearchOracle>::VPNode::CreateBucket(bool ChunkBucket, 
                                                        const ObjectVector& data, 
                                                        ProgressDisplay* progress_bar) {
    if (ChunkBucket) {
      CreateCacheOptimizedBucket(data, CacheOptimizedBucket_, bucket_);
    } else {
      bucket_ = new ObjectVector(data);
    }
    if (progress_bar) (*progress_bar) += data.size();
}

template <typename dist_t, typename SearchOracle>
VPTree<dist_t, SearchOracle>::VPNode::VPNode(const SearchOracle& oracle)
    : oracle_(oracle),
      pivot_(NULL),
      mediandist_(0),
      left_child_(NULL),
      right_child_(NULL),
      bucket_(NULL),
      father_node_(NULL),
      if_set_node_hash(false),
      //node_hash_value_(NULL),
      //node_id_(0),
      CacheOptimizedBucket_(NULL) {}

template <typename dist_t, typename SearchOracle>
VPTree<dist_t, SearchOracle>::VPNode::VPNode(
                               //unsigned level,
                               ProgressDisplay* progress_bar,
                               const SearchOracle& oracle,
                               const Space<dist_t>& space, const ObjectVector& data,
                               size_t max_pivot_select_attempts,
                               size_t BucketSize, bool ChunkBucket,
                               VPNode* father_node_point_,
                               //int node_id_,
                               unsigned level,
                               bool use_random_center)
    : oracle_(oracle),
      pivot_(NULL), mediandist_(0),
      left_child_(NULL), right_child_(NULL), father_node_(father_node_point_),
      level(level),
      if_set_node_hash(false),
      //node_hash_value_(0),
      bucket_(NULL), CacheOptimizedBucket_(NULL)
{
  CHECK(!data.empty());

  if (!data.empty() && data.size() <= BucketSize) {//如果是叶子节点
        WallClockTimer wtm_node_hash;  //对叶子节点进行hash的时间。计时器。
        wtm_node_hash.reset();

    CreateBucket(ChunkBucket, data, progress_bar);
    Keccak256::getHash(  (uint8_t *)CacheOptimizedBucket_, TotalSpaceUsed(data), node_hash_value_);
    //获得叶子节点的node_hash_value_

        wtm_node_hash.split();
        const double SearchTime_node_hash  = double(wtm_node_hash.elapsed())/1e3;
        total_vptree_merkle_node_time += SearchTime_node_hash;  //对叶子节点进行hash的时间
    //leaf_node_number+=data.size();
    return;
  }

  if (level > MAX_LEVEL){
    MAX_LEVEL = level;
    //LOG(LIB_INFO) << " VPNode MAX_LEVEL:   "<< MAX_LEVEL ;
  }


  if (data.size() >= 2) {
    unsigned bestDP = 0;
    float    largestSIGMA = 0;
    vector<DistObjectPairVector<dist_t>> dpARR(max_pivot_select_attempts);

    // To compute StdDev we need at least 2 points not counting the pivot
    size_t maxAtt = data.size() >= max(3, MIN_PIVOT_SELECT_DATA_QTY) ? max_pivot_select_attempts : 1;

    vector<double> dists(data.size());
    for (size_t att = 0; att < maxAtt; ++att) {
      dpARR[att].reserve(data.size());
      const size_t  currPivotIndex = SelectVantagePoint(data, use_random_center);
      const Object* pCurrPivot = data[currPivotIndex];
      for (size_t i = 0; i < data.size(); ++i) {
        if (i == currPivotIndex) {
          continue;
        }
        // Distance can be asymmetric, the pivot is always on the left side!
        dist_t d = space.IndexTimeDistance(pCurrPivot, data[i]);
        dists[i] = d;
        dpARR[att].emplace_back(d, data[i]);
      }

      double sigma = StdDev(&dists[0], dists.size());
      if (att == 0 || sigma > largestSIGMA) {
        //LOG(LIB_INFO) << " ### " << largestSIGMA << " -> "  << sigma << " att=" << att << " data.size()=" << data.size();
        largestSIGMA = sigma;
        bestDP = att;
        pivot_ = pCurrPivot;
        std::sort(dpARR[att].begin(), dpARR[att].end(), DistObjectPairAscComparator<dist_t>());
      }
    }


    DistObjectPairVector<dist_t>& dp = dpARR[bestDP];
    DistObjectPair<dist_t>  medianDistObj = GetMedian(dp);
    mediandist_ = medianDistObj.first; 

    ObjectVector left;
    ObjectVector right;

    for (auto it = dp.begin(); it != dp.end(); ++it) {
      const Object* v = it->second;

      /* 
       * Note that here we compare a pair (distance, pointer)
       * If distances are equal, pointers are compared.
       * Thus, we would get a balanced split, even if the median
       * occurs many times in the array dp[].
       */
      if (*it < medianDistObj) {
        left.push_back(v);
      } else {
        right.push_back(v);
      }
    }

    /*
     * Sometimes, e.g.., for integer-valued distances,
     * mediandist_ will be non-discriminative. In this case
     * it is more efficient to put everything into a single bucket.
     */
    size_t LeastSize = dp.size() / BalanceConst;

        WallClockTimer wtm_node_actualHash;
        wtm_node_actualHash.reset();
		
	sprintf(float_array, "%f", mediandist_);
  //char dest[540];
  memcpy(dest, pivot_->buffer(), 528);
      // LOG(LIB_INFO) << "pivot_->buffer(): ";
      //        for(int j = 0; j < 528; j++) {
      //            printf("%02X", pivot_->buffer()[j]);
      //           }
      //           printf("\n"); 
      // LOG(LIB_INFO) << "dest after memcpy pivot: ";
      //        for(int j = 0; j < 540; j++) {
      //            printf("%02X", dest[j]);
      //           }
      //           printf("\n");   
  memcpy(dest+528, float_array, 12);
  //打印hash结果
  Keccak256::getHash(  (uint8_t *)dest, 540, actualHash);
  //LOG(LIB_INFO) << "pivot_->bufferlength()         = " << pivot_->bufferlength();//都是528  
        wtm_node_actualHash.split();
        const double SearchTime_wtm_node_actualHash  = double(wtm_node_actualHash.elapsed())/1e3;
        total_vptree_merkle_node_time += SearchTime_wtm_node_actualHash; //加上，对中间节点进行hash的时间。





    if (left.size() < LeastSize || right.size() < LeastSize) {
        CreateBucket(ChunkBucket, data, progress_bar);
        return;
    }

    if (!left.empty()) {
      //LOG(LIB_INFO) << "Construct left_child_ "<< this; 左孩子
      left_child_ = new VPNode(progress_bar, oracle_, space, left, max_pivot_select_attempts, BucketSize, ChunkBucket, this, level + 1, use_random_center);
    }

    if (!right.empty()) {
      //LOG(LIB_INFO) << "Construct right_child_ "<< this; 右孩子
      right_child_ = new VPNode(progress_bar, oracle_, space, right, max_pivot_select_attempts, BucketSize, ChunkBucket, this, level + 1, use_random_center);
    }
 
  } else {
    LOG(LIB_INFO) << "We have a leaf node:  "<< this;
    CHECK_MSG(data.size() == 1, "Bug: expect the subset to contain exactly one element!");
    //这个分支里的逻辑根本没有用到。
    pivot_ = data[0];
  }
   
 
}

template <typename dist_t, typename SearchOracle>
VPTree<dist_t, SearchOracle>::VPNode::~VPNode() {
  delete left_child_;
  delete right_child_;
  delete father_node_;
  ClearBucket(CacheOptimizedBucket_, bucket_);
}

template <typename dist_t, typename SearchOracle>
template <typename QueryType>
void VPTree<dist_t, SearchOracle>::VPNode::GenericSearch(QueryType* query,
                                                         int& MaxLeavesToVisit) {



  if (MaxLeavesToVisit <= 0) return; // early termination

   VPtreeVisitTimes_test++; //每查询一次就自增一次。
  //LOG(LIB_INFO) << "GenericSearch VPtreeVisitTimes_test          = " << VPtreeVisitTimes_test;
  if_set_node_hash = true;
  if (bucket_) { //如果是叶子节点
    //VPtreeVisitTimes_leaf++;

    --MaxLeavesToVisit;

    if (CacheOptimizedBucket_) {
      PREFETCH(CacheOptimizedBucket_, _MM_HINT_T0);
    }

    for (unsigned i = 0; i < bucket_->size(); ++i) {
      const Object* Obj = (*bucket_)[i];
      dist_t distQC = query->DistanceObjLeft(Obj);
      query->CheckAndAddToResult(distQC, Obj);
      //VPtreeVisitTimes_leaf++;
    }
    return;
  }

  // Distance can be asymmetric, the pivot is always the left argument (see the function that creates the node)!
  dist_t distQC = query->DistanceObjLeft(pivot_);
  query->CheckAndAddToResult(distQC, pivot_);

  if (distQC < mediandist_) {      // the query is inside
    // then first check inside
    if (left_child_ != NULL && oracle_.Classify(distQC, query->Radius(), mediandist_) != kVisitRight)
       left_child_->GenericSearch(query, MaxLeavesToVisit);

    /* 
     * After potentially visiting the left child, we need to reclassify the node,
     * because the query radius might have decreased.
     */



    // after that outside
    if (right_child_ != NULL && oracle_.Classify(distQC, query->Radius(), mediandist_) != kVisitLeft)
       right_child_->GenericSearch(query, MaxLeavesToVisit);
  } else {                         // the query is outside
    // then first check outside
    if (right_child_ != NULL && oracle_.Classify(distQC, query->Radius(), mediandist_) != kVisitLeft)
       right_child_->GenericSearch(query, MaxLeavesToVisit);

    /* 
     * After potentially visiting the left child, we need to reclassify the node,
     * because the query radius might have decreased.
     */

    // after that inside
    if (left_child_ != NULL && oracle_.Classify(distQC, query->Radius(), mediandist_) != kVisitRight)
      left_child_->GenericSearch(query, MaxLeavesToVisit);
  }
}








template <typename dist_t, typename SearchOracle>
//用递归的方法，从下往上构建merkle root
std::uint8_t * VPTree<dist_t, SearchOracle>::VPNode::GetHashValueTest() {

  //如果节点是叶子节点。actualHash就是node_hash_value_。
  if(left_child_ == NULL){

    if(right_child_ != NULL){
    LOG(LIB_INFO) << " VPNode only have  right_child_";
  }
  }
  if(right_child_ == NULL){
    if(left_child_ != NULL){
    LOG(LIB_INFO) << " VPNode only have  left_child_";
  }
  } else if (left_child_ != NULL  && right_child_ != NULL){
    //如果是非叶子节点：node_hash_value_= 左孩子node_hash_value_+右孩子+自身hash（至高点+中间值）
    //测试第一个非叶子节点。

    std::uint8_t  internal_node_hash_value_test[(Keccak256::HASH_LEN)*3];

    memcpy(internal_node_hash_value_test, left_child_->GetHashValueTest(), Keccak256::HASH_LEN);
    memcpy(internal_node_hash_value_test+ Keccak256::HASH_LEN, right_child_->GetHashValueTest(), Keccak256::HASH_LEN);
    memcpy(internal_node_hash_value_test+ Keccak256::HASH_LEN + Keccak256::HASH_LEN, actualHash, Keccak256::HASH_LEN);
    Keccak256::getHash(internal_node_hash_value_test, (Keccak256::HASH_LEN)*3, node_hash_value_);
    //打印非叶子节点hash
    // LOG(LIB_INFO) << "Internal Node Hash: " << this <<"  :";
    //          for(int j = 0; j < 32; j++) {
    //              printf("%02X", node_hash_value_[j]);
    //             }
    //             printf("\n"); 



  } 
  return node_hash_value_; 

  }









template <typename dist_t, typename SearchOracle>
//用递归的方法，从下往上构建merkle root。逻辑上参考GetHashValue()
std::uint8_t * VPTree<dist_t, SearchOracle>::VPNode::GetHashValueForVOTest() {
  bool if_error = false;
  //如果节点是叶子节点。简化版，我们是直接存储了node_hash_value_（测试版首先比较一下是否一致）。我们直接通过节点构建hash。
  if (left_child_ == NULL  && right_child_ == NULL &&  if_set_node_hash == true){
    //比较一下是否一致
    //std::uint8_t  node_hash_value_test_[Keccak256::HASH_LEN];  
    Keccak256::getHash(  (uint8_t *)CacheOptimizedBucket_, TotalSpaceUsed(*bucket_), node_hash_value_);
    // for(int i=0;i<Keccak256::HASH_LEN;i++){
    //   if(node_hash_value_test_[i] != node_hash_value_[i]){
    //     if_error = true; 
    //   }
    // }

    // if(if_error){
    //   if_error = false;
    //   printf("node_hash_value_ error!!!!! \n"); 

    //   LOG(LIB_INFO) << "node_hash_value_ from store: ";
    //              for(int j = 0; j < 32; j++) {
    //              printf("%02X", node_hash_value_[j]);
    //             }
    //             printf("\n"); 
                
    // }

     //memcpy(node_hash_value_,node_hash_value_test_,Keccak256::HASH_LEN);
    //LOG(LIB_INFO) << "Finish Hash : ";
    //return node_hash_value_test_;

  }



  //或者如果是凑数节点 直接存储了node_hash_value_。



  if (left_child_ != NULL  && right_child_ != NULL &&  if_set_node_hash == true){
    //如果是非叶子节点：node_hash_value_= 左孩子+右孩子+自身hash（至高点+中间值）
    std::uint8_t  internal_node_hash_value_[(Keccak256::HASH_LEN)*3];

    memcpy(internal_node_hash_value_, left_child_->GetHashValueForVOTest(), Keccak256::HASH_LEN);
    //LOG(LIB_INFO) << "right_child_->VOTest-> " << this <<"  :";
    memcpy(internal_node_hash_value_+ Keccak256::HASH_LEN, right_child_->GetHashValueForVOTest(), Keccak256::HASH_LEN);

    //LOG(LIB_INFO) << "actualHash: "<< this <<"  :";
    memcpy(internal_node_hash_value_+ Keccak256::HASH_LEN + Keccak256::HASH_LEN, actualHash, Keccak256::HASH_LEN);

    std::uint8_t  actualHash_test_[Keccak256::HASH_LEN];    

	  sprintf(float_array, "%f", mediandist_);
    //char dest_test[540];
    memcpy(dest, pivot_->buffer(), 528);
    memcpy(dest+528, float_array, 12);
    Keccak256::getHash(  (uint8_t *)dest, 540, actualHash_test_);
 
    //首先比较一下中间节点的actualHash_test_是否一致。
    
    for(int i=0;i<Keccak256::HASH_LEN;i++){
      if(actualHash[i] != actualHash_test_[i]){
        if_error = true; 
      }
    }

    if(if_error){
      if_error = false;
      printf("actualHash error!!!!! \n"); 

      LOG(LIB_INFO) << "actualHash from store: ";
                 for(int j = 0; j < 32; j++) {
                 printf("%02X", actualHash[j]);
                }
                printf("\n"); 
                
    }
    //构建中间节点的node_hash_value_
    Keccak256::getHash(internal_node_hash_value_, (Keccak256::HASH_LEN)*3, node_hash_value_);
  }


  return node_hash_value_;
  }























template <typename dist_t, typename SearchOracle>
//template <typename QueryType>
void VPTree<dist_t, SearchOracle>::VPNode::RecursiveToPrintHashLevel(unsigned i) {
  //用这个函数把hash tree打印出来？

   if(left_child_ == NULL){
    if (if_set_node_hash && level == i){
             for(int j = 0; j < 1; j++) {
                 printf("%02X", node_hash_value_[j]);
                }
                printf("----");
    }
  }
  else if(left_child_ != NULL  && right_child_ != NULL){
    if(if_set_node_hash && level == i){
            //  for(int j = 0; j < 1; j++) {
            //      printf("%02X", node_hash_value_[j]);
            //     }
    if(left_child_->if_set_node_hash == false && right_child_->if_set_node_hash == false){
             printf("LeafO----");
             } 
             else{
             printf("O----");
             }                
    }
    if(level < i){
    left_child_->RecursiveToPrintHashLevel(i);
    right_child_->RecursiveToPrintHashLevel(i);
    }

  }
}


template <typename dist_t, typename SearchOracle>
//template <typename QueryType>
void VPTree<dist_t, SearchOracle>::VPNode::RecursiveToConstructHash() {

//用户没有访问过，但是为了构造merkle tree必须发送给用户的VO
if(if_set_node_hash && father_node_ !=NULL){

if (father_node_->left_child_->if_set_node_hash == false){
  //用户没有访问过，但是为了构造merkle tree必须发送给用户的VO
  father_node_->left_child_->if_set_node_hash = 2;
}
if (father_node_->right_child_->if_set_node_hash == false){
  //用户没有访问过，但是为了构造merkle tree必须发送给用户的VO
  father_node_->right_child_->if_set_node_hash = 2;
}

}
 //继续遍历过程。
 if(left_child_ != NULL  && right_child_ != NULL){
    left_child_->RecursiveToConstructHash();
    right_child_->RecursiveToConstructHash();
  }
}



template <typename dist_t, typename SearchOracle>
//template <typename QueryType>
void VPTree<dist_t, SearchOracle>::VPNode::RecursiveToSet_if_set_node_hash() {

if(if_set_node_hash != false){
   if_set_node_hash = false;
}
 //继续遍历过程。
 if(left_child_ != NULL  && right_child_ != NULL){
      //如果是中间节点
      //LOG(LIB_INFO) << "actualHash from store: ";
      //            for(int j = 0; j < 32; j++) {
      //            printf("%02X", actualHash[j]);
      //           }
      //           printf("\n");  
      // LOG(LIB_INFO) << "This is Recursive mediandist_ :  "<< mediandist_;          
      // LOG(LIB_INFO) << "pivot_->buffer() from store: ";
      //            for(int j = 0; j < 32; j++) {
      //            printf("%02X", pivot_->buffer()[j]);
      //           }
      //           printf("\n");  


    left_child_->RecursiveToSet_if_set_node_hash();
    right_child_->RecursiveToSet_if_set_node_hash();
  }
}


template <typename dist_t, typename SearchOracle>
//template <typename QueryType>
void VPTree<dist_t, SearchOracle>::VPNode::RecursiveToGet_VPtreeVisitTimes_leaf() {

if(if_set_node_hash==true && left_child_ == NULL && right_child_== NULL){
  //LOG(LIB_INFO) << "bucket_->size(): "<< bucket_->size();
  VPtreeVisitTimes_leaf+= bucket_->size();
}

 //遍历过程。
 if(left_child_ != NULL  && right_child_ != NULL){
    left_child_->RecursiveToGet_VPtreeVisitTimes_leaf();
    right_child_->RecursiveToGet_VPtreeVisitTimes_leaf();
  }


}




template <typename dist_t, typename SearchOracle>
//template <typename QueryType>
int VPTree<dist_t, SearchOracle>::VPNode::RecursivePrintHashTree() {
   if(left_child_ == NULL){
    if (if_set_node_hash){
      LOG(LIB_INFO) << "This is hash in level :  "<< level;
             for(int j = 0; j < 32; j++) {
                 printf("%02X", node_hash_value_[j]);
                }
                printf("\n");        
      return 1;
    }
    return 0;
  }
  else if(left_child_ != NULL  && right_child_ != NULL){
    int left = left_child_->RecursivePrintHashTree();
    int right = right_child_->RecursivePrintHashTree();
  
   if (left==0 && right==0 ){
    if (if_set_node_hash){
      LOG(LIB_INFO) << "This is internal hash in level :  "<< level;
             for(int j = 0; j < 32; j++) {
                 printf("%02X", node_hash_value_[j]);
                }
                printf("\n");        
      return 1;      
    }
    return 0;
   }
   return 1;   
  }
}






template class VPTree<float, PolynomialPruner<float> >;
template class VPTree<int, PolynomialPruner<int> >;

// namespace similarity
}
