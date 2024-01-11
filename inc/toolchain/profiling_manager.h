/**
 * @file profiling_manager.h
 *
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2020. All rights reserved.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 */

#ifndef ACL_PROFILING_MANAGER_H_
#define ACL_PROFILING_MANAGER_H_

#include "profiling.h"
#include <mutex>
#include <unordered_set>
#include <map>
#include "toolchain/prof_api.h"
#include "common/log_inner.h"

namespace acl {
enum AclProfType {
    ModelTypeStart = MSPROF_REPORT_ACL_MODEL_BASE_TYPE,
    AclmdlExecute,
    AclmdlLoadFromMemWithQ,
    AclmdlLoadFromMemWithMem,
    AclmdlGetDesc,
    AclmdlLoadFromFile,
    AclmdlLoadFromFileWithMem,
    AclmdlLoad,
    AclmdlLoadFromMem,
    AclmdlSetInputAIPP,
    AclmdlSetAIPPByInputIndex,
    AclmdlExecuteAsync,
    AclmdlQuerySize,
    AclmdlQuerySizeFromMem,
    AclmdlSetDynamicBatchSize,
    AclmdlSetDynamicHWSize,
    AclmdlSetInputDynamicDims,
    AclmdlLoadWithConfig,
    AclmdlLoadFromFileWithQ,
    AclmdlUnload,
    ModelTypeEnd,
    OpTypeStart = MSPROF_REPORT_ACL_OP_BASE_TYPE,
    AclopLoad,
    AclopExecute,
    AclopCreateHandle,
    AclopDestroyHandle,
    AclopExecWithHandle,
    AclopExecuteV2,
    AclTransTensorDescFormat,
    AclopCreateKernel,
    AclopUpdateParams,
    AclopInferShape,
    AclopCompile,
    AclopCompileAndExecute,
    AclopCompileAndExecuteV2,
    AclGenGraphAndDumpForOp,
    AclblasGemmEx,
    AclblasCreateHandleForGemmEx,
    AclblasCreateHandleForHgemm,
    AclblasHgemm,
    AclblasS8gemm,
    AclblasCreateHandleForS8gemm,
    AclblasGemvEx,
    AclblasCreateHandleForGemvEx,
    AclblasHgemv,
    AclblasCreateHandleForHgemv,
    AclblasCreateHandleForS8gemv,
    AclblasS8gemv,
    AclopCast,
    AclopCreateHandleForCast,
    OpCompile,
    OpCompileAndDump,
    AclopDestroyAttr,
    AclCreateTensorDesc,
    AclDestroyTensorDesc,
    AclCreateDataBuffer,
    AclopCreateAttr,
    OpTypeEnd,
    RtTypeStart = MSPROF_REPORT_ACL_RUNTIME_BASE_TYPE,
    AclrtLaunchCallback,
    AclrtProcessReport,
    AclrtCreateContext,
    AclrtDestroyContext,
    AclrtSetCurrentContext,
    AclrtGetCurrentContext,
    AclrtSetDevice,
    AclrtSetDeviceWithoutTsdVXX,
    AclrtResetDevice,
    AclrtResetDeviceWithoutTsdVXX,
    AclrtSynchronizeDevice,
    AclrtSetTsDevice,
    AclrtCreateEvent,
    AclrtCreateEventWithFlag,
    AclrtDestroyEvent,
    AclrtRecordEvent,
    AclrtResetEvent,
    AclrtQueryEvent,
    AclrtQueryEventStatus,
    AclrtQueryEventWaitStatus,
    AclrtSynchronizeEvent,
    AclrtSetOpWaitTimeout,
    AclrtSetOpExecuteTimeOut,
    AclrtSetGroup,
    AclrtGetGroupCount,
    AclrtGetAllGroupInfo,
    AclrtGetGroupInfoDetail,
    AclMallocMemInner,
    AclrtMallocCached,
    AclrtMemFlush,
    AclrtMemInvalidate,
    AclrtFree,
    AclrtMallocHost,
    AclrtFreeHost,
    AclrtMemcpy,
    AclrtMemset,
    AclrtMemcpyAsync,
    AclrtMemsetAsync,
    AclrtDeviceCanAccessPeer,
    AclrtDeviceEnablePeerAccess,
    AclrtDeviceDisablePeerAccess,
    AclrtGetMemInfo,
    AclrtMemcpy2d,
    AclrtMemcpy2dAsync,
    AclrtCreateStream,
    AclrtCreateStreamWithConfig,
    AclrtDestroyStream,
    AclrtDestroyStreamForce,
    AclrtSynchronizeStream,
    AclrtSynchronizeStreamWithTimeout,
    AclrtStreamQuery,
    AclrtStreamWaitEvent,
    AclrtAllocatorCreateDesc,
    AclrtAllocatorDestroyDesc,
    AclrtCtxGetSysParamOpt,
    AclrtCtxSetSysParamOpt,
    AclrtGetOverflowStatus,
    AclrtResetOverflowStatus,
    AclrtGetDeviceCount,
    AclrtGetDevice,
    AclrtMalloc,
    AclrtSetStreamFailureMode,
    AclrtQueryDeviceStatus,
    AclrtReserveMemAddress,
    AclrtReleaseMemAddress,
    AclrtMallocPhysical,
    AclrtFreePhysical,
    AclrtMapMem,
    AclrtUnmapMem,
    AclrtLaunchKernel,
    AclrtMemExportToShareableHandle,
    AclrtMemImportFromShareableHandle,
    AclrtMemSetPidToShareableHandle,
    AclrtMemGetAllocationGranularity,
    AclrtDeviceGetBareTgid,
    RtTypeEnd,
    OthersTypeStart = MSPROF_REPORT_ACL_OTHERS_BASE_TYPE,
    AcldvppCreateChannel,
    AcldvppDestroyChannel,
    AcldvppJpegDecodeAsync,
    AcldvppJpegEncodeAsync,
    AcldvppJpegGetImageInfo,
    AcldvppJpegGetImageInfoV2,
    AcldvppJpegPredictEncSize,
    AcldvppJpegPredictDecSize,
    AcldvppPngDecodeAsync,
    AcldvppPngGetImageInfo,
    AcldvppPngPredictDecSize,
    AclvdecCreateChannel,
    AclvdecDestroyChannel,
    AclvdecSendFrame,
    AclvdecSendSkippedFrame,
    AclvencCreateChannel,
    AclvencDestroyChannel,
    AclvencSendFrame,
    AcldvppVpcResizeAsync,
    AcldvppVpcCropAsync,
    AcldvppVpcCropAndPasteAsync,
    AcldvppVpcConvertColorAsync,
    AcldvppVpcPyrDownAsync,
    AcldvppVpcBatchCropAsync,
    AcldvppVpcBatchCropAndPasteAsync,
    AcldvppVpcEqualizeHistAsync,
    AcldvppVpcMakeBorderAsync,
    AcldvppVpcCalcHistAsync,
    AcldvppVpcCropResizeAsync,
    AcldvppVpcBatchCropResizeAsync,
    AcldvppVpcCropResizePasteAsync,
    AcldvppVpcBatchCropResizePasteAsync,
    AcldvppVpcBatchCropResizeMakeBorderAsync,
    AcltdtEnqueue,
    AcltdtDequeue,
    AcltdtEnqueueData,
    AcltdtDequeueData,
    AcldvppMalloc,
    AcldvppFree,
    AcldvppCreatePicDesc,
    AcldvppDestroyPicDesc,
    AcldvppCreateRoiConfig,
    AcldvppDestroyRoiConfig,
    AcldvppCreateJpegeConfig,
    AcldvppDestroyJpegeConfig,
    AcldvppCreateResizeConfig,
    AcldvppDestroyResizeConfig,
    AcldvppCreateChannelDesc,
    AcldvppDestroyChannelDesc,
    AclvdecCreateChannelDesc,
    AclvdecDestroyChannelDesc,
    AcldvppCreateStreamDesc,
    AcldvppDestroyStreamDesc,
    AclvdecCreateFrameConfig,
    AclvdecDestroyFrameConfig,
    AclvencCreateChannelDesc,
    AclvencDestroyChannelDesc,
    AclvencCreateFrameConfig,
    AclvencDestroyFrameConfig,
    AcldvppDestroyBatchPicDesc,
    // AclmdlLoadFromFileModified,
    // this is the end, can not add after OthersTypeEnd
    OthersTypeEnd
};

