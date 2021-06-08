//
//  Storage.hpp
//  RGAssignment2
//
//  Created by rick gessner on 2/27/21.
//

#ifndef Storage_hpp
#define Storage_hpp

#include <string>
#include <fstream>
#include <iostream>
#include <deque>
#include <set>
#include <functional>
#include "BlockIO.hpp"
#include "Errors.hpp"

namespace ECE141 {
      
  struct CreateDB {}; //tags for db-open modes...
  struct OpenDB {};

  const int32_t kNewBlock=-1;

  //using BlockList = std::deque<uint32_t>;
  using BlockList = std::set<uint32_t>;
  using BlockVisitor = std::function<bool(const Block&, uint32_t)>;

  struct BlockIterator {
      virtual bool each(const BlockVisitor& aVisitor) = 0;
  };

  class Storable {
  public:
    virtual StatusResult  encode(std::ostream &anOutput)=0;
    virtual StatusResult  decode(std::istream &anInput)=0;
  };

  struct StorageInfo {
    
    StorageInfo(size_t aRefId, size_t theSize, int32_t aStartPos=kNewBlock, BlockType aType=BlockType::data_block, size_t anID = 0)
      : type(aType), start(aStartPos), refId(aRefId), size(theSize), id(anID) {}
    
    BlockType type;
    int32_t   start; //block#
    size_t    refId;
    size_t    size;
    size_t    id;
  };
  
  class Storage : public BlockIO, BlockIterator {
  public:
        
    Storage(std::iostream &aStream);
    ~Storage();
 
    StatusResult save(std::iostream &aStream, StorageInfo &anInfo);
    StatusResult load(std::iostream &aStream, uint32_t aStartBlockNum);

    virtual bool each(const BlockVisitor& aVisitor);

    StatusResult markBlockAsFree(uint32_t aPos);
    
    //get next free block number, but leave it in the list
    uint32_t     getNextFreeBlockNum();

  protected:
  
    StatusResult releaseBlocks(uint32_t aPos, bool aInclusive=false);
    
    //get next free block number and take it out of list
    uint32_t     getFreeBlock(); //pos of next free (or new)...
                
    BlockList    available;
  };

}


#endif /* Storage_hpp */
