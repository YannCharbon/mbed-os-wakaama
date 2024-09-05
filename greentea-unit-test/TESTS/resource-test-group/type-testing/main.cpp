/**
 *  @file main.cpp
 *  @brief Test behaviour when accessing resource with different operation
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

static control_t readTestingWithDifferentType()
{
    int valI = 1;

    Resource resRd(valI, ResourceOp::RES_RD);

    TEST_ASSERT_EQUAL(valI, *resRd.Read<int>());
    TEST_ASSERT_EQUAL(nullptr, resRd.Read<double>());
    TEST_ASSERT_EQUAL(VALUE_TYPE_NOT_CORRESPONDING, resRd.GetErrorCode());
    TEST_ASSERT_EQUAL(nullptr, resRd.Read<float>());
    TEST_ASSERT_EQUAL(VALUE_TYPE_NOT_CORRESPONDING, resRd.GetErrorCode());
    TEST_ASSERT_EQUAL(nullptr, resRd.Read<string>());
    TEST_ASSERT_EQUAL(VALUE_TYPE_NOT_CORRESPONDING, resRd.GetErrorCode());

    return CaseNext;
}

static control_t writeTestingWithDifferentType()
{
    int valI = 1;

    Resource resWr(valI, ResourceOp::RES_WR);

    TEST_ASSERT_EQUAL(RES_SUCCESS, resWr.Write<int>(1));
    TEST_ASSERT_EQUAL(VALUE_TYPE_NOT_CORRESPONDING, resWr.Write<double>(1.3));
    TEST_ASSERT_EQUAL(VALUE_TYPE_NOT_CORRESPONDING, resWr.GetErrorCode());
    TEST_ASSERT_EQUAL(VALUE_TYPE_NOT_CORRESPONDING, resWr.Write<float>(1.2f));
    TEST_ASSERT_EQUAL(VALUE_TYPE_NOT_CORRESPONDING, resWr.GetErrorCode());
    TEST_ASSERT_EQUAL(VALUE_TYPE_NOT_CORRESPONDING, resWr.Write<string>(string("")));
    TEST_ASSERT_EQUAL(VALUE_TYPE_NOT_CORRESPONDING, resWr.GetErrorCode());

    return CaseNext;
}

static control_t execTestingWithDifferentType()
{
    int valI = 1;

    Resource resE(valI, ResourceOp::RES_E);

    TEST_ASSERT_EQUAL(RES_SUCCESS, resE.Exec<int>());
    TEST_ASSERT_EQUAL(VALUE_TYPE_NOT_CORRESPONDING, resE.Exec<double>());
    TEST_ASSERT_EQUAL(VALUE_TYPE_NOT_CORRESPONDING, resE.GetErrorCode());
    TEST_ASSERT_EQUAL(VALUE_TYPE_NOT_CORRESPONDING, resE.Exec<float>());
    TEST_ASSERT_EQUAL(VALUE_TYPE_NOT_CORRESPONDING, resE.GetErrorCode());
    TEST_ASSERT_EQUAL(VALUE_TYPE_NOT_CORRESPONDING, resE.Exec<string>());
    TEST_ASSERT_EQUAL(VALUE_TYPE_NOT_CORRESPONDING, resE.GetErrorCode());

    return CaseNext;
}

static control_t callbackTestingWithDifferentType()
{
    int valI = 1;

    Resource resRd(valI, ResourceOp::RES_RD);
    Resource resWr(valI, ResourceOp::RES_WR);
    Resource resE(valI, ResourceOp::RES_E);

    TEST_ASSERT_NOT_EQUAL(nullptr, resRd.BindOnRead<int>([](int a){}).get());
    TEST_ASSERT_EQUAL(nullptr, resRd.BindOnRead<double>([](double a){}).get());
    TEST_ASSERT_EQUAL(VALUE_TYPE_NOT_CORRESPONDING, resRd.GetErrorCode());
    TEST_ASSERT_EQUAL(nullptr, resRd.BindOnRead<float>([](float a){}).get());
    TEST_ASSERT_EQUAL(VALUE_TYPE_NOT_CORRESPONDING, resRd.GetErrorCode());
    TEST_ASSERT_EQUAL(nullptr, resRd.BindOnRead<string>([](string a){}).get());
    TEST_ASSERT_EQUAL(VALUE_TYPE_NOT_CORRESPONDING, resRd.GetErrorCode());

    TEST_ASSERT_NOT_EQUAL(nullptr, resWr.BindOnWrite<int>([](int a){}).get());
    TEST_ASSERT_EQUAL(nullptr, resWr.BindOnWrite<double>([](double a){}).get());
    TEST_ASSERT_EQUAL(VALUE_TYPE_NOT_CORRESPONDING, resWr.GetErrorCode());
    TEST_ASSERT_EQUAL(nullptr, resWr.BindOnWrite<float>([](float a){}).get());
    TEST_ASSERT_EQUAL(VALUE_TYPE_NOT_CORRESPONDING, resWr.GetErrorCode());
    TEST_ASSERT_EQUAL(nullptr, resWr.BindOnWrite<string>([](string a){}).get());
    TEST_ASSERT_EQUAL(VALUE_TYPE_NOT_CORRESPONDING, resWr.GetErrorCode());

    TEST_ASSERT_NOT_EQUAL(nullptr, resE.BindOnExec<int>([](int a){}).get());
    TEST_ASSERT_EQUAL(nullptr, resE.BindOnExec<double>([](double a){}).get());
    TEST_ASSERT_EQUAL(VALUE_TYPE_NOT_CORRESPONDING, resE.GetErrorCode());
    TEST_ASSERT_EQUAL(nullptr, resE.BindOnExec<float>([](float a){}).get());
    TEST_ASSERT_EQUAL(VALUE_TYPE_NOT_CORRESPONDING, resE.GetErrorCode());
    TEST_ASSERT_EQUAL(nullptr, resE.BindOnExec<string>([](string a){}).get());
    TEST_ASSERT_EQUAL(VALUE_TYPE_NOT_CORRESPONDING, resE.GetErrorCode());

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
    Case("Test read operation on different type defined resource", readTestingWithDifferentType),
    Case("Test write operation on different type defined resource", writeTestingWithDifferentType),
    Case("Test exec operation on different type defined resource", execTestingWithDifferentType),
    Case("Test callback binding on different type defined resource", callbackTestingWithDifferentType)};

Specification specification(greentea_setup, cases);

int main()
{
    return !Harness::run(specification);
}