    class AclProfilingManager final {
    public:
        static AclProfilingManager &GetInstance();
        // init acl prof module
        aclError Init();
        // uninit acl prof module
        aclError UnInit();
        // acl profiling module is running or not
        bool AclProfilingIsRun() const
        {
            return isProfiling_;
        }

        // return flag of device list that needs report prof data is empty or not
        bool IsDeviceListEmpty() const;
        aclError ProfilingData(ReporterData &data);
        aclError AddDeviceList(const uint32_t *const deviceIdList, const uint32_t deviceNums);
        aclError RemoveDeviceList(const uint32_t *const deviceIdList, const uint32_t deviceNums);
        bool IsDeviceEnable(const uint32_t deviceId) const;
        aclError RegisterProfilingType() const;

    private:
        ~AclProfilingManager() = default;
        AclProfilingManager() = default;
        bool isProfiling_ = false;
        std::mutex mutex_;
        std::unordered_set<uint32_t> deviceList_;
        std::map<std::string, uint64_t> HashMap = {};
    };

    class ACL_FUNC_VISIBILITY AclProfilingReporter {
    public:
        explicit AclProfilingReporter(const AclProfType apiId);
        virtual ~AclProfilingReporter() noexcept;
    private:
        uint64_t startTime_ = 0UL;
        int32_t deviceId_ = -1;
        const char_t* funcTag_ = nullptr;
        const AclProfType aclApi_;
    };

} // namespace acl
#define ACL_PROFILING_REG(apiId) \
    const acl::AclProfilingReporter profilingReporter(apiId)
#endif // ACL_PROFILING_MANAGER_H_
