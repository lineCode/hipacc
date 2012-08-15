//
// Copyright (c) 2012, University of Erlangen-Nuremberg
// Copyright (c) 2012, Siemens AG
// All rights reserved.
// 
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met: 
// 
// 1. Redistributions of source code must retain the above copyright notice, this
//    list of conditions and the following disclaimer. 
// 2. Redistributions in binary form must reproduce the above copyright notice,
//    this list of conditions and the following disclaimer in the documentation
//    and/or other materials provided with the distribution. 
// 
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND 
// ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
// WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE 
// DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR 
// ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
// (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
// LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND 
// ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
// SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//

//===--- ClassRepresentation.h - Representation of the DSL C++ classes ----===//
//
// This provides the internal representation of the compiler-known DSL C++
// classes.
//
//===----------------------------------------------------------------------===//

#include "hipacc/DSL/ClassRepresentation.h"

using namespace clang;
using namespace hipacc;


std::string HipaccImage::getTextureType() {
  const BuiltinType *BT = type->getAs<BuiltinType>();

  switch (BT->getKind()) {
    case BuiltinType::WChar_U:
    case BuiltinType::WChar_S:
    case BuiltinType::ULongLong:
    case BuiltinType::UInt128:
    case BuiltinType::LongLong:
    case BuiltinType::Int128:
    case BuiltinType::LongDouble:
    case BuiltinType::Void:
    case BuiltinType::Bool:
    case BuiltinType::Long:
    case BuiltinType::ULong:
    case BuiltinType::Double:
    default:
      Ctx.getDiagnostics().Report(VD->getLocation(),
          Ctx.getDiagnostics().getCustomDiagID(DiagnosticsEngine::Error,
            "BuiltinType %0 of Image %1 not supported for textures.")) <<
        BT->getName(PrintingPolicy(Ctx.getLangOpts())) << VD->getName();
      assert(0 && "BuiltinType for texture not supported");
    case BuiltinType::Char_S:
    case BuiltinType::SChar:
      return "CU_AD_FORMAT_SIGNED_INT8";
    case BuiltinType::Char_U:
    case BuiltinType::UChar:
      return "CU_AD_FORMAT_UNSIGNED_INT8";
    case BuiltinType::Short:
      return "CU_AD_FORMAT_SIGNED_INT16";
    case BuiltinType::Char16:
    case BuiltinType::UShort:
      return "CU_AD_FORMAT_UNSIGNED_INT16";
    case BuiltinType::Int:
      return "CU_AD_FORMAT_SIGNED_INT32";
    case BuiltinType::Char32:
    case BuiltinType::UInt:
      return "CU_AD_FORMAT_UNSIGNED_INT32";
    case BuiltinType::Float:
      return "CU_AD_FORMAT_FLOAT";
  }
}


std::string HipaccImage::getImageReadFunction() {
  const BuiltinType *BT = type->getAs<BuiltinType>();

  switch (BT->getKind()) {
    case BuiltinType::WChar_U:
    case BuiltinType::WChar_S:
    case BuiltinType::ULongLong:
    case BuiltinType::UInt128:
    case BuiltinType::LongLong:
    case BuiltinType::Int128:
    case BuiltinType::LongDouble:
    case BuiltinType::Void:
    case BuiltinType::Bool:
    case BuiltinType::Long:
    case BuiltinType::ULong:
    case BuiltinType::Double:
    default:
      Ctx.getDiagnostics().Report(VD->getLocation(),
          Ctx.getDiagnostics().getCustomDiagID(DiagnosticsEngine::Error,
            "BuiltinType %0 of Image %1 not supported for Image objects.")) <<
        BT->getName(PrintingPolicy(Ctx.getLangOpts())) << VD->getName();
      assert(0 && "BuiltinType for Image object not supported");
    case BuiltinType::Char_S:
    case BuiltinType::SChar:
    case BuiltinType::Short:
    case BuiltinType::Int:
      return "read_imagei";
    case BuiltinType::Char_U:
    case BuiltinType::UChar:
    case BuiltinType::Char16:
    case BuiltinType::UShort:
    case BuiltinType::Char32:
    case BuiltinType::UInt:
      return "read_imageui";
    case BuiltinType::Float:
      return "read_imagef";
  }
}


