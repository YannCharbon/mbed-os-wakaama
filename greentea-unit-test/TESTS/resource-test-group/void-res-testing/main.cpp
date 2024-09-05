/**
 *  @file main.cpp
 *  @brief Test creation of a void resource object
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

using namespace utest::v1;

static control_t createVoidResource(){
    Resource res;
    TEST_ASSERT_EQUAL(res.Empty(), true);
    return CaseNext;
}

static control_t writeVoidResource(){
    // A void ressource is read only
    Resource res;
    TEST_ASSERT_EQUAL(res.Write<int>(1), BAD_EXPECTED_ACCESS);
    TEST_ASSERT_EQUAL(res.GetErrorCode(), BAD_EXPECTED_ACCESS);
    TEST_ASSERT_EQUAL(res.Empty(), true);
    return CaseNext;
}

static control_t readVoidResource(){
    Resource res;
    TEST_ASSERT_EQUAL(res.Read<int>(), nullptr);
    TEST_ASSERT_EQUAL(res.GetErrorCode(), VALUE_IS_EMPTY);
    TEST_ASSERT_EQUAL(res.Empty(), true);
    return CaseNext;
}

static control_t execVoidResource(){
    Resource res;
    TEST_ASSERT_EQUAL(res.Exec<int>(), VALUE_IS_EMPTY);
    TEST_ASSERT_EQUAL(res.GetErrorCode(), VALUE_IS_EMPTY);
    TEST_ASSERT_EQUAL(res.Empty(), true);
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
    Case("Create a void resource", createVoidResource),
    Case("Write operation on a void resource", writeVoidResource),
    Case("Read operation on a void resource", readVoidResource),
    Case("Exec operation on a void resource", execVoidResource)
};

Specification specification(greentea_setup, cases);

int main()
{
    return !Harness::run(specification);
}