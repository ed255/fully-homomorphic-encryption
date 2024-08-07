load("@bazel_skylib//:bzl_library.bzl", "bzl_library")
load("//transpiler/rust:fhe_rs.bzl", "fhe_rust_library")
load("@rules_rust//rust:defs.bzl", "rust_binary", "rust_library", "rust_test")
load("@rules_cc//cc:defs.bzl", "cc_test") 

licenses(["notice"])

fhe_rust_library(
    name = "demowithtests_rs_fhe_lib",
    lut_size = 0,
    src = "demowithtests.cc",
    hdrs = ["demowithtests.h"],
)

rust_library(
    name = "demowithtests_rs_lib",
    srcs = ["demowithtests_rs_lib.rs"],
    deps = [
        ":demowithtests_rs_fhe_lib",
        "@crate_index//:phantom-zone",
    ],
)

rust_binary(
    name = "demowithtests_rs_main",
    srcs = ["demowithtests_rs_lib.rs"],
    deps = [
        ":demowithtests_rs_fhe_lib",
        "@crate_index//:phantom-zone",
    ],
)

cc_test(
    name = "demowithtests_test",
    srcs = [
        "demowithtests_test.cc",
        "tinytest.h",
        "demowithtests.cc",
        "demowithtests.h",
    ],
)

exports_files(
    [
        "demowithtests.h",
        "demowithtests.cc",
    ],
)
