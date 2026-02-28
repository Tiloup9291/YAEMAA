#include "ethercatPDOs.h"

const ec_pdo_entry_info_t slave_0_pdo_entries[] = {
	{0x7000, 0x00, 16},
};
const ec_pdo_entry_info_t slave_0_pdo_entries2[] = {
	{0x6000, 0x00, 16},
};

const ec_pdo_info_t slave_0_pdos[] = {
	{0x1600, 1, &slave_0_pdo_entries[0]},
};
const ec_pdo_info_t slave_0_pdos2[] = {
	{0x1a00, 1, &slave_0_pdo_entries2[0]},
};

const ec_sync_info_t slave_0_sync[] = {
	{0, EC_DIR_OUTPUT, 0, NULL, EC_WD_DISABLE},
	{1, EC_DIR_INPUT, 0, NULL, EC_WD_DISABLE},
	{2, EC_DIR_OUTPUT, 1, &slave_0_pdos[0], EC_WD_ENABLE},
	{3, EC_DIR_INPUT, 1, &slave_0_pdos2[0], EC_WD_DISABLE},
	{0xff}
};

unsigned int off_do = 0;
unsigned int off_do2 = 0;
unsigned int off_di = 0;
unsigned int off_di2 = 0;

ec_pdo_entry_reg_t domain_regs[] = {
	{0, 0, 0x00000b95, 0x00001500, 0x7000, 0x00, &off_do, NULL},
	{0, 0, 0x00000b95, 0x00001500, 0x6000, 0x00, &off_di, NULL},
	{}
};

