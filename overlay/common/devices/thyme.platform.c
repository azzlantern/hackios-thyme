/*
 * thyme: paste into xnuloader/loader.c next to platform_redwood and add &platform_thyme to platforms[].
 * Every value is explained in docs/PORTING.md ("The loader").
 */
static const struct platform platform_thyme = {
	.name = "thyme",
	.io_base = 0x00400000, .io_size = 0x3FC00000,
	.gic_dist = 0x17A00000, .gic_redist = 0x17A60000,
	.uart = 0,
	.log_other_cpu = 1,
	.cores = 8, .core_shift = 8,
	.small_cores = 0x0f,
	.fb_base = 0x9c000000,
};
