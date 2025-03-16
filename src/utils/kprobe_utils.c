#include "utils/kprobe_utils.h"

// Function to retrieve kallsyms_lookup_name using kprobes
kallsyms_lookup_name_t _kallsyms_lookup(void) {
    static struct kprobe kp = {
        .symbol_name = "kallsyms_lookup_name"
    };

    kallsyms_lookup_name_t kallsyms_lookup_name;
    register_kprobe(&kp);
    kallsyms_lookup_name = (kallsyms_lookup_name_t)kp.addr;
    unregister_kprobe(&kp);

    return kallsyms_lookup_name;
}

