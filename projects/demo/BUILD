load("@bazel_skylib//:bzl_library.bzl", "bzl_library")
load("//transpiler/rust:fhe_rs.bzl", "fhe_rust_library")
load("@rules_rust//rust:defs.bzl", "rust_binary", "rust_library", "rust_test")

licenses(["notice"])

fhe_rust_library(
    name = "demo_rs_fhe_lib",
    lut_size = 0,
    src = "demo.cc",
    hdrs = ["demo.h"],
)

rust_library(
    name = "demo_rs_lib",
    srcs = ["demo_rs_lib.rs"],
    deps = [
        ":demo_rs_fhe_lib",
        "@crate_index//:phantom-zone",
    ],
)

rust_binary(
    name = "demo_rs_main",
    srcs = ["demo_rs_lib.rs"],
    deps = [
        ":demo_rs_fhe_lib",
        "@crate_index//:phantom-zone",
    ],
)

exports_files(
    [
        "demo.h",
        "demo.cc",
    ],
)
