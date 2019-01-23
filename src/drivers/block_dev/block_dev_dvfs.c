/**
 * @file
 * @brief DVFS-specific bdev handling
 * @author Denis Deryugin <deryugin.denis@gmail.com>
 * @version 0.1
 * @date 2015-10-01
 */

#include <string.h>
#include <sys/stat.h>

#include <drivers/device.h>
#include <drivers/block_dev.h>
#include <fs/dvfs.h>

extern struct idesc_ops idesc_bdev_ops;
/*
extern struct file_operations bdev_dev_ops;

static struct dev_operations block_device_operations = {
	.probe  = NULL,
	.remove = NULL,
};

static struct device block_device = {
	.dev_dops = &block_device_operations,
	.dev_iops = &idesc_bdev_ops,
};
*/

/**
 * @brief Create node in devfs
 *
 * @param path Name of bdev
 * @param driver
 * @param privdata
 *
 * @return Pointer to created device or NULL if failed
 */
struct block_dev *block_dev_create(const char *path, void *driver, void *privdata) {
	struct block_dev *bdev;
	char full_path[DVFS_MAX_PATH_LEN];
	struct lookup lu;
	struct dev_module *devmod;

	assert(path);
	assert(driver);

	/* Get root of devfs in smarter way? */
	strcpy(full_path, "/dev/");
	strncat(full_path, path, DVFS_MAX_PATH_LEN - strlen("/dev") - 1);
	if (dvfs_lookup("/dev", &lu)) {
		return NULL;
	}

	lu.parent = lu.item;
	lu.item = NULL;

	if (NULL == (bdev = block_dev_create_common(dvfs_last_link(path), driver, privdata)))
		return NULL;

	devmod = dev_module_create(bdev->name, NULL, NULL, &idesc_bdev_ops, bdev);
	// bdev->dev_ops = &bdev_dev_ops;
	// devmod->dev_file.f_ops = &bdev_dev_ops;
	bdev->dev_module = devmod;

	return bdev;
}

/**
 * @brief Destroy corresponding devfs node
 *
 * @param dev Pointer to device
 *
 * @return Negative error code or 0 if succeed
 */
int block_dev_destroy(void *dev) {
	return dev_module_destroy(dev);
}
