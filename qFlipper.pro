TEMPLATE = subdirs

SUBDIRS += \
    3rdparty \
    application \
    backend \
    dfu \
    plugins \
    cli \
    workbench

backend.depends = dfu plugins workbench
application.depends = backend
cli.depends = backend
plugins.depends = 3rdparty

SUBDIRS += device_probe
device_probe.subdir = workbench/device-probe
device_probe.depends = backend workbench
