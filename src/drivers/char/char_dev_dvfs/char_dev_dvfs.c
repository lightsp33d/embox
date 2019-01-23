/**
 * @file
 * @brief Handle char devices
 * @author Denis Deryugin <deryugin.denis@gmail.com>
 * @version 0.1
 * @date 2015-10-05
 */
#include <errno.h>
#include <string.h>

#include <mem/misc/pool.h>
#include <util/array.h>
#include <util/dlist.h>
#include <util/indexator.h>
#include <util/log.h>

#include <fs/dvfs.h>

#include <drivers/char_dev.h>

#define MAX_DEV_QUANTITY OPTION_GET(NUMBER, dev_quantity)

#define CDEV_IDESC_POOL_SIZE OPTION_GET(NUMBER, cdev_idesc_quantity)
POOL_DEF(cdev_idesc_pool, struct idesc, CDEV_IDESC_POOL_SIZE);

static struct dev_module *devtab[MAX_DEV_QUANTITY];
INDEX_DEF(char_dev_idx, 0, MAX_DEV_QUANTITY);

ARRAY_SPREAD_DEF(const struct dev_module, __char_device_registry);

struct dev_module **get_cdev_tab(void) {
	return &devtab[0];
}

int char_dev_init_all(void) {
	struct dev_module *cdev;

	array_spread_foreach_ptr(cdev, __char_device_registry) {
		char_dev_register(cdev);
	}

	return 0;
}

int char_dev_register(struct dev_module *cdev) {
	int cdev_id;

	assert(cdev);

	/* This index is not supposed to be freed */
	cdev_id = index_alloc(&char_dev_idx, INDEX_MIN);
	if (cdev_id == INDEX_NONE) {
		log_error("Failed to register char dev %s", cdev->name ? cdev->name : "");
		return -ENOMEM;
	}

	devtab[cdev_id] = cdev;
	cdev->dev_id = cdev_id;

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

static struct idesc *char_dev_open(struct inode *node, struct idesc *idesc) {
	struct dev_module *cdev = node->i_data;
	struct idesc *idesc;

	if (!cdev) {
		log_error("Can't open char device");
		return NULL;
	}

	if (cdev->open != NULL) {
		idesc = cdev->open(cdev, cdev->dev_priv);
		return idesc;
	}

	idesc = pool_alloc(&cdev_idesc_pool);
	if (idesc == NULL) {
		log_error("Can't allocate char device");
		return NULL;
	}
	
	idesc_init(idesc, cdev->dev_iops, S_IROTH | S_IWOTH);
	return idesc;
	
	/* Perform device-specific initialization if neccessary.
	 *
	 * Generally it should be symmetrics with idesc_close(), but
	 * idesc ops has no open(), so we need to "imitate" opening like that */
	/*
	if (devmod->open) {
		if (devmod->open(devmod, devmod->dev_priv)) {
			log_error("Failed to open %s", devmod->name);
			return NULL;
		}
	}

	return ;
	*/
}
