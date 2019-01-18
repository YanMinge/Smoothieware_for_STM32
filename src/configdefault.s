.section .rodata.configdefault


.global _binary_config_default_start
.weak _binary_config_default_start

.global _binary_config_default_end
.weak _binary_config_default_end

.global _binary_config_default_size
.weak _binary_config_default_size

_binary_config_default_start:

.incbin "config.default"
_binary_config_default_end:

.set _binary_config_default_size, _binary_config_default_end

