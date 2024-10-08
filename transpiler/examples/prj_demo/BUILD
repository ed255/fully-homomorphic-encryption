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

exports_files(
    [
        "demo.h",
        "demo.cc",
    ],
)
