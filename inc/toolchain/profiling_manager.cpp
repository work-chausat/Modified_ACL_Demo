/**
* @file profiling_manager.cpp
*
* Copyright (c) Huawei Technologies Co., Ltd. 2019-2020. All Rights reserved.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
*/

#include "profiling_manager.h"

#include <thread>
#include <cstring>
#include <sstream>
#include <securec.h>

#include "framework/common/ge_format_util.h"
#include "framework/common/profiling_definitions.h"
#include "mmpa/mmpa_api.h"

#include "common/log_inner.h"

namespace acl {
    const std::map<AclProfType, std::string> PROF_TYPE_TO_NAMES = {
        {AclProfType::AclmdlExecute,                            "aclmdlExecute"},
        {AclProfType::AclmdlLoadFromMemWithQ,                   "aclmdlLoadFromMemWithQ"},
        {AclProfType::AclmdlLoadFromMemWithMem,                 "aclmdlLoadFromMemWithMem"},
        {AclProfType::AclmdlGetDesc,                            "aclmdlGetDesc"},
        {AclProfType::AclmdlLoadFromFile,                       "aclmdlLoadFromFile"},
        {AclProfType::AclmdlLoadFromFileWithMem,                "aclmdlLoadFromFileWithMem"},
        {AclProfType::AclmdlLoad,                               "aclmdlLoad"},
        {AclProfType::AclmdlLoadFromMem,                        "aclmdlLoadFromMem"},
        {AclProfType::AclmdlSetInputAIPP,                       "aclmdlSetInputAIPP"},
        {AclProfType::AclmdlSetAIPPByInputIndex,                "aclmdlSetAIPPByInputIndex"},
        {AclProfType::AclmdlExecuteAsync,                       "aclmdlExecuteAsync"},
        {AclProfType::AclmdlQuerySize,                          "aclmdlQuerySize"},
        {AclProfType::AclmdlQuerySizeFromMem,                   "aclmdlQuerySizeFromMem"},
        {AclProfType::AclmdlSetDynamicBatchSize,                "aclmdlSetDynamicBatchSize"},
        {AclProfType::AclmdlSetDynamicHWSize,                   "aclmdlSetDynamicHWSize"},
        {AclProfType::AclmdlSetInputDynamicDims,                "aclmdlSetInputDynamicDims"},
        {AclProfType::AclmdlLoadWithConfig,                     "aclmdlLoadWithConfig"},
        {AclProfType::AclmdlLoadFromFileWithQ,                  "aclmdlLoadFromFileWithQ"},
        {AclProfType::AclmdlUnload,                             "aclmdlUnload"},
        {AclProfType::AclopLoad,                                "aclopLoad"},
        {AclProfType::AclopExecute,                             "aclopExecute"},
        {AclProfType::AclopCreateHandle,                        "aclopCreateHandle"},
        {AclProfType::AclopDestroyHandle,                       "aclopDestroyHandle"},
        {AclProfType::AclopExecWithHandle,                      "aclopExecWithHandle"},
        {AclProfType::AclopExecuteV2,                           "aclopExecuteV2"},
        {AclProfType::AclTransTensorDescFormat,                 "aclTransTensorDescFormat"},
        {AclProfType::AclopCreateKernel,                        "aclopCreateKernel"},
        {AclProfType::AclopUpdateParams,                        "aclopUpdateParams"},
        {AclProfType::AclopInferShape,                          "aclopInferShape"},
        {AclProfType::AclopCompile,                             "aclopCompile"},
        {AclProfType::AclopCompileAndExecute,                   "aclopCompileAndExecute"},
        {AclProfType::AclopCompileAndExecuteV2,                 "aclopCompileAndExecuteV2"},
        {AclProfType::AclGenGraphAndDumpForOp,                  "aclGenGraphAndDumpForOp"},
        {AclProfType::AclblasGemmEx,                            "aclblasGemmEx"},
        {AclProfType::AclblasCreateHandleForGemmEx,             "aclblasCreateHandleForGemmEx"},
        {AclProfType::AclblasCreateHandleForHgemm,              "aclblasCreateHandleForHgemm"},
        {AclProfType::AclblasHgemm,                             "aclblasHgemm"},
        {AclProfType::AclblasS8gemm,                            "aclblasS8gemm"},
        {AclProfType::AclblasCreateHandleForS8gemm,             "aclblasCreateHandleForS8gemm"},
        {AclProfType::AclblasGemvEx,                            "aclblasGemvEx"},
        {AclProfType::AclblasCreateHandleForGemvEx,             "aclblasCreateHandleForGemvEx"},
        {AclProfType::AclblasHgemv,                             "aclblasHgemv"},
        {AclProfType::AclblasCreateHandleForHgemv,              "aclblasCreateHandleForHgemv"},
        {AclProfType::AclblasCreateHandleForS8gemv,             "aclblasCreateHandleForS8gemv"},
        {AclProfType::AclblasS8gemv,                            "aclblasS8gemv"},
        {AclProfType::AclopCast,                                "aclopCast"},
        {AclProfType::AclopCreateHandleForCast,                 "aclopCreateHandleForCast"},
        {AclProfType::OpCompile,                                "opCompile"},
        {AclProfType::OpCompileAndDump,                         "opCompileAndDump"},
        {AclProfType::AclopDestroyAttr,                         "aclopDestroyAttr"},
        {AclProfType::AclCreateTensorDesc,                      "aclCreateTensorDesc"},
        {AclProfType::AclDestroyTensorDesc,                     "aclDestroyTensorDesc"},
        {AclProfType::AclCreateDataBuffer,                      "aclCreateDataBuffer"},
        {AclProfType::AclopCreateAttr,                          "aclopCreateAttr"},
        {AclProfType::AclrtLaunchCallback,                      "aclrtLaunchCallback"},
        {AclProfType::AclrtProcessReport,                       "aclrtProcessReport"},
        {AclProfType::AclrtCreateContext,                       "aclrtCreateContext"},
        {AclProfType::AclrtDestroyContext,                      "aclrtDestroyContext"},
        {AclProfType::AclrtSetCurrentContext,                   "aclrtSetCurrentContext"},
        {AclProfType::AclrtGetCurrentContext,                   "aclrtGetCurrentContext"},
        {AclProfType::AclrtSetDevice,                           "aclrtSetDevice"},
        {AclProfType::AclrtSetDeviceWithoutTsdVXX,              "aclrtSetDeviceWithoutTsdVXX"},
        {AclProfType::AclrtResetDevice,                         "aclrtResetDevice"},
        {AclProfType::AclrtResetDeviceWithoutTsdVXX,            "aclrtResetDeviceWithoutTsdVXX"},
        {AclProfType::AclrtSynchronizeDevice,                   "aclrtSynchronizeDevice"},
        {AclProfType::AclrtSetTsDevice,                         "aclrtSetTsDevice"},
        {AclProfType::AclrtCreateEvent,                         "aclrtCreateEvent"},
        {AclProfType::AclrtCreateEventWithFlag,                 "aclrtCreateEventWithFlag"},
        {AclProfType::AclrtDestroyEvent,                        "aclrtDestroyEvent"},
        {AclProfType::AclrtRecordEvent,                         "aclrtRecordEvent"},
        {AclProfType::AclrtResetEvent,                          "aclrtResetEvent"},
        {AclProfType::AclrtQueryEvent,                          "aclrtQueryEvent"},
        {AclProfType::AclrtQueryEventStatus,                    "aclrtQueryEventStatus"},
        {AclProfType::AclrtQueryEventWaitStatus,                "aclrtQueryEventWaitStatus"},
        {AclProfType::AclrtSynchronizeEvent,                    "aclrtSynchronizeEvent"},
        {AclProfType::AclrtSetOpWaitTimeout,                    "aclrtSetOpWaitTimeout"},
        {AclProfType::AclrtSetOpExecuteTimeOut,                 "aclrtSetOpExecuteTimeOut"},
        {AclProfType::AclrtSetGroup,                            "aclrtSetGroup"},
        {AclProfType::AclrtGetGroupCount,                       "aclrtGetGroupCount"},
        {AclProfType::AclrtGetAllGroupInfo,                     "aclrtGetAllGroupInfo"},
        {AclProfType::AclrtGetGroupInfoDetail,                  "aclrtGetGroupInfoDetail"},
        {AclProfType::AclMallocMemInner,                        "aclMallocMemInner"},
        {AclProfType::AclrtMallocCached,                        "aclrtMallocCached"},
        {AclProfType::AclrtMemFlush,                            "aclrtMemFlush"},
        {AclProfType::AclrtMemInvalidate,                       "aclrtMemInvalidate"},
        {AclProfType::AclrtFree,                                "aclrtFree"},
        {AclProfType::AclrtMallocHost,                          "aclrtMallocHost"},
        {AclProfType::AclrtFreeHost,                            "aclrtFreeHost"},
        {AclProfType::AclrtMemcpy,                              "aclrtMemcpy"},
        {AclProfType::AclrtMemset,                              "aclrtMemset"},
        {AclProfType::AclrtMemcpyAsync,                         "aclrtMemcpyAsync"},
        {AclProfType::AclrtMemsetAsync,                         "aclrtMemsetAsync"},
        {AclProfType::AclrtDeviceCanAccessPeer,                 "aclrtDeviceCanAccessPeer"},
        {AclProfType::AclrtDeviceEnablePeerAccess,              "aclrtDeviceEnablePeerAccess"},
        {AclProfType::AclrtDeviceDisablePeerAccess,             "aclrtDeviceDisablePeerAccess"},
        {AclProfType::AclrtGetMemInfo,                          "aclrtGetMemInfo"},
        {AclProfType::AclrtMemcpy2d,                            "aclrtMemcpy2d"},
        {AclProfType::AclrtMemcpy2dAsync,                       "aclrtMemcpy2dAsync"},
        {AclProfType::AclrtCreateStream,                        "aclrtCreateStream"},
        {AclProfType::AclrtCreateStreamWithConfig,              "aclrtCreateStreamWithConfig"},
        {AclProfType::AclrtDestroyStream,                       "aclrtDestroyStream"},
        {AclProfType::AclrtDestroyStreamForce,                  "aclrtDestroyStreamForce"},
        {AclProfType::AclrtSynchronizeStream,                   "aclrtSynchronizeStream"},
        {AclProfType::AclrtSynchronizeStreamWithTimeout,        "aclrtSynchronizeStreamWithTimeout"},
        {AclProfType::AclrtStreamQuery,                         "aclrtStreamQuery"},
        {AclProfType::AclrtStreamWaitEvent,                     "aclrtStreamWaitEvent"},
        {AclProfType::AclrtAllocatorCreateDesc,                 "aclrtAllocatorCreateDesc"},
        {AclProfType::AclrtAllocatorDestroyDesc,                "aclrtAllocatorDestroyDesc"},
        {AclProfType::AclrtCtxGetSysParamOpt,                   "aclrtCtxGetSysParamOpt"},
        {AclProfType::AclrtCtxSetSysParamOpt,                   "aclrtCtxSetSysParamOpt"},
        {AclProfType::AclrtGetOverflowStatus,                   "aclrtGetOverflowStatus"},
        {AclProfType::AclrtResetOverflowStatus,                 "aclrtResetOverflowStatus"},
        {AclProfType::AclrtGetDeviceCount,                      "aclrtGetDeviceCount"},
        {AclProfType::AclrtGetDevice,                           "aclrtGetDevice"},
        {AclProfType::AclrtMalloc,                              "aclrtMalloc"},
        {AclProfType::AclrtSetStreamFailureMode,                "aclrtSetStreamFailureMode"},
        {AclProfType::AclrtQueryDeviceStatus,                   "aclrtQueryDeviceStatus"},
        {AclProfType::AclrtReserveMemAddress,                   "aclrtReserveMemAddress"},
        {AclProfType::AclrtReleaseMemAddress,                   "aclrtReleaseMemAddress"},
        {AclProfType::AclrtMallocPhysical,                      "aclrtMallocPhysical"},
        {AclProfType::AclrtFreePhysical,                        "aclrtFreePhysical"},
        {AclProfType::AclrtMapMem,                              "aclrtMapMem"},
        {AclProfType::AclrtUnmapMem,                            "aclrtUnmapMem"},
        {AclProfType::AclrtLaunchKernel,                        "aclrtLaunchKernel"},
        {AclProfType::AclrtMemExportToShareableHandle,          "AclrtMemExportToShareableHandle"},
        {AclProfType::AclrtMemImportFromShareableHandle,        "AclrtMemImportFromShareableHandle"},
        {AclProfType::AclrtMemGetAllocationGranularity,         "AclrtMemGetAllocationGranularity"},
        {AclProfType::AclrtMemSetPidToShareableHandle,          "AclrtMemSetPidToShareableHandle"},
        {AclProfType::AclrtDeviceGetBareTgid,                   "AclrtDeviceGetBareTgid"},
        {AclProfType::AcldvppCreateChannel,                     "acldvppCreateChannel"},
        {AclProfType::AcldvppDestroyChannel,                    "acldvppDestroyChannel"},
        {AclProfType::AcldvppJpegDecodeAsync,                   "acldvppJpegDecodeAsync"},
        {AclProfType::AcldvppJpegEncodeAsync,                   "acldvppJpegEncodeAsync"},
        {AclProfType::AcldvppJpegGetImageInfo,                  "acldvppJpegGetImageInfo"},
        {AclProfType::AcldvppJpegGetImageInfoV2,                "acldvppJpegGetImageInfoV2"},
        {AclProfType::AcldvppJpegPredictEncSize,                "acldvppJpegPredictEncSize"},
        {AclProfType::AcldvppJpegPredictDecSize,                "acldvppJpegPredictDecSize"},
        {AclProfType::AcldvppPngDecodeAsync,                    "acldvppPngDecodeAsync"},
        {AclProfType::AcldvppPngGetImageInfo,                   "acldvppPngGetImageInfo"},
        {AclProfType::AcldvppPngPredictDecSize,                 "acldvppPngPredictDecSize"},
        {AclProfType::AclvdecCreateChannel,                     "aclvdecCreateChannel"},
        {AclProfType::AclvdecDestroyChannel,                    "aclvdecDestroyChannel"},
        {AclProfType::AclvdecSendFrame,                         "aclvdecSendFrame"},
        {AclProfType::AclvdecSendSkippedFrame,                  "aclvdecSendSkippedFrame"},
        {AclProfType::AclvencCreateChannel,                     "aclvencCreateChannel"},
        {AclProfType::AclvencDestroyChannel,                    "aclvencDestroyChannel"},
        {AclProfType::AclvencSendFrame,                         "aclvencSendFrame"},
        {AclProfType::AcldvppVpcResizeAsync,                    "acldvppVpcResizeAsync"},
        {AclProfType::AcldvppVpcCropAsync,                      "acldvppVpcCropAsync"},
        {AclProfType::AcldvppVpcCropAndPasteAsync,              "acldvppVpcCropAndPasteAsync"},
        {AclProfType::AcldvppVpcConvertColorAsync,              "acldvppVpcConvertColorAsync"},
        {AclProfType::AcldvppVpcPyrDownAsync,                   "acldvppVpcPyrDownAsync"},
        {AclProfType::AcldvppVpcBatchCropAsync,                 "acldvppVpcBatchCropAsync"},
        {AclProfType::AcldvppVpcBatchCropAndPasteAsync,         "acldvppVpcBatchCropAndPasteAsync"},
        {AclProfType::AcldvppVpcEqualizeHistAsync,              "acldvppVpcEqualizeHistAsync"},
        {AclProfType::AcldvppVpcMakeBorderAsync,                "acldvppVpcMakeBorderAsync"},
        {AclProfType::AcldvppVpcCalcHistAsync,                  "acldvppVpcCalcHistAsync"},
        {AclProfType::AcldvppVpcCropResizeAsync,                "acldvppVpcCropResizeAsync"},
        {AclProfType::AcldvppVpcBatchCropResizeAsync,           "acldvppVpcBatchCropResizeAsync"},
        {AclProfType::AcldvppVpcCropResizePasteAsync,           "acldvppVpcCropResizePasteAsync"},
        {AclProfType::AcldvppVpcBatchCropResizePasteAsync,      "acldvppVpcBatchCropResizePasteAsync"},
        {AclProfType::AcldvppVpcBatchCropResizeMakeBorderAsync, "acldvppVpcBatchCropResizeMakeBorderAsync"},
        {AclProfType::AcltdtEnqueue,                            "acltdtEnqueue"},
        {AclProfType::AcltdtDequeue,                            "acltdtDequeue"},
        {AclProfType::AcltdtEnqueueData,                        "acltdtEnqueueData"},
        {AclProfType::AcltdtDequeueData,                        "acltdtDequeueData"},
        {AclProfType::AcldvppMalloc,                            "acldvppMalloc"},
        {AclProfType::AcldvppFree,                              "acldvppFree"},
        {AclProfType::AcldvppCreatePicDesc,                     "acldvppCreatePicDesc"},
        {AclProfType::AcldvppDestroyPicDesc,                    "acldvppDestroyPicDesc"},
        {AclProfType::AcldvppCreateRoiConfig,                   "acldvppCreateRoiConfig"},
        {AclProfType::AcldvppDestroyRoiConfig,                  "acldvppDestroyRoiConfig"},
        {AclProfType::AcldvppCreateJpegeConfig,                 "acldvppCreateJpegeConfig"},
        {AclProfType::AcldvppDestroyJpegeConfig,                "acldvppDestroyJpegeConfig"},
        {AclProfType::AcldvppCreateResizeConfig,                "acldvppCreateResizeConfig"},
        {AclProfType::AcldvppDestroyResizeConfig,               "acldvppDestroyResizeConfig"},
        {AclProfType::AcldvppCreateChannelDesc,                 "acldvppCreateChannelDesc"},
        {AclProfType::AcldvppDestroyChannelDesc,                "acldvppDestroyChannelDesc"},
        {AclProfType::AclvdecCreateChannelDesc,                 "aclvdecCreateChannelDesc"},
        {AclProfType::AclvdecDestroyChannelDesc,                "aclvdecDestroyChannelDesc"},
        {AclProfType::AcldvppCreateStreamDesc,                  "acldvppCreateStreamDesc"},
        {AclProfType::AcldvppDestroyStreamDesc,                 "acldvppDestroyStreamDesc"},
        {AclProfType::AclvdecCreateFrameConfig,                 "aclvdecCreateFrameConfig"},
        {AclProfType::AclvdecDestroyFrameConfig,                "aclvdecDestroyFrameConfig"},
        {AclProfType::AclvencCreateChannelDesc,                 "aclvencCreateChannelDesc"},
        {AclProfType::AclvencDestroyChannelDesc,                "aclvencDestroyChannelDesc"},
        {AclProfType::AclvencCreateFrameConfig,                 "aclvencCreateFrameConfig"},
        {AclProfType::AclvencDestroyFrameConfig,                "aclvencDestroyFrameConfig"},
        {AclProfType::AcldvppDestroyBatchPicDesc,               "acldvppDestroyBatchPicDesc"}
};

aclError RegisterType(const uint32_t index) {
    const AclProfType type = static_cast<AclProfType>(index);
    const auto iter = PROF_TYPE_TO_NAMES.find(type);
    if (iter != PROF_TYPE_TO_NAMES.cend()) {
        const auto ret = MsprofRegTypeInfo(MSPROF_REPORT_ACL_LEVEL, index, iter->second.c_str());
        if (ret != MSPROF_ERROR_NONE) {
            ACL_LOG_CALL_ERROR("Registered api type [%u] failed = %d", index, ret);
            return ACL_ERROR_PROFILING_FAILURE;
        }
    }
    return ACL_SUCCESS;
}
AclProfilingManager &AclProfilingManager::GetInstance()
{
    static AclProfilingManager profilingManager;
    return profilingManager;
}

aclError AclProfilingManager::Init()
{
    const std::lock_guard<std::mutex> lk(mutex_);
    const int32_t result = MsprofReportData(static_cast<uint32_t>(MSPROF_MODULE_ACL),
        static_cast<uint32_t>(MSPROF_REPORTER_INIT), nullptr, 0U);
    if (result != 0) {
        ACL_LOG_INNER_ERROR("[Init][ProfEngine]init acl profiling engine failed, errorCode = %d", result);
        return ACL_ERROR_PROFILING_FAILURE;
    }
    const aclError reg_ret = RegisterProfilingType();
    if (reg_ret != 0) {
        ACL_LOG_INNER_ERROR("[Init][ProfEngine]init acl profiling reg failed, errorCode = %d", reg_ret);
        return reg_ret;
    }
    isProfiling_ = true;
    return ACL_SUCCESS;
}

aclError AclProfilingManager::UnInit()
{
    const std::lock_guard<std::mutex> lk(mutex_);
    const int32_t result = MsprofReportData(static_cast<uint32_t>(MSPROF_MODULE_ACL),
        static_cast<uint32_t>(MSPROF_REPORTER_UNINIT), nullptr, 0U);
    if (result != MSPROF_ERROR_NONE) {
        ACL_LOG_CALL_ERROR("[Uninit][ProfEngine]Uninit profiling engine failed, errorCode = %d", result);
        return ACL_ERROR_PROFILING_FAILURE;
    }
    isProfiling_ = false;
    return ACL_SUCCESS;
}

bool AclProfilingManager::IsDeviceListEmpty() const
{
    return deviceList_.empty();
}

aclError AclProfilingManager::ProfilingData(ReporterData &data)
{
    const std::lock_guard<std::mutex> lk(mutex_);
    if (MsprofReportData(static_cast<uint32_t>(MSPROF_MODULE_ACL), static_cast<uint32_t>(MSPROF_REPORTER_REPORT),
        &data, sizeof(ReporterData)) != 0) {
        return ACL_ERROR_PROFILING_FAILURE;
    }
    return ACL_SUCCESS;
}

aclError AclProfilingManager::AddDeviceList(const uint32_t *const deviceIdList, const uint32_t deviceNums)
{
    if (deviceNums == 0U) {
        return ACL_SUCCESS;
    }
    ACL_REQUIRES_NOT_NULL(deviceIdList);
    for (size_t devId = 0U; devId < deviceNums; devId++) {
        if (deviceList_.count(*(deviceIdList + devId)) == 0U) {
            (void)deviceList_.insert(*(deviceIdList + devId));
            ACL_LOG_INFO("device id %u is successfully added in acl profiling", *(deviceIdList + devId));
        }
    }
    return ACL_SUCCESS;
}

aclError AclProfilingManager::RemoveDeviceList(const uint32_t *const deviceIdList, const uint32_t deviceNums)
{
    if (deviceNums == 0U) {
        return ACL_SUCCESS;
    }
    ACL_REQUIRES_NOT_NULL(deviceIdList);
    for (size_t devId = 0U; devId < deviceNums; devId++) {
        const auto iter = deviceList_.find(*(deviceIdList + devId));
        if (iter != deviceList_.end()) {
            (void)deviceList_.erase(iter);
        }
    }
    return ACL_SUCCESS;
}

bool AclProfilingManager::IsDeviceEnable(const uint32_t deviceId) const
{
    return deviceList_.count(deviceId) > 0U;
}

aclError AclProfilingManager::RegisterProfilingType() const
{
    for (uint32_t i = static_cast<uint32_t>(AclProfType::OthersTypeStart) + 1U;
         i < static_cast<uint32_t>(AclProfType::OthersTypeEnd); ++i) {
        const auto ret = RegisterType(i);
        if (ret != ACL_SUCCESS) {
            return ret;
        }
    }
    for (uint32_t i = static_cast<uint32_t>(AclProfType::OpTypeStart) + 1U;
         i < static_cast<uint32_t>(AclProfType::OpTypeEnd); ++i) {
        const auto ret = RegisterType(i);
        if (ret != ACL_SUCCESS) {
            return ret;
        }
    }
    for (uint32_t i = static_cast<uint32_t>(AclProfType::ModelTypeStart) + 1U;
         i < static_cast<uint32_t>(AclProfType::ModelTypeEnd); ++i) {
        const auto ret = RegisterType(i);
        if (ret != ACL_SUCCESS) {
            return ret;
        }
    }
    for (uint32_t i = static_cast<uint32_t>(AclProfType::RtTypeStart) + 1U;
         i < static_cast<uint32_t>(AclProfType::RtTypeEnd); ++i) {
        const auto ret = RegisterType(i);
        if (ret != ACL_SUCCESS) {
            return ret;
        }
    }
    return ACL_SUCCESS;
}

AclProfilingReporter::AclProfilingReporter(const AclProfType apiId) : aclApi_(apiId)
{
    if (AclProfilingManager::GetInstance().AclProfilingIsRun() &&
        (!ge::profiling::ProfilingContext::GetInstance().IsDumpToStdEnabled())) {
        startTime_ = MsprofSysCycleTime();
    }
}


AclProfilingReporter::~AclProfilingReporter() noexcept
{
    if (AclProfilingManager::GetInstance().AclProfilingIsRun() &&
        (!ge::profiling::ProfilingContext::GetInstance().IsDumpToStdEnabled())) {
        // 1000 ^ 3 converts second to nanosecond
        const uint64_t endTime = MsprofSysCycleTime();
        MsprofApi api{};
        api.beginTime = startTime_;
        api.endTime = endTime;
        thread_local static auto tid = mmGetTid();
        api.threadId = static_cast<uint32_t>(tid);
        api.level = MSPROF_REPORT_ACL_LEVEL;
        api.type = static_cast<uint32_t>(aclApi_);
        (void)MsprofReportApi(true, &api);
    }
}
}  // namespace acl
