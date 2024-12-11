const std = @import("std");

pub fn build(b: *std.Build) void {
    const target = b.standardTargetOptions(.{});
    const optimize = b.standardOptimizeOption(.{ .preferred_optimize_mode = .ReleaseFast });
    const version = std.SemanticVersion.parse("0.3.0") catch unreachable;

    const lib = b.addStaticLibrary(.{
        .name = "aegis",
        .target = target,
        .optimize = optimize,
        .strip = true,
        .version = version,
    });

    lib.linkLibC();

    const lib_options = b.addOptions();

    const favor_performance: bool = b.option(bool, "favor-performance", "Favor performance over side channel mitigations") orelse false;
    lib_options.addOption(bool, "favor_performance", favor_performance);
    if (favor_performance) {
        lib.defineCMacro("FAVOR_PERFORMANCE", "1");
    }

    const with_benchmark: bool = b.option(bool, "with-benchmark", "Compile benchmark") orelse false;
    lib_options.addOption(bool, "benchmark", with_benchmark);

    lib.addIncludePath(b.path("src/include"));

    const source_files = &.{
        "src/libaegis.c",
    };

    lib.addCSourceFiles(.{ .files = source_files });

    // This declares intent for the executable to be installed into the
    // standard location when the user invokes the "install" step (the default
    // step when running `zig build`).
    b.installArtifact(lib);

    b.installDirectory(.{
        .install_dir = .header,
        .install_subdir = "",
        .source_dir = b.path("src/include"),
    });

    // Creates a step for unit testing. This only builds the test executable
    // but does not run it.
    const main_tests = b.addTest(.{
        .root_source_file = b.path("src/test/main.zig"),
        .target = target,
        .optimize = optimize,
    });

    main_tests.addIncludePath(b.path("src/include"));
    main_tests.linkLibrary(lib);

    const run_main_tests = b.addRunArtifact(main_tests);

    const test_step = b.step("test", "Run library tests");
    test_step.dependOn(&run_main_tests.step);

    if (with_benchmark) {
        const benchmark = b.addExecutable(.{
            .name = "benchmark",
            .root_source_file = b.path("src/test/benchmark.zig"),
            .target = target,
            .optimize = optimize,
        });
        benchmark.addIncludePath(b.path("src/include"));
        benchmark.linkLibrary(lib);
        b.installArtifact(benchmark);
    }
}
