#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/kvm.h>

int main(int argc, char **argv) {
    printf("🚀 Hypervisor starting...\n");
    
    // Try to open /dev/kvm
    int kvm_fd = open("/dev/kvm", O_RDWR);
    if (kvm_fd < 0) {
        perror("ERROR: Failed to open /dev/kvm");
        printf("❌ Make sure KVM is enabled and you're in the 'kvm' group\n");
        return 1;
    }
    
    // Get KVM API version
    int api_version = ioctl(kvm_fd, KVM_GET_API_VERSION, 0);
    printf("✅ KVM API version: %d\n", api_version);
    
    if (api_version != KVM_API_VERSION) {
        printf("⚠️  KVM API version mismatch\n");
    }
    
    close(kvm_fd);
    printf("✅ Hypervisor initialized successfully!\n");
    
    return 0;
}
                            