void HipaccBoundaryCondition::setConstExpr(APValue &val, ASTContext &Ctx) {
  QualType QT = getImage()->getPixelQualType();
  const BuiltinType *BT = QT->getAs<BuiltinType>();

  switch (BT->getKind()) {
    case BuiltinType::WChar_S:
    case BuiltinType::WChar_U:
    case BuiltinType::Char16:
    case BuiltinType::Char32:
    case BuiltinType::ULongLong:
    case BuiltinType::UInt128:
    case BuiltinType::LongLong:
    case BuiltinType::Int128:
    case BuiltinType::LongDouble:
    case BuiltinType::Void:
    case BuiltinType::Bool:
    default:
      assert(0 && "BuiltinType for Boundary handling constant not supported.");
    case BuiltinType::Char_S:
    case BuiltinType::SChar:
    case BuiltinType::Char_U:
    case BuiltinType::UChar:
      constExpr = new (Ctx) CharacterLiteral(val.getInt().getSExtValue(),
          CharacterLiteral::Ascii, QT, SourceLocation());
      break;
    case BuiltinType::Short:
    case BuiltinType::UShort:
    case BuiltinType::Int:
    case BuiltinType::UInt:
    case BuiltinType::Long:
    case BuiltinType::ULong:
      constExpr = new (Ctx) IntegerLiteral(Ctx, val.getInt(), QT,
          SourceLocation());
      break;
    case BuiltinType::Float:
    case BuiltinType::Double:
      constExpr = FloatingLiteral::Create(Ctx, llvm::APFloat(val.getFloat()),
          false, QT, SourceLocation());
      break;
  }
}


void HipaccIterationSpace::createOutputAccessor() {
  // create Accessor for accessing the image associated with the IterationSpace
  // during ASTTranslate
  HipaccBoundaryCondition *BC = new HipaccBoundaryCondition(img, VD);
  BC->setSizeX(0);
  BC->setSizeY(0);
  BC->setBoundaryHandling(BOUNDARY_UNDEFINED);

  acc = new HipaccAccessor(BC, InterpolateNO, VD);
}


void HipaccKernel::calcSizes() {
  for (std::map<FieldDecl *, HipaccAccessor *>::iterator iter = imgMap.begin(),
      eiter=imgMap.end(); iter!=eiter; ++iter) {
    if (iter->second->getSizeX() > max_size_x &&
        iter->second->getBoundaryHandling()!=BOUNDARY_UNDEFINED)
      max_size_x = iter->second->getSizeX();
    if (iter->second->getSizeY() > max_size_y &&
        iter->second->getBoundaryHandling()!=BOUNDARY_UNDEFINED)
      max_size_y = iter->second->getSizeY();
  }
}


struct sortOccMap {
  bool operator()(const std::pair<unsigned int, float> &left, const std::pair<unsigned int, float> &right) {
    if (left.second < right.second) return false;
    if (right.second < left.second) return true;
    return left.first < right.first;
  }
};


