//
//  BlockIO.hpp
//  RGAssignment2
//
//  Created by rick gessner on 2/27/21.
//

#ifndef BlockIO_hpp
#define BlockIO_hpp

#include <stdio.h>
#include <iostream>
#include "Errors.hpp"

namespace ECE141 {

  enum class BlockType {
    meta_block='M',
    data_block='D',
    entity_block='E',
    free_block='F',
    index_block='I',
    unknown_block='U',
  };

  //a small header that describes the block...
  struct BlockHeader {
   
    BlockHeader(BlockType aType=BlockType::data_block)
      : type(static_cast<char>(aType)), count(1),
        pos(1), next(0), refId(0), id(0), size(0){}

    BlockHeader(const BlockHeader &aCopy) {
      *this=aCopy;
    }
        
    BlockHeader& operator=(const BlockHeader &aCopy) {
      type=aCopy.type;
      pos=aCopy.pos;
      count=aCopy.count;
      refId=aCopy.refId;
      id=aCopy.id;
      next=aCopy.next;
      size = aCopy.size;
      return *this;
    }
   
    char      type;     //char version of block type
    uint8_t   count;    //how many parts
    uint8_t   pos;      //i of n...
    uint32_t  next;     //next block in sequence.
    uint32_t  refId;    //e.g. id of thing it refers to
    uint32_t  id;       //use this anyway you like
    uint32_t  size;     //valid payload size
  };

  const size_t kBlockSize = 1024;
  const size_t kPayloadSize = kBlockSize - sizeof(BlockHeader);
  

  //block .................
  class Block {
  public:
      Block(BlockType aType=BlockType::data_block);
      Block(const Block &aCopy);
    
    Block& operator=(const Block &aCopy);
   
    StatusResult write(std::ostream &aStream);

    //we use attributes[0] as table name...
    BlockHeader   header;
    char          payload[kPayloadSize];
  };

  //blockIO............
  class BlockIO {
  public:
    
    BlockIO(std::iostream &aStream);
    
    uint32_t              getBlockCount();
    bool                  isReady() const;
    
    virtual StatusResult  readBlock(uint32_t aBlockNumber,
                                    Block &aBlock);
    virtual StatusResult  writeBlock(uint32_t aBlockNumber,
                                     Block &aBlock);
    
  protected:
    std::iostream &stream;
  };

}


#endif /* BlockIO_hpp */
