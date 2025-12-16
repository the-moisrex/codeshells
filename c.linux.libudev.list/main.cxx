import standard;

#include <cstdint>
#include <print>

#include <libudev.h>
#include <stdio.h>
#include <stdlib.h>


using namespace std;

int main(void) {
    struct udev *udev;
    struct udev_enumerate *enumerate;
    struct udev_list_entry *devices, *entry;
    struct udev_device *dev;

    /* Create the udev object */
    udev = udev_new();
    if (!udev) {
        fprintf(stderr, "Can't create udev\n");
        return 1;
    }

    /* Create an enumeration object and match the 'input' subsystem */
    enumerate = udev_enumerate_new(udev);
    udev_enumerate_add_match_subsystem(enumerate, "input");
    udev_enumerate_scan_devices(enumerate);
    devices = udev_enumerate_get_list_entry(enumerate);

    /* Iterate over matching devices */
    udev_list_entry_foreach(entry, devices) {
        const char *path = udev_list_entry_get_name(entry);
        dev = udev_device_new_from_syspath(udev, path);

        const char *devnode = udev_device_get_devnode(dev);
        const char *name = udev_device_get_property_value(dev, "NAME");
        const char *phys = udev_device_get_property_value(dev, "PHYS");
        const char *id_input = udev_device_get_property_value(dev, "ID_INPUT");

        if (devnode) {  /* Only print devices with a /dev node (e.g., eventX, mouseX) */
            printf("Device Node: %s\n", devnode);
            if (name) printf("  Name: %s\n", name);
            if (phys) printf("  Phys: %s\n", phys);
            if (id_input) printf("  Type Flags: %s\n", id_input);  /* e.g., ID_INPUT_KEYBOARD=1 */

            /* Optional: Filter for specific types, e.g., keyboards */
            if (udev_device_get_property_value(dev, "ID_INPUT_KEYBOARD")) {
                printf("  -> Keyboard\n");
            }
            if (udev_device_get_property_value(dev, "ID_INPUT_MOUSE")) {
                printf("  -> Mouse\n");
            }
            printf("\n");
        }

        udev_device_unref(dev);
    }

    /* Cleanup */
    udev_enumerate_unref(enumerate);
    udev_unref(udev);
    return 0;
}

// view ./build/optimizations.txt
