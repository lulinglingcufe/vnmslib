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
#ifndef _VPTREE_H_
#define _VPTREE_H_

#include <string>
#include <vector>
#include <memory>

#include "index.h"
#include "object.h"
#include "params.h"
#include "ported_boost_progress.h"
#include "Keccak256.h"


#define METH_VPTREE          "vptree"

namespace similarity {

using std::string;
using std::vector;
using std::unique_ptr;

// Vantage point tree

template <typename dist_t> class Space;

template <typename dist_t, typename SearchOracle>
class VPTree : public Index<dist_t> {
 public:
  VPTree(bool PrintProgress,
         Space<dist_t>& space,
         const ObjectVector& data,
         //ObjectVector& data,
         bool use_random_center = true);

  void CreateIndex(const AnyParams& IndexParams) override;
  
  //void GenericConstructHash(VPNode* begin_pointer) const;                  

  ~VPTree();

  const std::string StrDesc() const override;

  void SaveIndex(const string& location) override;
  void LoadIndex(const string& location) override;

  void SaveIndexVO() const;
  void LoadIndexVO() const;

  void Search(RangeQuery<dist_t>* query, IdType) const override;
  void Search(KNNQuery<dist_t>* query, IdType) const override;

  const vector<string>& getQueryTimeParams() const { return QueryTimeParams_; }


  void SetQueryTimeParams(const AnyParams& QueryTimeParams) override {
    AnyParamManager pmgr(QueryTimeParams);
    oracle_.SetQueryTimeParams(pmgr); 
    pmgr.GetParamOptional("maxLeavesToVisit", MaxLeavesToVisit_, FAKE_MAX_LEAVES_TO_VISIT);
    LOG(LIB_INFO) << "Set VP-tree query-time parameters:";
    LOG(LIB_INFO) << "maxLeavesToVisit=" << MaxLeavesToVisit_;
    pmgr.CheckUnused();
  }

  virtual bool DuplicateData() const override { return ChunkBucket_; }
 private:

  class VPNode {
   public:
    // We want trees to be balanced
    const size_t BalanceConst = 4; 

    VPNode(const SearchOracle& oracle);
    VPNode(//unsigned level,
           ProgressDisplay* progress_bar,
           const SearchOracle&  oracle,
           const Space<dist_t>& space, const ObjectVector& data,
           size_t max_pivot_select_attempts,
           size_t BucketSize, bool ChunkBucket,
           VPNode* father_node_point_,
           unsigned level,
           //int node_id_,
           bool use_random_center);
    ~VPNode();

    template <typename QueryType>
    void GenericSearch(QueryType* query, int& MaxLeavesToVisit); //const;

    //void GenericConstructHash(); 
    //std::uint8_t * GetHashValue();  
    //std::uint8_t * VerifyHashValue(); 
    std::uint8_t * GetHashValueTest(); 

    //std::uint8_t * GetHashValueForVO();
    std::uint8_t * GetHashValueForVOTest();
    
    std::uint8_t  node_hash_value_[Keccak256::HASH_LEN];
    std::uint8_t  actualHash[Keccak256::HASH_LEN];
    int if_set_node_hash; //是否在查找的过程中，使用了这个节点
    void RecursiveToPrintHashLevel(unsigned i); 
    void RecursiveToConstructHash(); 
    int RecursivePrintHashTree(); 
    void RecursiveToSet_if_set_node_hash(); 
    void RecursiveToGet_VPtreeVisitTimes_leaf(); 

   private:
    void CreateBucket(bool ChunkBucket, const ObjectVector& data, 
                      ProgressDisplay* progress_bar);

    const SearchOracle& oracle_; // The search oracle must be accessed by reference,
                                 // so that VP-tree may be able to change its parameters
    const Object* pivot_;
    /* 
     * Even if dist_t is double, or long double
     * storing the median as the single-precision number (i.e., float)
     * should be good enough.
     */
    float         mediandist_;
    // std::uint8_t  node_hash_value_[Keccak256::HASH_LEN];
    // std::uint8_t  actualHash[Keccak256::HASH_LEN];
    // std::uint8_t  internal_node_Hash[Keccak256::HASH_LEN];
    //int           node_id_;
    unsigned level;
    VPNode*       left_child_;
    VPNode*       right_child_;
    VPNode*       father_node_;
    ObjectVector* bucket_;
    char*         CacheOptimizedBucket_;

    friend class VPTree;
  };



  Space<dist_t>&      space_;
  bool                PrintProgress_;
  bool                use_random_center_;
  size_t              max_pivot_select_attempts_;

  SearchOracle        oracle_;
  unique_ptr<VPNode>  root_;
  size_t              BucketSize_;
  int                 MaxLeavesToVisit_;
  bool                ChunkBucket_;

  vector<string>  QueryTimeParams_;

  VPNode* LoadNodeData(std::ifstream& input, bool ChunkBucket, const vector<IdType>& IdMapper) const;


  //VPNode* LoadVONodeData(std::ifstream& input, bool ChunkBucket, const vector<IdType>& IdMapper) const;
  VPNode* LoadVONodeDataTest(std::ifstream& input, bool ChunkBucket, const vector<IdType>& IdMapper) const;
  //VPNode* LoadVONodeData(std::ifstream& input, bool ChunkBucket, vector<IdType>& IdMapper);


  void    SaveNodeData(std::ofstream& output, const VPNode* node) const;

  
  //void    SaveVONodeData(std::ofstream& output, const VPNode* node) const;
  void    SaveVONodeDataTest(std::ofstream& output, const VPNode* node) const;

  // disable copy and assign
  DISABLE_COPY_AND_ASSIGN(VPTree);
};


}   // namespace similarity

#endif