void HipaccKernel::calcConfig() {
  std::vector<std::pair<unsigned int, float> > occVec;
  unsigned int num_threads = max_threads_per_warp;

  while (num_threads <= max_threads_per_block) {
    // allocations per thread block limits
    int warps_per_block = (int)ceil((float)num_threads /
        (float)max_threads_per_warp);
    int registers_per_block;
    if (isAMDGPU()) {
      // for AMD assume simple allocation strategy
      registers_per_block = warps_per_block * num_reg * max_threads_per_warp;
    } else {
      switch (allocation_granularity) {
        case BLOCK:
          // allocation in steps of two
          registers_per_block = (int)ceil((float)warps_per_block /
              (float)warp_register_alloc_size) * warp_register_alloc_size *
            num_reg * max_threads_per_warp;
          registers_per_block = (int)ceil((float)registers_per_block /
              (float)register_alloc_size) * register_alloc_size;
          break;
        case WARP:
          registers_per_block = (int)ceil((float)(num_reg *
                max_threads_per_warp) / (float)register_alloc_size) *
            register_alloc_size;
          registers_per_block *= (int)ceil((float)warps_per_block /
              (float)warp_register_alloc_size) * warp_register_alloc_size;
          break;
      }
    }

    int smem_used = 0;
    // calculate shared memory usage for pixels staged to shared memory
    for (unsigned int i=0; i<KC->getNumImages(); i++) {
      HipaccAccessor *Acc = getImgFromMapping(KC->getImgFields().data()[i]);
      if (useLocalMemory(Acc)) {
        // assume we use a configuration with warp_size threads for the
        // x-dimension
        smem_used += (Acc->getSizeX() + max_threads_per_warp) * (Acc->getSizeY()
            + num_threads/max_threads_per_warp - 1) *
          Acc->getImage()->getPixelSize();
      }
    }
    int shared_memory_per_block = (int)ceil((float)smem_used /
        (float)shared_memory_alloc_size) * shared_memory_alloc_size;

    // maximum thread blocks per multiprocessor
    int lim_by_max_warps = std::min(max_blocks_per_multiprocessor, (unsigned
          int)floor((float)max_warps_per_multiprocessor /
            (float)warps_per_block));
    int lim_by_reg, lim_by_smem;
    if (num_reg > max_register_per_thread) {
      lim_by_reg = 0;
    } else {
      if (num_reg > 0) {
        lim_by_reg = (int)floor((float)max_total_registers /
            (float)registers_per_block);
      } else {
        lim_by_reg = max_blocks_per_multiprocessor;
      }
    }
    if (smem_used > 0) {
      lim_by_smem = (int)floor((float)max_total_shared_memory /
          (float)shared_memory_per_block);
    } else {
      lim_by_smem = max_blocks_per_multiprocessor;
    }

    // calculate GPU occupancy
    int active_thread_blocks_per_multiprocessor = std::min(std::min(lim_by_max_warps, lim_by_reg), lim_by_smem);
    if (active_thread_blocks_per_multiprocessor > 0) max_threads_for_kernel = num_threads;
    int active_warps_per_multiprocessor = active_thread_blocks_per_multiprocessor * warps_per_block;
    //int active_threads_per_multiprocessor = active_thread_blocks_per_multiprocessor * num_threads;
    float occupancy = (float)active_warps_per_multiprocessor/(float)max_warps_per_multiprocessor;
    //int max_simultaneous_blocks_per_GPU = active_thread_blocks_per_multiprocessor*max_multiprocessors_per_GPU;

    occVec.push_back(std::pair<int, float>(num_threads, occupancy));
    num_threads += max_threads_per_warp;
  }

  // sort configurations according to occupancy and number of threads
  std::sort(occVec.begin(), occVec.end(), sortOccMap());
  std::vector<std::pair<unsigned int, float> >::iterator iter;

  // calculate (optimal) kernel configuration from the kernel window sizes and
  // ignore the limitation of maximal threads per block
  unsigned int num_threads_x_opt = max_threads_per_warp;
  unsigned int num_threads_y_opt = 1;
  while (num_threads_x_opt < max_size_x>>1)
    num_threads_x_opt += max_threads_per_warp;
  if (!options.useLocalMemory())
  while (num_threads_y_opt < max_size_y>>1) num_threads_y_opt += 1;

  // Heuristic:
  // 0) Maximize occupancy (e.g. to hide instruction latency
  // 1) - Minimize #threads for border handling (e.g. prefer y over x)
  //    - Prefer x over y when no border handling is necessary
  llvm::errs() << "\nCalculating kernel configuration for " << kernelName << "\n";
  llvm::errs() << "\toptimal configuration: " << num_threads_x_opt << "x" << num_threads_y_opt << "\n";
  for (iter=occVec.begin(); iter<occVec.end(); ++iter) {
    std::pair<unsigned int, float> occMap = *iter;
    llvm::errs() << "\t" << occMap.first << " threads:\t" << occMap.second << "\t";

    // make difference if we create border handling or not
    if (max_size_y > 1 && !options.useLocalMemory()) {
      // start with warp_size or num_threads_x_opt if possible
      unsigned int num_threads_x = max_threads_per_warp;
      if (occMap.first >= num_threads_x_opt && occMap.first % num_threads_x_opt == 0) {
        num_threads_x = num_threads_x_opt;
      }
      unsigned int num_threads_y = occMap.first / num_threads_x;
      llvm::errs() << " -> " << num_threads_x << "x" << num_threads_y << "\n";
    } else {
      // use all threads for x direction
      llvm::errs() << " -> " << occMap.first << "x1\n";
    }
  }

  // start with first configuration
  iter = occVec.begin();
  std::pair<unsigned int, float> occMap = *iter;

  // make difference if we create border handling or not
  // TODO: add exploration for PPT > 1
  if (max_size_y > 1 && !options.useLocalMemory()) {
    // start with warp_size or num_threads_x_opt if possible
    num_threads_x = max_threads_per_warp;
    if (occMap.first >= num_threads_x_opt && occMap.first % num_threads_x_opt == 0)
      num_threads_x = num_threads_x_opt;
    num_threads_y = occMap.first / num_threads_x;
  } else {
    // use all threads for x direction
    num_threads_x = occMap.first;
    num_threads_y = 1;
  }

  // estimate block required for border handling - the exact number depends on
  // offsets and is not known at compile time 
  unsigned int num_blocks_bh_x = max_size_x<=1?0:(unsigned int)ceil((float)(max_size_x>>1) / (float)num_threads_x);
  unsigned int num_blocks_bh_y = max_size_y<=1?0:(unsigned int)ceil((float)(max_size_y>>1) / (float)(num_threads_y*pixels_per_thread[KC->getKernelType()]));

  if ((max_size_y > 1) || num_threads_x != num_threads_x_opt || num_threads_y != num_threads_y_opt) {
    //std::vector<std::pair<unsigned int, float> >::iterator iter_n = occVec.begin()

    // look-ahead if other configurations match better
    while (++iter<occVec.end()) {
      std::pair<unsigned int, float> occMapNext = *iter;
      // bail out on lower occupancy
      if (occMapNext.second < occMap.second) break;

      // start with warp_size or num_threads_x_opt if possible
      unsigned int num_threads_x_tmp = max_threads_per_warp;
      if (occMapNext.first >= num_threads_x_opt && occMapNext.first % num_threads_x_opt == 0)
        num_threads_x_tmp = num_threads_x_opt;
      unsigned int num_threads_y_tmp = occMapNext.first / num_threads_x_tmp;

      // block required for border handling
      unsigned int num_blocks_bh_x_tmp = max_size_x<=1?0:(unsigned int)ceil((float)(max_size_x>>1) / (float)num_threads_x_tmp);
      unsigned int num_blocks_bh_y_tmp = max_size_y<=1?0:(unsigned int)ceil((float)(max_size_y>>1) / (float)(num_threads_y_tmp*pixels_per_thread[KC->getKernelType()]));

      // use new configuration if we save blocks for border handling
      if (num_blocks_bh_x_tmp+num_blocks_bh_y_tmp < num_blocks_bh_x+num_blocks_bh_y) {
        num_threads_x = num_threads_x_tmp;
        num_threads_y = num_threads_y_tmp;
        num_blocks_bh_x = num_blocks_bh_x_tmp;
        num_blocks_bh_y = num_blocks_bh_y_tmp;
      }
    }
  }

  // fall back to user specified configuration
  if (options.useKernelConfig(USER_ON)) {
    setDefaultConfig();
    num_blocks_bh_x = max_size_x<=1?0:(unsigned int)ceil((float)(max_size_x>>1) / (float)num_threads_x);
    num_blocks_bh_y = max_size_y<=1?0:(unsigned int)ceil((float)(max_size_y>>1) / (float)(num_threads_y*pixels_per_thread[KC->getKernelType()]));
  }

  llvm::errs() << "Using configuration " << num_threads_x << "x" <<
    num_threads_y << "(occupancy: " << occMap.second << ") for kernel '" <<
    kernelName << "'\n";
  llvm::errs() << "\t Blocks required for border handling: " <<
    num_blocks_bh_x << "x" << num_blocks_bh_y << "\n\n";
}

