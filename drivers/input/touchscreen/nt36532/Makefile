# Makefile for the Novatek NT36xxx touchscreen driver.
ccflags-y += -Wno-missing-prototypes -Wno-missing-declarations -Wno-unused-variable -Wno-unused-function
ccflags-$(CONFIG_DRM_MEDIATEK_V2) += -I$(DEVICE_MODULES_PATH)/drivers/gpu/drm/mediatek/mediatek_v2/
obj-$(CONFIG_TOUCHSCREEN_NT36xxx_HOSTDL_SPI) += nt36532.o
nt36532-y += nt36xxx.o
nt36532-y += nt36xxx_fw_update.o
nt36532-y += nt36xxx_ext_proc.o
