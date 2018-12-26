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

#if 0
static struct idesc *char_dev_open(struct node *node, struct file_desc *file_desc, int flags) {
	struct dev_module *dev = node->nas->fi->privdata;

	if (!dev) {
		log_error("Can't open char device");
		return NULL;
	}

	dev->d_idesc = &file_desc->idesc;

	if (dev->open != NULL) {
		if (dev->open(dev, dev->dev_priv)) {
			log_error("Failed to open %s", dev->name);
			return NULL;
		}
	}

	file_desc->idesc.idesc_ops = dev->dev_iops;

	return &file_desc->idesc;
}
#endif

int char_dev_register(const struct dev_module *cdev) {
	struct path  node;
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
	dev_nas->fs = filesystem_create("empty");
	if (dev_nas->fs == NULL) {
		return -ENOMEM;
	}

	//dev_nas->fs->file_op = ops ? ops : &char_file_ops;
	node.node->nas->fi->privdata = (void *) cdev;

	return 0;
}
