/*
 * rpmh-rails.c — print the RPMh command database's VRM entries from the kernel.
 *
 * Kept from the first MI 10S touchscreen attempt (2026-10-24). It is not a driver: it is
 * the tool that answers "which rails does this phone actually have" when a supply lookup
 * fails. The umi port's umi_rpmh.c refuses to continue when a rail it asks for is not in
 * the command database:
 *
 *   UMI RPMH: <rail> lookup matches=0; refusing
 *
 * That is what stopped the Mi 10 Ultra's Wi-Fi. The Mi 10S device tree asks for the same
 * rails the Mi 10 port uses, and a parsed copy of devices/thyme/prebuilt/device-tree.dtb
 * agrees on all seven:
 *
 *   WLAN IO  smpa4   WLAN AON smpa6   WLAN digital smpf2
 *   WLAN RFA1 smpa5  WLAN RFA2 smpc8  PCIe 1p8 ldoa9  PCIe 0p9 ldoa5
 *
 * so nothing needs changing today. This file is what to reach for after a firmware update
 * changes the database, or on another board of the same family.
 *
 * How to use it: drop it next to umi_rpmh.c (devices/thyme/overlays/xnu/files/bsd/dev/),
 * add it to bsd/conf/files, and call rpmh_dump_vrm_rails() once from umi_rpmh_init().
 * The types below come from uart.h / cmd_db, as in the original driver.
 */

/* Verbatim from the 2026-10-24 attempt; the surrounding declarations live in
 * uart.h (struct cmd_db_header, cmd_db_slave, cmd_db_entry, CMD_DB_SIZE) and in
 * umi_rpmh.c (the `cmd_db` base pointer and the locking). Keep the debug print
 * prefix short: the loader's log window is 32 KB. */

static void
rpmh_dump_vrm_rails(void)
{
	const struct cmd_db_header *db = (const struct cmd_db_header *)cmd_db;

	if (db->magic[0] != 0xdb || db->magic[1] != 0x30 || db->magic[2] != 0x03 || db->magic[3] != 0x0c) {
		printf("rpmh: command database magic is not db 30 03 0c; no rails to list\n");
		return;
	}
	for (unsigned s = 0; s < 8; s++) {
		const struct cmd_db_slave *slave = &db->slave[s];
		const struct cmd_db_entry *entry = (const struct cmd_db_entry *)(db->data + slave->header_offset);

		if (slave->id == 0 || slave->header_offset >= CMD_DB_SIZE - 0x1000 || slave->count > 512) {
			continue;
		}
		for (unsigned i = 0; i < slave->count; i++) {
			/* type 4 = VRM: what a regulator vote names */
			if ((entry[i].address >> 16 & 0xf) == 4) {
				printf("rpmh: VRM %.8s=%#x\n", entry[i].id, entry[i].address);
			}
		}
	}
}

/*
 * Compare the list above with the rails[] table in umi_rpmh.c. A rail the kernel asks
 * for but the database lacks has to be replaced by one it has, at the voltage the device
 * tree names — never by a guess: a wrong rail on a Wi-Fi supply leaves the controller
 * unpowered while every other line of the log still looks healthy.
 */
