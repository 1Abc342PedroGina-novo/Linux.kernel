# SPDX-License-Identifier: GPL-2.0
#
# Makefile for the Linux kernel device drivers - HYBRID MODE
# Only essential boot-critical drivers compiled in-kernel
# All other drivers will run in user mode
#

# ============================================================================
# CRITICAL BOOT-TIME DRIVERS (Must be in-kernel)
# ============================================================================

# Core infrastructure (always needed)
obj-y				+= cache/
obj-y				+= irqchip/
obj-y				+= bus/
obj-y				+= clk/
obj-y				+= soc/

# Memory management and DMA (critical for boot)
obj-$(CONFIG_DMADEVICES)	+= dma/
obj-y				+= iommu/

# Power management (critical)
obj-$(CONFIG_PM_GENERIC_DOMAINS)	+= pmdomain/
obj-$(CONFIG_REGULATOR)		+= regulator/

# Reset controllers (needed for device init)
obj-$(CONFIG_RESET_CONTROLLER)	+= reset/

# PCI subsystem (common bus)
obj-y				+= pci/

# GPIO and pinctrl (basic hardware control)
obj-$(CONFIG_PINCTRL)		+= pinctrl/
obj-$(CONFIG_GPIOLIB)		+= gpio/

# PWM (common for many SoCs)
obj-y				+= pwm/

# Basic TTY/console (must be in-kernel for boot output)
obj-y				+= tty/
obj-y				+= char/

# RTC (system time)
obj-$(CONFIG_RTC_LIB)		+= rtc/

# Watchdog (system health)
obj-$(CONFIG_WATCHDOG)		+= watchdog/

# ============================================================================
# OPTIONAL CORE DRIVERS (Keep based on your hardware)
# ============================================================================

# Uncomment these if your platform needs them in-kernel
# obj-$(CONFIG_ACPI)		+= acpi/        # For x86/ARM64 servers
# obj-$(CONFIG_OF)		+= of/          # For device-tree platforms
# obj-$(CONFIG_PNP)		+= pnp/         # Legacy plug-and-play
# obj-y				+= amba/        # ARM AMBA bus
# obj-$(CONFIG_PCI)		+= usb/         # USB host controllers

# ============================================================================
# STORAGE (Minimal - only what's needed to mount rootfs)
# ============================================================================

# Basic block layer and SCSI (if needed)
obj-y				+= block/
obj-y				+= scsi/        # For SCSI/SATA drives
obj-$(CONFIG_ATA)		+= ata/         # For SATA controllers

# NVMe (modern SSDs)
obj-y				+= nvme/

# ============================================================================
# NETWORKING (Basic - if network boot needed)
# ============================================================================

# Minimal networking core
obj-y				+= net/

# ===========================================================================
# VIDEO (Basic Video Driver)
# ===========================================================================

obj-y	                        += video/
obj-y	                        += gpu/
