/**
 * @file
 * @brief
 *
 * @date 03.11.10
 * @author Anton Bondarev
 */

#ifndef DEVICE_H_
#define DEVICE_H_


#define has_file
#include <fs/idesc.h>
struct idesc;
struct file {
	struct idesc f_idesc;
};

#include <drivers/device.h>

#include <util/array.h>
#define CHAR_DEV_DEF(cname, file_op, idesc_op, init_func) \
	ARRAY_SPREAD_DECLARE(const struct dev_module, __device_registry); \
	ARRAY_SPREAD_ADD(__device_registry, { \
			.name = cname, \
			.fops = file_op, \
			.init = init_func})

extern int char_dev_init_all(void);
extern int char_dev_register(const char *name, const struct file_operations *ops);
extern int char_dev_idesc_fstat(struct idesc *idesc, void *buff);

#endif /* DEVICE_H_ */