void HipaccKernel::setDefaultConfig() {
  num_threads_x = default_num_threads_x;
  num_threads_y = default_num_threads_y;
}

void HipaccKernel::addParam(QualType QT1, QualType QT2, QualType QT3, std::string
    typeC, std::string typeO, llvm::StringRef name, FieldDecl *fd) {
  argTypesCUDA.push_back(QT1);
  argTypesOpenCL.push_back(QT2);
  argTypesC.push_back(QT3);

  argTypeNamesCUDA.push_back(typeC);
  argTypeNamesOpenCL.push_back(typeO);

  argNames.push_back(name);
  argFields.push_back(fd);
}

void HipaccKernel::createArgInfo() {
  if (argTypesCUDA.size()) return;

  llvm::SmallVector<HipaccKernelClass::argumentInfo, 16> arguments =
    KC->arguments;

  // add output image from iteration space 
  QualType isQT = iterationSpace->getImage()->getPixelQualType();
  addParam(Ctx.getPointerType(isQT), Ctx.getPointerType(isQT),
      Ctx.getPointerType(Ctx.getConstantArrayType(isQT, llvm::APInt(32, 4096),
          ArrayType::Normal, false)), Ctx.getPointerType(isQT).getAsString(),
      "cl_mem", "Output", NULL);
  
  // normal parameters
  for (unsigned int i=0; i<KC->getNumArgs(); i++) {
    FieldDecl *FD = arguments.data()[i].field;
    QualType QT = arguments.data()[i].type;
    llvm::StringRef name = arguments.data()[i].name;
    QualType QTtmp;

    switch (arguments.data()[i].kind) {
      case HipaccKernelClass::Normal:
        addParam(QT, QT, QT, QT.getAsString(), QT.getAsString(), name, FD);

        break;
      case HipaccKernelClass::IterationSpace:
        // iteration space is added as the first argument
        break;
      case HipaccKernelClass::Image: 
        // for textures use no pointer type
        if (useTextureMemory(getImgFromMapping(FD)) &&
            KC->getImgAccess(FD) == READ_ONLY) {
          addParam(Ctx.getPointerType(QT), Ctx.getPointerType(QT),
              Ctx.getPointerType(Ctx.getConstantArrayType(QT, llvm::APInt(32,
                    4096), ArrayType::Normal, false)), QT.getAsString(),
              "cl_mem", name, FD);
        } else {
          addParam(Ctx.getPointerType(QT), Ctx.getPointerType(QT),
              Ctx.getPointerType(Ctx.getConstantArrayType(QT, llvm::APInt(32,
                    4096), ArrayType::Normal, false)),
              Ctx.getPointerType(QT).getAsString(), "cl_mem", name, FD);
        }

        // add types for image width/height plus stride
        addParam(Ctx.getConstType(Ctx.IntTy), Ctx.getConstType(Ctx.IntTy),
            Ctx.getConstType(Ctx.IntTy),
            Ctx.getConstType(Ctx.IntTy).getAsString(),
            Ctx.getConstType(Ctx.IntTy).getAsString(),
            getImgFromMapping(FD)->getWidthParm(), NULL);
        addParam(Ctx.getConstType(Ctx.IntTy), Ctx.getConstType(Ctx.IntTy),
            Ctx.getConstType(Ctx.IntTy),
            Ctx.getConstType(Ctx.IntTy).getAsString(),
            Ctx.getConstType(Ctx.IntTy).getAsString(),
            getImgFromMapping(FD)->getHeightParm(), NULL);

        // stride
        if (options.emitPadding() || getImgFromMapping(FD)->isCrop()) {
          addParam(Ctx.getConstType(Ctx.IntTy), Ctx.getConstType(Ctx.IntTy),
              Ctx.getConstType(Ctx.IntTy),
              Ctx.getConstType(Ctx.IntTy).getAsString(),
              Ctx.getConstType(Ctx.IntTy).getAsString(),
              getImgFromMapping(FD)->getStrideParm(), NULL);
        }

        // offset_x
        if (!getImgFromMapping(FD)->getOffsetX().empty()) {
          addParam(Ctx.getConstType(Ctx.IntTy), Ctx.getConstType(Ctx.IntTy),
              Ctx.getConstType(Ctx.IntTy),
              Ctx.getConstType(Ctx.IntTy).getAsString(),
              Ctx.getConstType(Ctx.IntTy).getAsString(),
              getImgFromMapping(FD)->getOffsetXParm(), NULL);
        }
        // offset_y
        if (!getImgFromMapping(FD)->getOffsetY().empty()) {
          addParam(Ctx.getConstType(Ctx.IntTy), Ctx.getConstType(Ctx.IntTy),
              Ctx.getConstType(Ctx.IntTy),
              Ctx.getConstType(Ctx.IntTy).getAsString(),
              Ctx.getConstType(Ctx.IntTy).getAsString(),
              getImgFromMapping(FD)->getOffsetYParm(), NULL);
        }

        break;
      case HipaccKernelClass::Mask:
        QTtmp = Ctx.getPointerType(Ctx.getConstantArrayType(QT, llvm::APInt(32,
                getMaskFromMapping(FD)->getSizeX()), ArrayType::Normal, false));
        // OpenCL non-constant mask
        if (!getMaskFromMapping(FD)->isConstant()) {
          addParam(QTtmp, Ctx.getPointerType(QT), QTtmp, QTtmp.getAsString(),
              Ctx.getPointerType(QT).getAsString(), name, FD);
        } else {
          addParam(QTtmp, QTtmp, QTtmp, QTtmp.getAsString(),
              QTtmp.getAsString(), name, FD);
        }

        break;
    }
  }

  // is_stride
  addParam(Ctx.getConstType(Ctx.IntTy), Ctx.getConstType(Ctx.IntTy),
      Ctx.getConstType(Ctx.IntTy), Ctx.getConstType(Ctx.IntTy).getAsString(),
      Ctx.getConstType(Ctx.IntTy).getAsString(), "is_stride", NULL);

  // is_width, is_height
  addParam(Ctx.getConstType(Ctx.IntTy), Ctx.getConstType(Ctx.IntTy),
      Ctx.getConstType(Ctx.IntTy), Ctx.getConstType(Ctx.IntTy).getAsString(),
      Ctx.getConstType(Ctx.IntTy).getAsString(), "is_width", NULL);
  addParam(Ctx.getConstType(Ctx.IntTy), Ctx.getConstType(Ctx.IntTy),
      Ctx.getConstType(Ctx.IntTy), Ctx.getConstType(Ctx.IntTy).getAsString(),
      Ctx.getConstType(Ctx.IntTy).getAsString(), "is_height", NULL);

  // is_offset_x, is_offset_y
  if (!iterationSpace->getOffsetX().empty()) {
    addParam(Ctx.getConstType(Ctx.IntTy), Ctx.getConstType(Ctx.IntTy),
        Ctx.getConstType(Ctx.IntTy), Ctx.getConstType(Ctx.IntTy).getAsString(),
        Ctx.getConstType(Ctx.IntTy).getAsString(), "is_offset_x", NULL);
    addParam(Ctx.getConstType(Ctx.IntTy), Ctx.getConstType(Ctx.IntTy),
        Ctx.getConstType(Ctx.IntTy), Ctx.getConstType(Ctx.IntTy).getAsString(),
        Ctx.getConstType(Ctx.IntTy).getAsString(), "is_offset_y", NULL);
  }

  // bh_start_left, bh_start_right
  if (getMaxSizeX()) {
    addParam(Ctx.getConstType(Ctx.IntTy), Ctx.getConstType(Ctx.IntTy),
        Ctx.getConstType(Ctx.IntTy), Ctx.getConstType(Ctx.IntTy).getAsString(),
        Ctx.getConstType(Ctx.IntTy).getAsString(), "bh_start_left", NULL);
    addParam(Ctx.getConstType(Ctx.IntTy), Ctx.getConstType(Ctx.IntTy),
        Ctx.getConstType(Ctx.IntTy), Ctx.getConstType(Ctx.IntTy).getAsString(),
        Ctx.getConstType(Ctx.IntTy).getAsString(), "bh_start_right", NULL);
  }
  // bh_start_top, bh_start_bottom
  if (getMaxSizeY()) {
    addParam(Ctx.getConstType(Ctx.IntTy), Ctx.getConstType(Ctx.IntTy),
        Ctx.getConstType(Ctx.IntTy), Ctx.getConstType(Ctx.IntTy).getAsString(),
        Ctx.getConstType(Ctx.IntTy).getAsString(), "bh_start_top", NULL);
    addParam(Ctx.getConstType(Ctx.IntTy), Ctx.getConstType(Ctx.IntTy),
        Ctx.getConstType(Ctx.IntTy), Ctx.getConstType(Ctx.IntTy).getAsString(),
        Ctx.getConstType(Ctx.IntTy).getAsString(), "bh_start_bottom", NULL);
  }
  // bh_fall_back
  addParam(Ctx.getConstType(Ctx.IntTy), Ctx.getConstType(Ctx.IntTy),
      Ctx.getConstType(Ctx.IntTy), Ctx.getConstType(Ctx.IntTy).getAsString(),
      Ctx.getConstType(Ctx.IntTy).getAsString(), "bh_fall_back", NULL);
}


