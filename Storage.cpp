//
//  Storage.cpp
//  RGAssignment2
//
//  Created by rick gessner on 2/27/21.
//


#include <sstream>
#include <cmath>
#include <cstdlib>
#include <optional>
#include <cstring>
#include "Storage.hpp"
#include "Config.hpp"

namespace ECE141 {

  Storage::Storage(std::iostream &aStream) : BlockIO(aStream) {
  }

  Storage::~Storage() {
  }

  bool Storage::each(const BlockVisitor &aVisitor) {
      int blockCount = getBlockCount();
      Block theBlock;
      
      for (int i = 0; i < blockCount; ++i) {
          readBlock(i, theBlock);
          if (!aVisitor(theBlock, i))
              break; //break if target block reached
      }

      return true;
  }

  uint32_t Storage::getNextFreeBlockNum() {
      uint32_t res = getBlockCount();

      if (!available.empty()) { //if there are any empty blocks
          res = *available.begin();
      }

      return res;
  }
  
  uint32_t Storage::getFreeBlock() {
      uint32_t res = getBlockCount();

      if (!available.empty()) { //pop an empty block number if there are any
          res = *available.begin();
          available.erase(res);          
      }

      if (available.empty()) { //put the next empty block into available
          available.insert(getBlockCount() + 1);
      }

      return res;
  }

  StatusResult Storage::markBlockAsFree(uint32_t aPos) {
      return releaseBlocks(aPos);
  }

  StatusResult Storage::releaseBlocks(uint32_t aPos,bool aInclusive) {
      Block theBlock;
      Block freeBlock(BlockType::free_block);
      bool more = true;

      while (more) {
          //read the block and get next block number
          readBlock(aPos, theBlock);
          auto nextPos = theBlock.header.next;
          
          //overwrite the block as free
          writeBlock(aPos, freeBlock);

          //insert into available list
          available.insert(aPos);

          aPos = nextPos;
          more = aPos; //if aPos = 0 implies no next block
      }
      
      return StatusResult{Errors::noError};
  }

  StatusResult Storage::save(std::iostream &aStream, StorageInfo &anInfo) {      
      size_t streamSize = anInfo.size;

      size_t blockIndex = 0;
      if (anInfo.start == kNewBlock)
          blockIndex = getFreeBlock();
      else
          blockIndex = anInfo.start;

      size_t pos = 0;
      size_t blockCount = anInfo.size / kPayloadSize;
      if (anInfo.size % kPayloadSize) //one more block required
          ++blockCount;

      while (pos < blockCount) {
          //set up block
          Block theBlock(anInfo.type);
          theBlock.header = BlockHeader(anInfo.type);
          theBlock.header.pos = pos;
          theBlock.header.count = blockCount;
          theBlock.header.refId = anInfo.refId;
          theBlock.header.next = available.size() > 0 ? *available.begin() : getBlockCount();
          theBlock.header.size = streamSize >= kPayloadSize ? kPayloadSize : streamSize;
          theBlock.header.id = anInfo.id;

          if (pos + 1 == blockCount) { //no next block 
              available.insert(theBlock.header.next);
              theBlock.header.next = 0;
          }

          if (streamSize >= kPayloadSize)
              aStream.read(theBlock.payload, kPayloadSize);
          else
              aStream.read(theBlock.payload, streamSize);

          writeBlock(blockIndex, theBlock);

          streamSize -= kPayloadSize;
          ++pos;
          blockIndex = theBlock.header.next;
      }
            
      return StatusResult{Errors::noError};
  }

  StatusResult Storage::load(std::iostream & aStream, uint32_t aBlockNum) {      
      Block theBlock;
      bool more = true;

      while (more) {
          readBlock(aBlockNum, theBlock);
          aStream.write(theBlock.payload, theBlock.header.size);
          aBlockNum = theBlock.header.next;
          more = aBlockNum; //if aBlockNum = 0 implies no next block
      }

      return StatusResult{Errors::noError};
  }

}