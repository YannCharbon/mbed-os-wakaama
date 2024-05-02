/**
 *  @file main.cpp
 *  @brief Test of the read, write and execute operation on non void resource
 *
 *  @author Bastien Pillonel
 *
 *  @date 4/30/2024
 */

#include "mbed.h"
#include "utest/utest.h"
#include "unity/unity.h"
#include "greentea-client/test_env.h"

#include "resource.h"
#include <string>

using namespace utest::v1;
using namespace std;

static control_t readTesting(){
    int valI = 1;
    double valD = 3.14;
    float valF = 7.2f;
    string valS("hello");

    Resource resInt(valI, ResourceOp::RES_RD);
    Resource resDouble(valD, ResourceOp::RES_RD);
    Resource resFloat(valF, ResourceOp::RES_RD);
    Resource resString(valS, ResourceOp::RES_RD);

    TEST_ASSERT_EQUAL(valI, *resInt.Read<int>());
    TEST_ASSERT_EQUAL(valD, *resDouble.Read<double>());
    TEST_ASSERT_EQUAL(valF, *resFloat.Read<float>());
    TEST_ASSERT_EQUAL_STRING(valS.c_str(), (*resString.Read<string>()).c_str());
    
    return CaseNext;
}

static control_t writeTesting(){
    int valI = 1;
    double valD = 3.14;
    float valF = 7.2f;
    string valS("hello");

    Resource resInt(0, ResourceOp::RES_WR);
    Resource resDouble(0., ResourceOp::RES_WR);
    Resource resFloat(0.f, ResourceOp::RES_WR);
    Resource resString(string(""), ResourceOp::RES_WR);

    TEST_ASSERT_EQUAL(RES_SUCCESS, resInt.Write<int>(valI));
    TEST_ASSERT_EQUAL(RES_SUCCESS, resDouble.Write<double>(valD));
    TEST_ASSERT_EQUAL(RES_SUCCESS, resFloat.Write<float>(valF));
    TEST_ASSERT_EQUAL(RES_SUCCESS, resString.Write<string>(valS));
   
    return CaseNext;
}

static control_t readWriteTesting(){
    int valI = 1;
    double valD = 3.14;
    float valF = 7.2f;
    string valS("hello");

    Resource resInt(0, ResourceOp::RES_RDWR);
    Resource resDouble(0., ResourceOp::RES_RDWR);
    Resource resFloat(0.f, ResourceOp::RES_RDWR);
    Resource resString(string(""), ResourceOp::RES_RDWR);

    TEST_ASSERT_EQUAL(RES_SUCCESS, resInt.Write<int>(valI));
    TEST_ASSERT_EQUAL(RES_SUCCESS, resDouble.Write<double>(valD));
    TEST_ASSERT_EQUAL(RES_SUCCESS, resFloat.Write<float>(valF));
    TEST_ASSERT_EQUAL(RES_SUCCESS, resString.Write<string>(valS));

    TEST_ASSERT_EQUAL(valI, *resInt.Read<int>());
    TEST_ASSERT_EQUAL(valD, *resDouble.Read<double>());
    TEST_ASSERT_EQUAL(valF, *resFloat.Read<float>());
    TEST_ASSERT_EQUAL_STRING(valS.c_str(), (*resString.Read<string>()).c_str());

    return CaseNext;
}

static control_t execTesting(){
    Resource resInt(0, ResourceOp::RES_E);
    Resource resDouble(0., ResourceOp::RES_E);
    Resource resFloat(0.f, ResourceOp::RES_E);
    Resource resString(string(""), ResourceOp::RES_E);

    TEST_ASSERT_EQUAL(RES_SUCCESS, resInt.Exec<int>());
    TEST_ASSERT_EQUAL(RES_SUCCESS, resDouble.Exec<double>());
    TEST_ASSERT_EQUAL(RES_SUCCESS, resFloat.Exec<float>());
    TEST_ASSERT_EQUAL(RES_SUCCESS, resString.Exec<string>());

    return CaseNext;
}

utest::v1::status_t greentea_setup(const size_t number_of_cases)
{
    // Here, we specify the timeout (60s) and the host test (a built-in host test or the name of our Python file)
    GREENTEA_SETUP(60, "default_auto");

    return greentea_test_setup_handler(number_of_cases);
}

// List of test cases in this file
Case cases[] = {
    Case("Read operation on multiple resources", readTesting),
    Case("Write operation on multiple resources", writeTesting),
    Case("Read and write operation on multiple resources", readWriteTesting),
    Case("Exec operation on multiple resources", execTesting)
};

Specification specification(greentea_setup, cases);

int main()
{
    return !Harness::run(specification);
}