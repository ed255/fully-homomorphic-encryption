load("@bazel_skylib//:bzl_library.bzl", "bzl_library")
load("//transpiler/rust:fhe_rs.bzl", "fhe_rust_library")
load("@rules_rust//rust:defs.bzl", "rust_binary", "rust_library", "rust_test")

licenses(["notice"])

fhe_rust_library(
    name = "move_test_rs_fhe_lib",
    lut_size = 0,
    src = "move_test.cc",
    hdrs = ["move_test.h"],
)

rust_library(
    name = "move_test_rs_lib",
    srcs = ["move_test_rs_lib.rs"],
    deps = [
        ":move_test_rs_fhe_lib",
        "@crate_index//:phantom-zone",
    ],
)

rust_binary(
    name = "move_test_rs_main",
    srcs = ["move_test_rs_lib.rs"],
    deps = [
        ":move_test_rs_fhe_lib",
        "@crate_index//:phantom-zone",
    ],
)

exports_files(
    [
        "move_test.h",
        "move_test.cc",
    ],
)
