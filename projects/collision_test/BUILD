load("@bazel_skylib//:bzl_library.bzl", "bzl_library")
load("//transpiler/rust:fhe_rs.bzl", "fhe_rust_library")
load("@rules_rust//rust:defs.bzl", "rust_binary", "rust_library", "rust_test")

licenses(["notice"])

fhe_rust_library(
    name = "collision_test_rs_fhe_lib",
    lut_size = 0,
    loop_unroll_rlimit = 256,
    src = "collision_test.cc",
    hdrs = ["collision_test.h"],
)

rust_library(
    name = "collision_test_rs_lib",
    srcs = ["collision_test_rs_lib.rs"],
    deps = [
        ":collision_test_rs_fhe_lib",
        "@crate_index//:phantom-zone",
    ],
)

rust_binary(
    name = "collision_test_rs_main",
    srcs = ["collision_test_rs_lib.rs"],
    deps = [
        ":collision_test_rs_fhe_lib",
        "@crate_index//:phantom-zone",
    ],
)

exports_files(
    [
        "collision_test.h",
        "collision_test.cc",
    ],
)
