/**
* @file main.cpp
*
* Copyright (C) 2020. Huawei Technologies Co., Ltd. All rights reserved.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
*/
#include <iostream>
#include "sample_process.h"
#include "utils.h"

using namespace std;
bool g_isDevice = false;

int main()
{
    SampleProcess sampleProcess;
    Result ret = sampleProcess.InitResource();

    if (ret != SUCCESS) {
        ERROR_LOG("sample init resource failed");
        return FAILED;
    }

    ret = sampleProcess.Process();
    if (ret != SUCCESS) {
        ERROR_LOG("sample process failed");
        return FAILED;
    } 
    else {
        INFO_LOG("execute sample success");
    }

    ret = sampleProcess.DemoProcess();
    if (ret != SUCCESS) {
        ERROR_LOG("sample demo process failed");
        return FAILED;
    }
    else {
        INFO_LOG("execute encrypted sample success");
    }
    return SUCCESS;
}