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
#include <functional>

using namespace utest::v1;
using namespace std;

static control_t readTestingWithDifferentResourceOpDefined()
{
    int valI = 1;

    Resource resRd(valI, ResourceOp::RES_RD);
    Resource resWr(valI, ResourceOp::RES_WR);
    Resource resRdWr(valI, ResourceOp::RES_RDWR);
    Resource resE(valI, ResourceOp::RES_E);

    TEST_ASSERT_EQUAL(valI, *resRd.Read<int>());
    TEST_ASSERT_EQUAL(nullptr, resWr.Read<int>());
    TEST_ASSERT_EQUAL(BAD_EXPECTED_ACCESS, resWr.GetErrorCode());
    TEST_ASSERT_EQUAL(valI, *resRdWr.Read<int>());
    TEST_ASSERT_EQUAL(nullptr, resE.Read<int>());
    TEST_ASSERT_EQUAL(BAD_EXPECTED_ACCESS, resE.GetErrorCode());

    return CaseNext;
}

static control_t writeTestingWithDifferentResourceOpDefined()
{
    int valI = 1;

    Resource resRd(valI, ResourceOp::RES_RD);
    Resource resWr(valI, ResourceOp::RES_WR);
    Resource resRdWr(valI, ResourceOp::RES_RDWR);
    Resource resE(valI, ResourceOp::RES_E);

    TEST_ASSERT_EQUAL(BAD_EXPECTED_ACCESS, resRd.Write<int>(1));
    TEST_ASSERT_EQUAL(BAD_EXPECTED_ACCESS, resRd.GetErrorCode());
    TEST_ASSERT_EQUAL(RES_SUCCESS, resWr.Write<int>(1));
    TEST_ASSERT_EQUAL(RES_SUCCESS, resRdWr.Write<int>(1));
    TEST_ASSERT_EQUAL(BAD_EXPECTED_ACCESS, resE.Write<int>(1));
    TEST_ASSERT_EQUAL(BAD_EXPECTED_ACCESS, resE.GetErrorCode());

    return CaseNext;
}

static control_t execTestingWithDifferentResourceOpDefined()
{
    int valI = 1;

    Resource resRd(valI, ResourceOp::RES_RD);
    Resource resWr(valI, ResourceOp::RES_WR);
    Resource resRdWr(valI, ResourceOp::RES_RDWR);
    Resource resE(valI, ResourceOp::RES_E);

    TEST_ASSERT_EQUAL(BAD_EXPECTED_ACCESS, resRd.Exec<int>());
    TEST_ASSERT_EQUAL(BAD_EXPECTED_ACCESS, resRd.GetErrorCode());
    TEST_ASSERT_EQUAL(BAD_EXPECTED_ACCESS, resWr.Exec<int>());
    TEST_ASSERT_EQUAL(BAD_EXPECTED_ACCESS, resWr.GetErrorCode());
    TEST_ASSERT_EQUAL(BAD_EXPECTED_ACCESS, resRdWr.Exec<int>());
    TEST_ASSERT_EQUAL(BAD_EXPECTED_ACCESS, resRdWr.GetErrorCode());
    TEST_ASSERT_EQUAL(RES_SUCCESS, resE.Exec<int>());

    return CaseNext;
}

static control_t callbackTestingWithDifferentResourceOpDefined()
{
    int valI = 1;

    Resource resRd(valI, ResourceOp::RES_RD);
    Resource resWr(valI, ResourceOp::RES_WR);
    Resource resRdWr(valI, ResourceOp::RES_RDWR);
    Resource resE(valI, ResourceOp::RES_E);

    TEST_ASSERT_EQUAL(nullptr, resRd.BindOnWrite<int>([](int a){}).get());
    TEST_ASSERT_EQUAL(BAD_EXPECTED_ACCESS, resRd.GetErrorCode());
    TEST_ASSERT_NOT_EQUAL(nullptr, resRd.BindOnRead<int>([](int a){}).get());
    TEST_ASSERT_EQUAL(nullptr, resRd.BindOnExec<int>([](int a){}).get());
    TEST_ASSERT_EQUAL(BAD_EXPECTED_ACCESS, resRd.GetErrorCode());

    TEST_ASSERT_NOT_EQUAL(nullptr, resWr.BindOnWrite<int>([](int a){}).get());
    TEST_ASSERT_EQUAL(nullptr, resWr.BindOnRead<int>([](int a){}).get());
    TEST_ASSERT_EQUAL(BAD_EXPECTED_ACCESS, resWr.GetErrorCode());
    TEST_ASSERT_EQUAL(nullptr, resWr.BindOnExec<int>([](int a){}).get());
    TEST_ASSERT_EQUAL(BAD_EXPECTED_ACCESS, resWr.GetErrorCode());

    TEST_ASSERT_NOT_EQUAL(nullptr, resRdWr.BindOnWrite<int>([](int a){}).get());
    TEST_ASSERT_NOT_EQUAL(nullptr, resRdWr.BindOnRead<int>([](int a){}).get());
    TEST_ASSERT_EQUAL(nullptr, resRdWr.BindOnExec<int>([](int a){}).get());
    TEST_ASSERT_EQUAL(BAD_EXPECTED_ACCESS, resRdWr.GetErrorCode());

    TEST_ASSERT_EQUAL(nullptr, resE.BindOnWrite<int>([](int a){}).get());
    TEST_ASSERT_EQUAL(BAD_EXPECTED_ACCESS, resE.GetErrorCode());
    TEST_ASSERT_EQUAL(nullptr, resE.BindOnRead<int>([](int a){}).get());
    TEST_ASSERT_EQUAL(BAD_EXPECTED_ACCESS, resE.GetErrorCode());
    TEST_ASSERT_NOT_EQUAL(nullptr, resE.BindOnExec<int>([](int a){}).get());

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
    Case("Test read operation on different operation defined resource", readTestingWithDifferentResourceOpDefined),
    Case("Test write operation on different operation defined resource", writeTestingWithDifferentResourceOpDefined),
    Case("Test exec operation on different operation defined resource", execTestingWithDifferentResourceOpDefined),
    Case("Test callback binding on different operation defined resource", callbackTestingWithDifferentResourceOpDefined)};

Specification specification(greentea_setup, cases);

int main()
{
    return !Harness::run(specification);
}