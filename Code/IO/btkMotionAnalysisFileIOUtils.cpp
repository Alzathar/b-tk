/** 
 * The Biomechanical ToolKit
 * Copyright (c) 2009-2012, Arnaud Barré
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 
 *     * Redistributions of source code must retain the above
 *       copyright notice, this list of conditions and the following
 *       disclaimer.
 *     * Redistributions in binary form must reproduce the above
 *       copyright notice, this list of conditions and the following
 *       disclaimer in the documentation and/or other materials
 *       provided with the distribution.
 *     * Neither the name(s) of the copyright holders nor the names
 *       of its contributors may be used to endorse or promote products
 *       derived from this software without specific prior written
 *       permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 * ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#include "btkMotionAnalysisFileIOUtils.h"
#include "btkConvert.h"

#include <algorithm>
#include <cctype>

namespace btk
{  
  /**
   * @class MotionAnalysisBinaryFileIOException
   * @brief Exception class for the MotionAnalysisBinaryFileIO class.
   */

  /**
   * @fn MotionAnalysisBinaryFileIOException::MotionAnalysisBinaryFileIOException(const std::string& msg)
   * Constructor.
   */

  /**
   * @fn virtual MotionAnalysisBinaryFileIOException::~MotionAnalysisBinaryFileIOException()
   * Empty destructor.
   */

  // -------------------------------------------------------------------- // 
  //                         ANB/TRB util methods                         //
  // -------------------------------------------------------------------- // 

  MotionAnalysisBinaryFileIO::MotionAnalysisBinaryFileIO()
  : AcquisitionFileIO()
  {
    this->SetFileType(AcquisitionFileIO::Binary);
    this->SetByteOrder(AcquisitionFileIO::IEEE_LittleEndian);
    this->SetStorageFormat(AcquisitionFileIO::Integer);
  };

  size_t MotionAnalysisBinaryFileIO::ReadKeyValueU8(uint8_t* val, IEEELittleEndianBinaryFileStream* bifs, int key)
  {
    this->ReadKey(bifs, key);
    int16_t size = bifs->ReadU16();
    if (size != 1)
      throw(MotionAnalysisBinaryFileIOException("Wrong value size."));
    *val = bifs->ReadU8();
    return 2;
  };

  size_t MotionAnalysisBinaryFileIO::ReadKeyValueU8(std::vector<uint8_t>& val, IEEELittleEndianBinaryFileStream* bifs, int key)
  {  
    this->ReadKey(bifs, key);
    int16_t size = bifs->ReadU16();
    val.resize(size * 4);
    for (size_t i = 0 ; i < val.size() ; ++i)
      val[i] = bifs->ReadU8();
    return 1 + size;
  };

  size_t MotionAnalysisBinaryFileIO::ReadKeyValueU16(uint16_t* val, IEEELittleEndianBinaryFileStream* bifs, int key)
  {
    this->ReadKey(bifs, key);
    this->CheckSizeForSingleValue(bifs);
    *val = bifs->ReadU16();
    return 2;
  };

  size_t MotionAnalysisBinaryFileIO::ReadKeyValueU16(std::vector<uint16_t>& val, IEEELittleEndianBinaryFileStream* bifs, int key)
  {
    this->ReadKey(bifs, key);
    int16_t size = bifs->ReadU16();
    val.resize(size * 2);
    for (int i = 0 ; i < static_cast<int>(val.size()) ; ++i)
      val[i] = bifs->ReadU16();
    return 1 + size;
  };

  size_t MotionAnalysisBinaryFileIO::ReadKeyValueU32(uint32_t* val, IEEELittleEndianBinaryFileStream* bifs, int key)
  {
    this->ReadKey(bifs, key);
    this->CheckSizeForSingleValue(bifs);
    uint16_t hsb = bifs->ReadU16();
    uint16_t lsb = bifs->ReadU16();
    *val = hsb << 16 | lsb;
    return 2;
  };

  size_t MotionAnalysisBinaryFileIO::ReadKeyValueFloat(float* val, IEEELittleEndianBinaryFileStream* bifs, int key)
  {
    this->ReadKey(bifs, key);
    this->CheckSizeForSingleValue(bifs);
    *val = bifs->ReadFloat();
    return 2;
  };

  size_t MotionAnalysisBinaryFileIO::ReadKeyValueString(std::string& val, IEEELittleEndianBinaryFileStream* bifs, int key)
  {
    this->ReadKey(bifs, key);
    int16_t size = bifs->ReadU16();
    val = bifs->ReadString(size * 4);
    val = val.erase(val.find_last_not_of((char)0x00) + 1);
    val = val.erase(0, val.find_first_not_of((char)0x00));
    return 1 + size;
  };

  void MotionAnalysisBinaryFileIO::ReadKey(IEEELittleEndianBinaryFileStream* bifs, int key) const
  {
    uint16_t readKey = bifs->ReadU16();
    if (readKey != key)
      throw(MotionAnalysisBinaryFileIOException("Keys mismatch: " + ToString(readKey) + " vs " + ToString(key)));
  };

  void MotionAnalysisBinaryFileIO::CheckSizeForSingleValue(IEEELittleEndianBinaryFileStream* bifs) const
  {
    int16_t size = bifs->ReadU16();
    if (size != 1)
      throw(MotionAnalysisBinaryFileIOException("Wrong value size."));
  };

  size_t MotionAnalysisBinaryFileIO::WriteKeyValue(IEEELittleEndianBinaryFileStream* bofs, uint16_t key, const std::vector<uint8_t>& val)
  {
    bofs->Write(key); bofs->Write(static_cast<uint16_t>(val.size() / 4));
    for (size_t i = 0 ; i < val.size() ; ++i)
      bofs->Write(val[i]);
    size_t size = val.size() / 4;
    size += ((val.size() % 4) > 0 ? 1 : 0);
    bofs->Fill(size * 4 - val.size());
    return 4 + size * 4;
  };

  size_t MotionAnalysisBinaryFileIO::WriteKeyValue(IEEELittleEndianBinaryFileStream* bofs, uint16_t key, const std::vector<uint16_t>& val)
  {
    bofs->Write(key); bofs->Write(static_cast<uint16_t>(val.size() / 2));
    for (size_t i = 0 ; i < val.size() ; ++i)
      bofs->Write(val[i]); 
    size_t size = val.size() / 2;
    size += ((val.size() % 2) > 0 ? 1 : 0);
    bofs->Fill((size * 2 - val.size()) * 2);
    return 4 + size * 4;
  };

  size_t MotionAnalysisBinaryFileIO::WriteKeyValue(IEEELittleEndianBinaryFileStream* bofs, uint16_t key, uint32_t val)
  {
    bofs->Write(key); bofs->Write((uint16_t)0x0001);
    uint16_t hsb = val >> 16;
    uint16_t lsb = val - (hsb << 16);
    bofs->Write(hsb); bofs->Write(lsb);
    return 4 + 4;
  };

  size_t MotionAnalysisBinaryFileIO::WriteKeyValue(IEEELittleEndianBinaryFileStream* bofs, uint16_t key, float val)
  {
    bofs->Write(key); bofs->Write((uint16_t)0x0001);
    uint16_t byteptr[2];
    memcpy(&byteptr, &val, sizeof(byteptr));
#if  PROCESSOR_TYPE == 3 /* IEEE_BigEndian */
    bofs->Write(byteptr[0]);
    bofs->Write(byteptr[1]);
#else
    bofs->Write(byteptr[1]);
    bofs->Write(byteptr[0]);
#endif   
    return 4 + 4;
  };

  size_t MotionAnalysisBinaryFileIO::WriteKeyValue(IEEELittleEndianBinaryFileStream* bofs, uint16_t key, const std::string& val, bool spacing)
  {
    size_t size = val.size() / 4;
    size_t sizeUpdated = size + ((val.size() % 4) > 0 ? 1 : 0);
    if ((size == sizeUpdated) && spacing)
      size += 1;
    else
      size = sizeUpdated;
    bofs->Write(key); bofs->Write(static_cast<uint16_t>(size));
    bofs->Write(val); 
    bofs->Fill(size * 4 - val.size());
    return 4 + size * 4;
  };
};