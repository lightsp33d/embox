/*
 * @file
 *
 * @date 28.11.12
 * @author Anton Bondarev
 * @author Ilia Vaprol
 */
#include <errno.h>
#include <stdlib.h>

#include <drivers/char_dev.h>
#include <fs/file_desc.h>
#include <fs/node.h>
#include <fs/vfs.h>
#include <fs/file_operation.h>

#include <kernel/printk.h>

#include <util/array.h>
#include <util/err.h>
#include <util/log.h>
#include <mem/misc/pool.h>

#define MAX_DEV_QUANTITY OPTION_GET(NUMBER, dev_quantity)
POOL_DEF(cdev_standard_pool, struct idesc, MAX_DEV_QUANTITY);

ARRAY_SPREAD_DEF(const struct dev_module, __device_registry);


int char_dev_init_all(void) {
	const struct dev_module *cdev;

	array_spread_foreach_ptr(cdev, __device_registry) {
		char_dev_register(cdev);
	}

	return 0;
}

/* This stub is supposed to be used when there's no need
 * for device-specific idesc_ops.fstat() */
int char_dev_idesc_fstat(struct idesc *idesc, void *buff) {
	struct stat *sb;

	assert(buff);
	sb = buff;
	memset(sb, 0, sizeof(struct stat));
	sb->st_mode = S_IFCHR;

	return 0;
}

int cdev_idesc_alloc(struct dev_module *cdev) {
	cdev->d_idesc = pool_alloc(&cdev_standard_pool);
	if (!cdev->d_idesc) {
		return -ENOMEM;
	}

	return 0;
}

struct idesc *char_dev_open(struct node *node, int flags) {
	struct dev_module *cdev = node->nas->fi->privdata;

	if (!cdev) {
		log_error("Can't open char device");
		return NULL;
	}

	if (cdev->open != NULL) {
		log_error("No open function for char device %s",
				  cdev->name ? cdev->name : "");
		return cdev->open(cdev, cdev->dev_priv);
	}

	return NULL;
}

int char_dev_register(const struct dev_module *cdev) {
	struct path node;
	struct nas *dev_nas;

	if (vfs_lookup("/dev", &node)) {
		return -ENOENT;
	}

	if (node.node == NULL) {
		return -ENODEV;
	}

	vfs_create_child(&node, cdev->name, S_IFCHR | S_IRALL | S_IWALL, &node);
	if (!(node.node)) {
		return -1;
	}

	dev_nas = node.node->nas;
	dev_nas->fs = filesystem_create("devfs");
	if (dev_nas->fs == NULL) {
		return -ENOMEM;
	}

	//dev_nas->fs->file_op = ops ? ops : &char_file_ops;
	node.node->nas->fi->privdata = (void *) cdev;

	return 0;
}
