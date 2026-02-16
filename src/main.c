#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <linux/kvm.h>
#include <string.h>

int main(int argc, char **argv) {
    printf("Hypervisor starting...\n\n");

    // Open /dev/kvm
    int kvm_fd = open("/dev/kvm", O_RDWR);
    if (kvm_fd < 0) {
        perror("Failed to open /dev/kvm");
        return 1;
    }
    printf("Opened /dev/kvm (fd=%d)\n", kvm_fd);

    // Create a virtual machine
    int vm_fd = ioctl(kvm_fd, KVM_CREATE_VM, 0);
    if (vm_fd < 0) {
        perror("Failed to create VM");
        close(kvm_fd);
        return 1;
    }
    printf("Created VM (fd=%d)\n", vm_fd);

    // Create a vCPU
    int vcpu_fd = ioctl(vm_fd, KVM_CREATE_VCPU, 0);
    if (vcpu_fd < 0) {
        perror("Failed to create vCPU");
        close(vm_fd);
        close(kvm_fd);
        return 1;
    }
    printf("Created vCPU #0 (fd=%d)\n", vcpu_fd);

    // Get vCPU mmap size for kvm_run struct
    int vcpu_mmap_size = ioctl(kvm_fd, KVM_GET_VCPU_MMAP_SIZE, 0);
    if (vcpu_mmap_size < 0) {
        perror("Failed to get vCPU mmap size");
        close(vcpu_fd);
        close(vm_fd);
        close(kvm_fd);
        return 1;
    }
    printf("vCPU mmap size: %d bytes\n", vcpu_mmap_size);

    // Map the vCPU run structure
    struct kvm_run *run = mmap(NULL, vcpu_mmap_size, PROT_READ | PROT_WRITE,
                                MAP_SHARED, vcpu_fd, 0);
    if (run == MAP_FAILED) {
        perror("Failed to mmap vCPU");
        close(vcpu_fd);
        close(vm_fd);
        close(kvm_fd);
        return 1;
    }
    printf("Mapped vCPU run structure\n");
    printf("\nSuccessfully created VM with 1 vCPU\n");

    // Allocate memory for the VM
    // mapping: GPA X read/write bytes at host address HVA + (X - GPA)
    size_t guest_mem_size = 0x1000;  // 4KB for now
    void *guest_mem = mmap(NULL, guest_mem_size, PROT_READ | PROT_WRITE,
                     MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    if (guest_mem == MAP_FAILED) {
        perror("Failed to allocate guest memory");
        munmap(run, vcpu_mmap_size);
        close(vcpu_fd);
        close(vm_fd);
        close(kvm_fd);
        return 1;
    }
    printf("Allocated %zu bytes of guest memory at %p\n", guest_mem_size, guest_mem);

    // Set up the memory region for the guest
    struct kvm_userspace_memory_region region = {
        .slot = 0,
        .flags = 0,
        .guest_phys_addr = 0x1000,  // Guest will see this at physical addr 0x1000
        .memory_size = guest_mem_size,
        .userspace_addr = (unsigned long)guest_mem,  // Host address
    };

    if (ioctl(vm_fd, KVM_SET_USER_MEMORY_REGION, &region) < 0) {
        perror("Failed to set memory region");
        munmap(guest_mem, guest_mem_size);
        munmap(run, vcpu_mmap_size);
        close(vcpu_fd);
        close(vm_fd);
        close(kvm_fd);
        return 1;
    }
    printf("Mapped guest physical 0x%llx -> host %p (%zu bytes)\n",
           region.guest_phys_addr, guest_mem, guest_mem_size);

    // Cleanup
    munmap(guest_mem, guest_mem_size);
    munmap(run, vcpu_mmap_size);
    close(vcpu_fd);
    close(vm_fd);
    close(kvm_fd);

    return 0;
}
