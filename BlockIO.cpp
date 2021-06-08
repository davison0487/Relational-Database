//
//  BlockIO.cpp
//  RGAssignment2
//
//  Created by rick gessner on 2/27/21.
//

#include "BlockIO.hpp"

namespace ECE141 {

    Block::Block(BlockType aType) : header(BlockHeader{ aType }) {
        for (int i = 0; i < kPayloadSize; ++i)
            payload[i] = 0;
    }

  Block::Block(const Block &aCopy) {
      this->header = aCopy.header;
      for (int i = 0; i < kPayloadSize; ++i)
          payload[i] = aCopy.payload[i];
  }

  Block& Block::operator=(const Block &aCopy) {
      this->header = aCopy.header;
      for (int i = 0; i < kPayloadSize; ++i)
          payload[i] = aCopy.payload[i];
      
      return *this;
  }

  StatusResult Block::write(std::ostream &aStream) {
    return StatusResult{Errors::noError};
  }

  //---------------------------------------------------

  BlockIO::BlockIO(std::iostream &aStream) : stream(aStream) {}

  StatusResult write(Block &aBlock, std::iostream &aStream, size_t aBlockSize) {
    if(aStream.write ((char*)&aBlock, aBlockSize)) {
      aStream.flush();
      return StatusResult{noError};
    }
    return StatusResult{writeError};
  }

  // USE: write data a given block (after seek) ---------------------------------------
  StatusResult BlockIO::writeBlock(uint32_t aBlockNum, Block &aBlock) {
    static size_t theSize=sizeof(aBlock);
    stream.seekg(stream.tellg(), std::ios::beg); //sync buffers...
    stream.seekp(aBlockNum * theSize);
    return write(aBlock, stream, theSize);
  }

  // USE: write data a given block (after seek) ---------------------------------------
  StatusResult BlockIO::readBlock(uint32_t aBlockNumber, Block &aBlock) {
    static size_t theSize=sizeof(aBlock);
    stream.seekg(aBlockNumber * theSize);
    //size_t thePos=stream.tellg();
    if(!stream.read ((char*)&aBlock, theSize)) {
      return StatusResult(readError);
    }
    return StatusResult{noError};
  }

  // USE: count blocks in file ---------------------------------------
  uint32_t BlockIO::getBlockCount() {
    stream.seekg(stream.tellg(), std::ios::beg); //force read mode; dumb c++ issue...
    stream.seekg(0, std::ios::end);
    int thePos = (int)stream.tellg();
    return thePos / sizeof(Block);
  }

}
