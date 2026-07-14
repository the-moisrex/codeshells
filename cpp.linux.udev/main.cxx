import standard;

#include <cstdint>
#include <libudev.h>
#include <print>
#include <stdio.h>

using namespace std;

int main(void) {
    struct udev* udev;
    struct udev_enumerate* enumerate;
    struct udev_list_entry *devices, *dev_list_entry;

    // 1. Create the main udev context
    udev = udev_new();
    if (!udev) {
        fprintf(stderr, "Can't create udev context\n");
        return 1;
    }

    // 2. Initialize the enumerate object
    enumerate = udev_enumerate_new(udev);
    if (!enumerate) {
        fprintf(stderr, "Can't create enumerate object\n");
        udev_unref(udev);
        return 1;
    }

    // 3. ADD FILTERS FIRST (Define what we want before scanning)
    // Filter by the "block" subsystem (hard drives, flash drives)

    // 4. SCAN THE DEVICES (Executes the search based on filters)

    udev_enumerate_add_match_subsystem(enumerate, "input");
    udev_enumerate_scan_devices(enumerate);

    // 5. Retrieve the head of the populated linked list
    devices = udev_enumerate_get_list_entry(enumerate);

    // 6. Iterate through the results
    udev_list_entry_foreach(dev_list_entry, devices) {
        const char* syspath;
        struct udev_device* dev;

        // Get the /sys path of the matching device
        syspath = udev_list_entry_get_name(dev_list_entry);

        // Create an actual device object from the syspath to read its
        // properties
        dev = udev_device_new_from_syspath(udev, syspath);
        if (!dev)
            continue;

        // Print out the hardware details
        printf("Device Found:\n");
        printf("  Syspath:   %s\n", syspath);
        printf("  Devnode:   %s\n",
               udev_device_get_devnode(dev)); // e.g., /dev/sda
        printf("  Subsystem: %s\n", udev_device_get_subsystem(dev));
        printf("\n");

        // Free the individual device object memory
        udev_device_unref(dev);
    }

    // 7. Clean up the enumeration object and udev context
    udev_enumerate_ref(
        enumerate); // Decrement loop/use reference if needed, or just unref
    udev_enumerate_unref(enumerate);
    udev_unref(udev);

    return 0;
}

// view ./build/optimizations.txt
