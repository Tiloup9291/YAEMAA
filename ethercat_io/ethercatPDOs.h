#ifndef _ETHERCATPDOS_H_
#define _ETHERCATPDOS_H_

#include <ecrt.h>

extern const ec_pdo_entry_info_t slave_0_pdo_entries[];
extern const ec_pdo_entry_info_t slave_0_pdo_entries2[];

extern const ec_pdo_info_t slave_0_pdos[];
extern const ec_pdo_info_t slave_0_pdos2[];

extern const ec_sync_info_t slave_0_sync[];

extern unsigned int off_do;
extern unsigned int off_do2;
extern unsigned int off_di;
extern unsigned int off_di2;

extern ec_pdo_entry_reg_t domain_regs[];

#endif /* _ETHERCATPDOS_H_ */
