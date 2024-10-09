load("@bazel_skylib//:bzl_library.bzl", "bzl_library")
load("//transpiler/rust:fhe_rs.bzl", "fhe_rust_library")
load("@rules_rust//rust:defs.bzl", "rust_binary", "rust_library", "rust_test")

licenses(["notice"])

fhe_rust_library(
    name = "getview_test_rs_fhe_lib",
    lut_size = 0,
    loop_unroll_rlimit = 256,
    src = "getview_test.cc",
    hdrs = ["getview_test.h"],
)

rust_library(
    name = "getview_test_rs_lib",
    srcs = ["getview_test_rs_lib.rs"],
    deps = [
        ":getview_test_rs_fhe_lib",
        "@crate_index//:phantom-zone",
    ],
)

rust_binary(
    name = "getview_test_rs_main",
    srcs = ["getview_test_rs_lib.rs"],
    deps = [
        ":getview_test_rs_fhe_lib",
        "@crate_index//:phantom-zone",
    ],
)

exports_files(
    [
        "getview_test.h",
        "getview_test.cc",
    ],
)
