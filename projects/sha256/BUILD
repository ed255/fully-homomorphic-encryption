load("@bazel_skylib//:bzl_library.bzl", "bzl_library")
load("//transpiler/rust:fhe_rs.bzl", "fhe_rust_library")
load("@rules_rust//rust:defs.bzl", "rust_binary", "rust_library", "rust_test")

licenses(["notice"])

fhe_rust_library(
    name = "sha256_rs_fhe_lib",
    lut_size = 0,
    loop_unroll_rlimit = 100,
    src = "sha256.cc",
    hdrs = ["sha256.h"],
)

rust_library(
    name = "sha256_rs_lib",
    srcs = ["sha256_rs_lib.rs"],
    deps = [
        ":sha256_rs_fhe_lib",
        "@crate_index//:phantom-zone",
    ],
)

rust_binary(
    name = "sha256_rs_main",
    srcs = ["sha256_rs_lib.rs"],
    deps = [
        ":sha256_rs_fhe_lib",
        "@crate_index//:phantom-zone",
    ],
)

exports_files(
    [
        "sha256.h",
        "sha256.cc",
    ],
)
