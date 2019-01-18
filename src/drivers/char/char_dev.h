/**
 * @file
 * @brief
 *
 * @date 03.11.10
 * @author Anton Bondarev
 */

#ifndef DEVICE_H_
#define DEVICE_H_

#include <fs/idesc.h>
#include <fs/file_operation.h>
#include <drivers/device.h>
#include <util/array.h>

#define CHAR_DEV_DEF(chname, open_fn, close_fn, idesc_op, priv) \
	ARRAY_SPREAD_DECLARE(const struct dev_module, __device_registry); \
	ARRAY_SPREAD_ADD(__device_registry, { \
			.name = chname, \
			.dev_priv = priv, \
			.dev_iops = idesc_op, \
			.open = open_fn, \
			.close = close_fn, \
			 })

extern int char_dev_init_all(void);
extern int char_dev_register(const struct dev_module *cdev);
extern int char_dev_idesc_fstat(struct idesc *idesc, void *buff);
extern int cdev_idesc_alloc(struct dev_module *cdev);

#endif /* DEVICE_H_ */
