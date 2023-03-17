// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#define DOCTEST_CONFIG_IMPLEMENT
#include <doctest/impl.h>
#include <unity.h>

extern "C" {
void setUp(void) {}
void tearDown(void) {}
extern void run_unity_tests();
}

int main(int argc, char* argv[]) {
    UnityBegin("dynamix");
    run_unity_tests();
    int unity_result = UnityEnd();
    puts("");
    int doctest_result = doctest::Context(argc, argv).run();
    return unity_result + doctest_result;
}