void HipaccKernel::createHostArgInfo(Expr **hostArgs, std::string &hostLiterals,
    unsigned int &literalCount) {
  if (hostArgNames.size()) hostArgNames.clear();

  // iteration space image
  hostArgNames.push_back(iterationSpace->getImage()->getName());

  for (unsigned int i=0; i<KC->getNumArgs(); i++) {
    FieldDecl *FD = KC->arguments.data()[i].field;

    std::string Str;
    llvm::raw_string_ostream SS(Str);

    switch (KC->arguments.data()[i].kind) {
      case HipaccKernelClass::Normal:
        hostArgs[i+1]->printPretty(SS, Ctx, 0,
            PrintingPolicy(Ctx.getLangOpts()));

        if (options.emitCUDA()) {
          hostArgNames.push_back(SS.str());
        } else {
          // create a temporary variable for integer literals and floating
          // literals for OpenCL
          if (isa<IntegerLiteral>(hostArgs[i+1]->IgnoreParenCasts()) ||
              isa<FloatingLiteral>(hostArgs[i+1]->IgnoreParenCasts()) ||
              isa<CharacterLiteral>(hostArgs[i+1]->IgnoreParenCasts())) {

            // get the text string for the argument
            std::stringstream LSS;
            LSS << "_tmpLiteral" << literalCount;
            literalCount++;

            hostLiterals += hostArgs[i+1]->IgnoreParenCasts()->getType().getAsString();
            hostLiterals += " ";
            hostLiterals += LSS.str();
            hostLiterals += " = ";
            hostLiterals += SS.str();
            hostLiterals += ";\n    ";
            hostArgNames.push_back(LSS.str());
          } else {
            hostArgNames.push_back(SS.str());
          }
        }

        break;
      case HipaccKernelClass::IterationSpace:
        // iteration space is added as the first argument -> i+1
        break;
      case HipaccKernelClass::Image: 
        // image
        hostArgNames.push_back(getImgFromMapping(FD)->getImage()->getName());

        // width, height
        hostArgNames.push_back(getImgFromMapping(FD)->getWidth());
        hostArgNames.push_back(getImgFromMapping(FD)->getHeight());

        // stride
        if (options.emitPadding() || getImgFromMapping(FD)->isCrop()) {
          hostArgNames.push_back(getImgFromMapping(FD)->getImage()->getStride());
        }
        
        // offset_x
        if (!getImgFromMapping(FD)->getOffsetX().empty()) {
          hostArgNames.push_back(getImgFromMapping(FD)->getOffsetX());
        }
        // offset_y
        if (!getImgFromMapping(FD)->getOffsetY().empty()) {
          hostArgNames.push_back(getImgFromMapping(FD)->getOffsetY());
        }

        break;
      case HipaccKernelClass::Mask:
        hostArgNames.push_back(getMaskFromMapping(FD)->getName());

        break;
    }
  }
  // is_stride
  hostArgNames.push_back(iterationSpace->getImage()->getStride());

  // is_width, is_height
  hostArgNames.push_back(iterationSpace->getWidth());
  hostArgNames.push_back(iterationSpace->getHeight());

  // is_offset_x, is_offset_y
  if (!iterationSpace->getOffsetX().empty()) {
    hostArgNames.push_back(iterationSpace->getOffsetX());
    hostArgNames.push_back(iterationSpace->getOffsetY());
  }

  // bh_start_left, bh_start_right
  if (getMaxSizeX()) {
    hostArgNames.push_back(getName() + "_info.bh_start_left");
    hostArgNames.push_back(getName() + "_info.bh_start_right");
  }
  // bh_start_top, bh_start_bottom
  if (getMaxSizeY()) {
    hostArgNames.push_back(getName() + "_info.bh_start_top");
    hostArgNames.push_back(getName() + "_info.bh_start_bottom");
  }
  // bh_fall_back
  hostArgNames.push_back(getName() + "_info.bh_fall_back");
}

// vim: set ts=2 sw=2 sts=2 et ai:

